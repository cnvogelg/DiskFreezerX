#include "spi.h"

u08 spi_io(u08 d)
{
  while(!spi_low_tx_empty());
  spi_low_tx_byte(d);
  while(!spi_low_rx_full());
  return spi_low_rx_byte();
}

u08 spi_dummy_buffer[SPI_BUFFER_SIZE];

void spi_write_dma(const u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(data,size);
  spi_low_rx_dma_set_first(spi_dummy_buffer,size);
  spi_low_dma_enable();
  while(!spi_low_rx_dma_empty());
  spi_low_dma_disable();
}

void spi_read_dma(u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(spi_dummy_buffer,size);
  spi_low_rx_dma_set_first(data,size); // addr must be != 0 otherwise breaks
  spi_low_dma_enable();
  while(!spi_low_rx_dma_empty());
  spi_low_dma_disable();
}

