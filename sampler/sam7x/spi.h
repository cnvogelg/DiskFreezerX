#ifndef SPI_H
#define SPI_H

#include "board.h"
#include "spi_low.h"

#define SPI_BUFFER_SIZE 512

extern u08  spi_io(u08 d);
extern void spi_write_dma(const u08 *data, u16 size);
extern void spi_read_dma(u08 *data, u16 size);

extern u08 spi_dummy_buffer[SPI_BUFFER_SIZE];

#endif
