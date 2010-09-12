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
extern void spi_bulk_handle(void);

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
