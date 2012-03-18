#ifndef TIMER_H
#define TIMER_H

#include "board.h"

__inline void timer_trigger_all()
{
  *AT91C_TCB_BCR = AT91C_TCB_SYNC;
}

// TIMER1: Measure INDEX
void timer1_init ( void );

__inline u32 timer1_get_status( void )
{
  return AT91C_BASE_TC1->TC_SR;
}

__inline u32 timer1_get_capture_a( void )
{
  return AT91C_BASE_TC1->TC_RA;
}

__inline void timer1_enable(void)
{
    AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKEN;
}

__inline void timer1_trigger(void)
{
    AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG;
}

__inline void timer1_disable(void)
{
    AT91C_BASE_TC1->TC_CCR = AT91C_TC_CLKDIS;
}

// TIMER2: Capture Timer for Read Data
typedef void (*irq_func)(void);

void timer2_init( void );
void timer2_enable_intr(irq_func f);
void timer2_disable_intr(void);

__inline u32 timer2_get_status( void )
{
  return AT91C_BASE_TC2->TC_SR;
}

__inline u32 timer2_get_capture_a( void )
{
  return AT91C_BASE_TC2->TC_RA;
}

__inline u32 timer2_get_value( void )
{
  return AT91C_BASE_TC2->TC_CV;
}

__inline void timer2_enable( void )
{
    AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKEN;
}

__inline void timer2_trigger(void)
{
    AT91C_BASE_TC2->TC_CCR = AT91C_TC_SWTRG;
}

__inline void timer2_disable( void )
{
    AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS;
}

#endif
