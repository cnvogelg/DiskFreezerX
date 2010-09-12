#include "spi.h"
#include "spi_low.h"

#include "uartutil.h"
#include "delay.h"

static u08 spi_buffer[4][SPI_BUFFER_SIZE];
static u32 spi_write_index;

u08 *spi_write_ptr;
u32  spi_write_size;
u32  spi_write_overruns;

void spi_init(void)
{
  spi_low_init();
  //spi_low_irq_init();
}

void spi_enable(void)
{
  spi_low_enable();
}

void spi_disable(void)
{
  spi_low_disable();
}

void spi_write_byte(u08 data)
{
  while(!spi_low_tx_empty());
  spi_low_tx_byte(data);
}

u08  spi_read_byte(void)
{
  while(!spi_low_rx_full());
  return spi_low_rx_byte();
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

void spi_bulk_handle(void)
{
  // next DMA request is empty again -> fill it!
  if(spi_low_tx_dma_next_empty()) {
      // write End Of Frame in current write buffer
      spi_write_ptr[spi_write_size] = SPI_BULK_EOF;

      // make current write buffer the next DMA buffer
      u08 *ptr = spi_buffer[spi_write_index];
      spi_low_tx_dma_set_next(ptr, SPI_BUFFER_SIZE);

      // advance to new write buffer
      spi_write_index = (spi_write_index + 1) & 3;
      spi_write_size = 0;
      spi_write_ptr = spi_buffer[spi_write_index] + 1; // skip BOF marker
  }
}

u32 spi_bulk_end(void)
{
  uart_send_string((u08 *)"flushing");
  uart_send_crlf();

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
          uart_send_string((u08 *)"TIMEOUT (2)");
          uart_send_crlf();
      }
  } else {
      error = 1;
      uart_send_string((u08 *)"TIMEOUT (1)");
      uart_send_crlf();
  }

  // disable bulk DMA
  spi_low_tx_dma_disable();

  // set read bytes to 0 again
  spi_write_byte(0);

  uart_send_string((u08 *)"stopping");
  uart_send_crlf();

  return error;
}
