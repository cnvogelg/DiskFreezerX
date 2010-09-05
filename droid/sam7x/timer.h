#ifndef TIMER_H
#define TIMER_H

#include "board.h"

extern volatile int timer0_counter;
extern volatile int timer1_counter;

void timer0_init ( void );
void timer1_init ( void );

// TIMER2: Capture Timer for Read Data
typedef void (*data_func)(u16 delta);
void timer2_init( void );
void timer2_irq_enable( data_func f );
void timer2_start( void );
void timer2_stop( void );

__inline u32 timer2_get_status( void )
{
  return AT91C_BASE_TC2->TC_SR;
}

__inline u32 timer2_get_capture_a( void )
{
  return AT91C_BASE_TC2->TC_RA;
}

#endif
