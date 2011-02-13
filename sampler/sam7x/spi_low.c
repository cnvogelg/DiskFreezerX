#include "spi_low.h"

#define SPI_INTERRUPT_LEVEL 4

// ----- SPI control -----

void spi_low_cs_init(void)
{
  // Enable PIO for CSx
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;

  // manual config CS
  AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, SPI_CS0_MASK | SPI_MULTI_ALL_MASK );
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, SPI_CS0_MASK | SPI_MULTI_ALL_MASK );
}

void spi_low_mst_init(void)
{
    // Enable SPI clock
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_SPI ) ;

    // Configure PIO controllers to periph mode
    AT91F_PIO_CfgPeriph(
        AT91C_BASE_PIOA, // PIO controller base address
        //((unsigned int) AT91C_PA11_NPCS0) |
        ((unsigned int) AT91C_PA12_MISO ) |
        ((unsigned int) AT91C_PA13_MOSI ) |
        ((unsigned int) AT91C_PA14_SPCK ), // Periph A
        0); // Periph B

    AT91PS_SPI spi = AT91C_BASE_SPI;

    // reset SPI and enable it
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_CR = AT91C_SPI_SPIEN;

    // SPI mode: master mode
    // MODFDIS is required to allow manual CSx control
    spi->SPI_MR = AT91C_SPI_MSTR | AT91C_SPI_PS_FIXED | AT91C_SPI_MODFDIS;
}

void spi_low_init_channel(int ch, int scbr, int ncpha, int cpol)
{
    u32 val = AT91C_SPI_BITS_8;
    if(ncpha) {
        val |= AT91C_SPI_NCPHA;
    }
    if(cpol) {
        val |= AT91C_SPI_CPOL;
    }

    // configure channel
    AT91C_BASE_SPI->SPI_CSR[ch] = val | (scbr << 8);
}

void spi_low_set_speed(int ch, int scbr)
{
    // set new speed
    u32 old = AT91C_BASE_SPI->SPI_CSR[ch] &  ~(AT91C_SPI_SCBR);
    AT91C_BASE_SPI->SPI_CSR[ch] = old | (scbr << 8);
}

void spi_low_set_channel(int ch)
{
  u32 old = AT91C_BASE_SPI->SPI_MR &  ~(AT91C_SPI_PCS);
  AT91C_BASE_SPI->SPI_MR = old | (ch << 16);
}

void spi_low_slv_init(void)
{
    // Enable SPI clock
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_SPI ) ;

    // Configure PIO controllers to periph mode
    AT91F_PIO_CfgPeriph(
        AT91C_BASE_PIOA, // PIO controller base address
        //((unsigned int) AT91C_PA11_NPCS0) |
        ((unsigned int) AT91C_PA12_MISO ) |
        ((unsigned int) AT91C_PA13_MOSI ) |
        ((unsigned int) AT91C_PA14_SPCK ), // Periph A
        0); // Periph B

    AT91PS_SPI spi = AT91C_BASE_SPI;

     // SPI mode: slave mode
    spi->SPI_MR = 0;

    // CS0: 8 bits
    spi->SPI_CSR[0] = AT91C_SPI_BITS_8;
}

// ----- IRQ Handling -----

#if 0
void spi_low_irq_init(void)
{
    // Configure IRQ for SPI
	AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, 
	                        AT91C_ID_SPI, 
	                        SPI_INTERRUPT_LEVEL,
	                        AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, 
	                        (void (*)())spi_irq_handler);
}

void spi_low_irq_start(void)        
{
    spi_low_tx_irq_enable();
    
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SPI);
}

void spi_low_irq_stop(void)
{
    spi_low_tx_irq_disable();

    AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SPI);
}
#endif

// ----- DMA -----

void spi_low_dma_init(void)
{
  AT91PS_SPI spi = AT91C_BASE_SPI;

  // init the SPI's PDC-controller:
  // disable PDC TX and RX
  spi->SPI_PTCR = AT91C_PDC_TXTDIS | AT91C_PDC_RXTDIS;
  // init counters and buffer-pointers to 0
  // "next" TX
  spi->SPI_TNPR = 0;
  spi->SPI_TNCR = 0;
  // "next" RX
  spi->SPI_RNPR = 0;
  spi->SPI_RNCR = 0;
  // TX
  spi->SPI_TPR = 0;
  spi->SPI_TCR = 0;
  // RX
  spi->SPI_RPR = 0;
  spi->SPI_RCR = 0;
}

// ----- Multi -----

void spi_low_set_multi(int num)
{
  int set_mask = (num & 0x7) << SPI_MULTI_A0_PIN;
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, set_mask );
  int clr_mask = ((num ^ 0x7) & 0x7) << SPI_MULTI_A0_PIN;
  AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, clr_mask );
}
