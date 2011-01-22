#ifndef SPI_LOW_H
#define SPI_LOW_H

#include "board.h"

// setup SPI hardware
extern void spi_low_slv_init(void);
extern void spi_low_mst_init(unsigned int scbr);
extern void spi_low_close(void);

__inline u32 spi_low_status(void)
{
    return *AT91C_SPI_SR;
}

extern void spi_low_irq_init(void);
extern void spi_low_irq_start(void);
extern void spi_low_irq_stop(void);

__inline void spi_low_lastxfer(void)
{
  *AT91C_SPI_CR = AT91C_SPI_LASTXFER;
}

// ----- Transfer Inlines -----

__inline int spi_low_rx_full(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_RDRF) == AT91C_SPI_RDRF;
}

__inline int spi_low_tx_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_TDRE) == AT91C_SPI_TDRE;    
}

__inline int spi_low_tx_all_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_TXEMPTY) == AT91C_SPI_TXEMPTY;
}

__inline void spi_low_tx_byte(u08 b)
{
    *AT91C_SPI_TDR = (u32)b;
}

__inline u08 spi_low_rx_byte(void)
{
    return (u08)(*AT91C_SPI_RDR & 0xff);
}

// ----- DMA Inlines -----

__inline void spi_low_tx_irq_enable(void)
{
    *AT91C_SPI_IER = AT91C_SPI_ENDTX;
}

__inline void spi_low_tx_irq_disable(void)
{
    *AT91C_SPI_IDR = AT91C_SPI_ENDTX;
}

__inline int spi_low_all_tx_dma_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_TXBUFE) == AT91C_SPI_TXBUFE;
}

__inline int spi_low_tx_dma_first_empty(void)
{
	return *AT91C_SPI_TCR == 0;
}

__inline int spi_low_tx_dma_next_empty(void)
{
	return *AT91C_SPI_TNCR == 0;
}

__inline void spi_low_tx_dma_enable(void)
{
	*AT91C_SPI_PTCR = AT91C_PDC_TXTEN;
}

__inline void spi_low_tx_dma_disable(void)
{
	*AT91C_SPI_PTCR = AT91C_PDC_TXTDIS;
}

__inline void spi_low_tx_dma_set_first(const u08 *buffer, u32 n)
{
	*AT91C_SPI_TPR = (u32)buffer;
	*AT91C_SPI_TCR = n;
}

__inline void spi_low_tx_dma_set_next(const u08 *buffer, u32 n)
{
	*AT91C_SPI_TNPR = (u32)buffer;
	*AT91C_SPI_TNCR = n;
}

#endif
