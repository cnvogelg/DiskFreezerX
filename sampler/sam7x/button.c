#include "target.h"
#include "button.h"

#define BUTTON1         _BV(BUTTON1_PIN)
#define BUTTON2         _BV(BUTTON2_PIN)
#define ALL_BUTTONS     (BUTTON1 | BUTTON2)

void button_init(void)
{
  // Enable PIO
  AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, _BV(AT91C_ID_PIOA) ) ;

  AT91C_BASE_PIOA->PIO_PPUDR = ALL_BUTTONS; // disable pull-up (is external)

  AT91F_PIO_InputFilterEnable( AT91C_BASE_PIOA, ALL_BUTTONS ); // glitch filter
  AT91F_PIO_SetOutput( AT91C_BASE_PIOA, ALL_BUTTONS ); // set HI
  AT91F_PIO_OutputDisable( AT91C_BASE_PIOA, ALL_BUTTONS ); // its an input
  AT91F_PIO_Enable( AT91C_BASE_PIOA, ALL_BUTTONS ); // enable
}

int button1_pressed(void)
{
  return ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & BUTTON1_PIN) == 0 );
}

int button2_pressed(void)
{
  return ( (AT91F_PIO_GetInput(AT91C_BASE_PIOA) & BUTTON2_PIN) == 0 );
}
