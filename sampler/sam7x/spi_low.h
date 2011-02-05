#ifndef SPI_LOW_H
#define SPI_LOW_H

#include "board.h"

// CS pins
#define SPI_CS0_PIN         11
#define SPI_CS0_MASK        (1<<11)

// CS demuxer for RAM
#define SPI_MULTI_A0_PIN    2
#define SPI_MULTI_A0_MASK   (1<<2)
#define SPI_MULTI_A1_PIN    3
#define SPI_MULTI_A1_MASK   (1<<3)
#define SPI_MULTI_A2_PIN    4
#define SPI_MULTI_A2_MASK   (1<<4)
#define SPI_MULTI_EN_PIN    8
#define SPI_MULTI_EN_MASK   (1<<8)

#define SPI_MULTI_ALL_MASK  (SPI_MULTI_EN_MASK | SPI_MULTI_A0_MASK | SPI_MULTI_A1_MASK | SPI_MULTI_A2_MASK)

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

__inline void spi_low_set_cs(u32 cs)
{
  u32 mr = *AT91C_SPI_MR;
  mr &= ~AT91C_SPI_PCS;
  mr |= (cs << 16);
  *AT91C_SPI_MR = mr;
}

__inline void spi_low_enable_cs0(void)
{
  AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, SPI_CS0_MASK );
}

__inline void spi_low_disable_cs0(void)
{
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, SPI_CS0_MASK );
}

__inline void spi_low_enable_multi(void)
{
  AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, SPI_MULTI_EN_MASK );
}

__inline void spi_low_disable_multi(void)
{
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, SPI_MULTI_EN_MASK );
}

extern void spi_low_set_multi(int num);
extern void spi_low_dma_init(void);

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

__inline void spi_low_dma_enable(void)
{
        *AT91C_SPI_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;
}

__inline void spi_low_dma_disable(void)
{
        *AT91C_SPI_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS ;
}

__inline void spi_low_tx_irq_enable(void)
{
    *AT91C_SPI_IER = AT91C_SPI_ENDTX;
}

__inline void spi_low_tx_irq_disable(void)
{
    *AT91C_SPI_IDR = AT91C_SPI_ENDTX;
}

__inline int spi_low_tx_dma_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_ENDTX) == AT91C_SPI_ENDTX;
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

// --- RX ---

__inline int spi_low_rx_dma_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_ENDRX) == AT91C_SPI_ENDRX;
}

__inline int spi_low_all_rx_dma_empty(void)
{
    return (*AT91C_SPI_SR & AT91C_SPI_RXBUFF) == AT91C_SPI_RXBUFF;
}

__inline int spi_low_rx_dma_first_size(void)
{
        return *AT91C_SPI_RCR;
}

__inline int spi_low_rx_dma_first_empty(void)
{
        return *AT91C_SPI_RCR == 0;
}

__inline int spi_low_rx_dma_next_empty(void)
{
        return *AT91C_SPI_RNCR == 0;
}

__inline void spi_low_rx_dma_enable(void)
{
        *AT91C_SPI_PTCR = AT91C_PDC_RXTEN;
}

__inline void spi_low_rx_dma_disable(void)
{
        *AT91C_SPI_PTCR = AT91C_PDC_RXTDIS;
}

__inline void spi_low_rx_dma_set_first(u08 *buffer, u32 n)
{
        *AT91C_SPI_RPR = (u32)buffer;
        *AT91C_SPI_RCR = n;
}

__inline void spi_low_rx_dma_set_next(u08 *buffer, u32 n)
{
        *AT91C_SPI_RNPR = (u32)buffer;
        *AT91C_SPI_RNCR = n;
}

#endif
