#include "button.h"

#define BUTTON1_PIN     (1 << 19)
#define BUTTON2_PIN     (1 << 20)

void button_init(void)
{
  // Enable PIO
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, _BV(AT91C_ID_PIOA) ) ;

  AT91C_BASE_PIOA->PIO_PPUDR = BUTTON1_PIN | BUTTON2_PIN; // disable pull-up (is external)

  AT91F_PIO_InputFilterEnable( AT91C_BASE_PIOA, BUTTON1_PIN | BUTTON2_PIN ); // glitch filter
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, BUTTON1_PIN | BUTTON2_PIN ); // set HI
  AT91F_PIO_OutputDisable( AT91C_BASE_PIOA, BUTTON1_PIN | BUTTON2_PIN ); // its an input
  AT91F_PIO_Enable( AT91C_BASE_PIOA, BUTTON1_PIN | BUTTON2_PIN ); // enable
}

int button1_pressed(void)
{
  return ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & BUTTON1_PIN) == 0 );
}

int button2_pressed(void)
{
  return ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & BUTTON2_PIN) == 0 );
}
