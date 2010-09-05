#include "timer.h"
#include "timer_irq.h"

#include "spi.h"

__ramfunc void timer0_irq_handler(void)
{
	AT91PS_TC TC_pt = AT91C_BASE_TC0;
    unsigned int dummy;
    //* Acknowledge interrupt status
    dummy = TC_pt->TC_SR;
    //* Suppress warning variable "dummy" was set but never used
    dummy = dummy;
    timer0_counter++;
}

__ramfunc void timer1_irq_handler(void)
{
	AT91PS_TC TC_pt = AT91C_BASE_TC1;
    unsigned int dummy;
    //* Acknowledge interrupt status
    dummy = TC_pt->TC_SR;
    //* Suppress warning variable "dummy" was set but never used
    dummy = dummy;
    timer1_counter++;
}

extern data_func timer2_func;
extern volatile u32 timer2_overflow;

__ramfunc void timer2_irq_handler(void)
{
    AT91PS_TC TC_pt = AT91C_BASE_TC2;

    // check status
    u32 status = TC_pt->TC_SR;
    
    // check for overflows
    if(status & AT91C_TC_LOVRS)
      timer2_overflow ++;

    // a data level was captured
    if(status & AT91C_TC_LDRAS) {
          u16 ra = (u16)TC_pt->TC_RA;
          timer2_func(ra);
    }    
}

