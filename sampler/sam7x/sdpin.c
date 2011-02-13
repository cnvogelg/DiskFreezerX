#include "sdpin.h"

void sdpin_init(void)
{
  // Enable PIO
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, _BV(AT91C_ID_PIOA) ) ;

  AT91C_BASE_PIOA->PIO_PPUDR = SD_SOCKET_PINS; // disable pull-up (is external)

  AT91F_PIO_InputFilterEnable( AT91C_BASE_PIOA, SD_SOCKET_PINS ); // glitch filter
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, SD_SOCKET_PINS ); // set HI
  AT91F_PIO_OutputDisable( AT91C_BASE_PIOA, SD_SOCKET_PINS ); // its an input
  AT91F_PIO_Enable( AT91C_BASE_PIOA, SD_SOCKET_PINS ); // enable
}

int  sdpin_no_card(void)
{
  return ( AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SD_SOCKET_INS_PIN );
}

int  sdpin_write_protect(void)
{
  return ( AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SD_SOCKET_WP_PIN );
}
