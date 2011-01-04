#ifndef SPI_H
#define SPI_H

#include "board.h"
#include "spi_low.h"

// for irq:
#define SPI_BUFFER_SIZE 1024

// marker for SPI bulk transfer
#define SPI_BULK_EOT    0xff
#define SPI_BULK_EOF    0xfe
#define SPI_BULK_BOF    0xfd
#define SPI_BULK_LAST   0xfc // last free for user in bulk transfers

extern u08 *spi_write_ptr;
extern u32  spi_write_size;
extern u32  spi_write_overruns;
extern u08 spi_buffer[4][SPI_BUFFER_SIZE];
extern u32 spi_write_index;

// ----- public interface -----

/* init SPI on board */
extern void spi_init(void);

/* ----- normal transfer ----- */

extern void spi_enable(void);
extern void spi_disable(void);

extern void spi_write_byte(u08 data);
extern u08  spi_read_byte(void);

/* ----- bulk (write) transfer ----- */

/* begin a bulk transfer */
extern void spi_bulk_begin(void);
/* end a bulk transfer. return 0 if no error otherwise time out occurred.  */
extern u32 spi_bulk_end(void);

/* handle DMA page flipping */
__inline void spi_bulk_handle(void)
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

/* write a byte to the bulk buffer */
__inline void spi_bulk_write_byte(u08 data)
{
    // store in current buffer
    if(spi_write_size == (SPI_BUFFER_SIZE-2)) {
        spi_write_overruns ++;
    } else {
        spi_write_ptr[spi_write_size++] = data;
    }
}

#endif
