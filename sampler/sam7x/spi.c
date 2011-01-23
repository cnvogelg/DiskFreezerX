#include "spi.h"
#include "spi_low.h"
#include "board.h"

#include "delay.h"

u08 spi_buffer[4][SPI_BUFFER_SIZE];
u32 spi_write_index;

u08 *spi_write_ptr;
u32  spi_write_size;
u32  spi_write_overruns;

void spi_slv_init(void)
{
  spi_low_slv_init();
  //spi_low_irq_init();
}

void spi_mst_init(unsigned int scbr)
{
  spi_low_mst_init(scbr);
  //spi_low_irq_init();
}

void spi_close(void)
{
  spi_low_close();
}

u08 spi_io(u08 d)
{
  while(!spi_low_tx_empty());
  spi_low_tx_byte(d);
  while(!spi_low_rx_full());
  u08 r = spi_low_rx_byte();
  return r;
}

/* ----- bulk mode ----- */

void spi_bulk_begin(void)
{
  int i,j;

  // clear all buffers and prepare with BOF/EOF tags
  for(j=3;j>=0;j--) {
      u08 *ptr = spi_buffer[j];
      for(i=SPI_BUFFER_SIZE-1;i>=0;i--) {
          *(ptr++) = 0;
      }

      // set BOF/EOF
      spi_buffer[j][0] = SPI_BULK_BOF;
      spi_buffer[j][1] = SPI_BULK_EOF;
  }

  // init write state: we start writing buffer 0
  spi_write_ptr = spi_buffer[0] + 1;
  spi_write_index = 0;
  spi_write_size = 0;
  spi_write_overruns = 0;

  // setup dma to start with buffe 2 and then continue with buffer 3
  spi_low_tx_dma_set_first(spi_buffer[2], SPI_BUFFER_SIZE);
  spi_low_tx_dma_set_next(spi_buffer[3], SPI_BUFFER_SIZE);
  spi_low_tx_dma_enable();
}

u32 spi_bulk_end(void)
{
  // wait for next DMA buffer to become empty
  int i;
  for(i=1000 * 1000;i>0;i--) {
      if(spi_low_tx_dma_next_empty())
        break;
      delay_us(1);
  }

  // no time out -> next DMA buffer is empty
  u32 error = 0;
  if(i!=0) {
      // write End Of Frame marker in current write buffer
      spi_write_ptr[spi_write_size] = SPI_BULK_EOF;

      // place End Of Transmission marker at the end of the block
      u08 *ptr = spi_buffer[spi_write_index];
      ptr[SPI_BUFFER_SIZE-1] = SPI_BULK_EOT;

      // set as next dma buffer
      spi_low_tx_dma_set_next(ptr, SPI_BUFFER_SIZE);

      // now wait for both DMA buffers getting drained
      for(i=1000 * 1000;i>0;i--) {
          if(spi_low_all_tx_dma_empty())
            break;
          delay_us(1);
      }

      if(i==0) {
          error = 1;
      }
  } else {
      error = 1;
  }

  // disable bulk DMA
  spi_low_tx_dma_disable();

  return error;
}
