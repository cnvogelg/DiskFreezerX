#include "uart.h"

void uart_init(void)
{
    // Enable USART 0
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_US0 ) ;
    
    // Configure PIO controllers to periph mode
	AT91F_PIO_CfgPeriph(
		AT91C_BASE_PIOA, // PIO controller base address
		((unsigned int) AT91C_PA5_RXD0    ) |
        ((unsigned int) AT91C_PA6_TXD0    ), // Periph A
        0); // Periph B
	
	// setup USART 0
    AT91PS_USART p = AT91C_BASE_US0;
    
    // set mode
    p->US_MR = AT91C_US_USMODE_NORMAL |         // normal mode
               AT91C_US_CLKS_CLOCK |            // MCK
               AT91C_US_CHRL_8_BITS |           // 8 bits
               AT91C_US_PAR_NONE |              // parity: none
               AT91C_US_NBSTOP_1_BIT |          // 1 stop bit
               AT91C_US_CHMODE_NORMAL;          // channel mode: normal

    // baud rate
    // CD = MCK / (16 * baud)
    // MCK = 48054857
    // baud = 38400
    // -> CD = 78
    p->US_BRGR = 26; // 115200

    // enable RX + TX + reset status
    p->US_CR = AT91C_US_RXEN |  AT91C_US_TXEN | AT91C_US_RSTSTA;
}

u32 uart_read_ready(void)
{
    AT91PS_USART p = AT91C_BASE_US0;
    u32 status = p->US_CSR;
    return (status & AT91C_US_RXRDY) == AT91C_US_RXRDY;
}

u32 uart_send_ready(void)
{
    AT91PS_USART p = AT91C_BASE_US0;
    u32 status = p->US_CSR;
    return (status & AT91C_US_TXRDY) == AT91C_US_TXRDY;
}

u32 uart_read(u08 *data)
{
    AT91PS_USART p = AT91C_BASE_US0;
    *data = (u08)(p->US_RHR & 0xff);
    return true;
}

u32 uart_send(u08 data)
{
    AT91PS_USART p = AT91C_BASE_US0;
    
    // wait for ready
    while((p->US_CSR & AT91C_US_TXRDY) == 0);
    
    p->US_THR = (u32)data;
    return true;
}

