#include "spi_low.h"

#define SPI_INTERRUPT_LEVEL 4

// ----- SPI control -----

void spi_low_mst_init(unsigned int scbr)
{
    // Enable SPI clock
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_SPI ) ;

    // Configure PIO controllers to periph mode
    AT91F_PIO_CfgPeriph(
        AT91C_BASE_PIOA, // PIO controller base address
        ((unsigned int) AT91C_PA11_NPCS0) |
        ((unsigned int) AT91C_PA12_MISO ) |
        ((unsigned int) AT91C_PA13_MOSI ) |
        ((unsigned int) AT91C_PA14_SPCK ), // Periph A
        0); // Periph B
    
    AT91PS_SPI spi = AT91C_BASE_SPI;

    // reset
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_CR = AT91C_SPI_SPIEN;

    // SPI mode: master
    spi->SPI_MR = AT91C_SPI_MSTR;

    unsigned int dlybct = 2;

    // CS0: 8 bits
    spi->SPI_CSR[0] = AT91C_SPI_BITS_8 | AT91C_SPI_NCPHA | (scbr << 8) | AT91C_SPI_CSAAT | (dlybct << 24);
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

void spi_low_close(void)
{
    *AT91C_SPI_CR = AT91C_SPI_SPIDIS;
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
