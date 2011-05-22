#include "sdpin.h"

// all pins combined

#define SD_SOCKET_WP    _BV(SD_SOCKET_WP_PIN)
#define SD_SOCKET_INS   _BV(SD_SOCKET_INS_PIN)
#define SD_SOCKET_ALL   (SD_SOCKET_WP | SD_SOCKET_INS)

void sdpin_init(void)
{
  // Enable PIO
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, _BV(AT91C_ID_PIOA) ) ;

  AT91C_BASE_PIOA->PIO_PPUDR = SD_SOCKET_ALL; // disable pull-up (is external)

  AT91F_PIO_InputFilterEnable( AT91C_BASE_PIOA, SD_SOCKET_ALL ); // glitch filter
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, SD_SOCKET_ALL ); // set HI
  AT91F_PIO_OutputDisable( AT91C_BASE_PIOA, SD_SOCKET_ALL ); // its an input
  AT91F_PIO_Enable( AT91C_BASE_PIOA, SD_SOCKET_ALL ); // enable
}

int  sdpin_no_card(void)
{
  return ( AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SD_SOCKET_INS );
}

int  sdpin_write_protect(void)
{
  return ( AT91F_PIO_GetInput(AT91C_BASE_PIOA) & SD_SOCKET_WP );
}
