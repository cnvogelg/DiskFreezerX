#include "board.h"
#include "timer.h"
#include "timer_irq.h"

#include "uartutil.h"

//* Global variable
volatile int timer0_counter;
volatile int timer1_counter;

#define TIMER0_INTERRUPT_LEVEL      1
#define TIMER1_INTERRUPT_LEVEL      4
#define TIMER2_INTERRUPT_LEVEL      5

#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

static void AT91F_TC_Open(AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId)
{
    unsigned int dummy;

    // enable timer
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< TimerId );

    // stop timer and disable irq
    TC_pt->TC_CCR = AT91C_TC_CLKDIS;
    TC_pt->TC_IDR = 0xFFFFFFFF;

    // reset status
    dummy = TC_pt->TC_SR;
    dummy = dummy;
    
    // set requested mode
    TC_pt->TC_CMR = Mode;

    // disable clock for now
    TC_pt->TC_CCR = AT91C_TC_CLKDIS ;
}

void timer0_init(void)
{
    timer0_counter=0;

    // timer0 init
    AT91F_TC_Open(AT91C_BASE_TC0,TC_CLKS_MCK1024,AT91C_ID_TC0);

    // timer0 irq init
    AT91F_AIC_ConfigureIt (AT91C_BASE_AIC, 
                           AT91C_ID_TC0,
                           TIMER0_INTERRUPT_LEVEL,
                           AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
                           (void (*)())timer0_irq_handler);
    AT91C_BASE_TC0->TC_IER = AT91C_TC_CPCS;  //  IRQ enable CPC
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC0);

    // timer0 trigger
    AT91C_BASE_TC0->TC_CCR = AT91C_TC_SWTRG ;
}

void timer1_init(void)
{
    timer1_counter=0;

    // timer1 init
    AT91F_TC_Open(AT91C_BASE_TC1,TC_CLKS_MCK128,AT91C_ID_TC1);

    // timer1 irq init
    AT91F_AIC_ConfigureIt (AT91C_BASE_AIC,
                           AT91C_ID_TC1,
                           TIMER1_INTERRUPT_LEVEL,
                           AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL,
                           (void (*)())timer1_irq_handler);
    AT91C_BASE_TC1->TC_IER  = AT91C_TC_CPCS;  //  IRQ enable CPC
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC1);

    // timer1 trigger
    AT91C_BASE_TC1->TC_CCR = AT91C_TC_SWTRG ;
}

// ----- timer 2 -----

data_func timer2_func;
volatile u32 timer2_overflow;

void timer2_init( void )
{
    // Setup Timer2
    AT91F_TC_Open(AT91C_BASE_TC2,
                  TC_CLKS_MCK2 
                  | AT91C_TC_LDRA_FALLING // capture on falling edge of trigger (TIOA2)
                  | AT91C_TC_ETRGEDG_FALLING // reset on falling edge of trigger (TIOA2)
                  | AT91C_TC_ABETRG, // TIOA2 is trigger
                  AT91C_ID_TC2);
}

void timer2_irq_enable( data_func f )
{
    // Configure IRQ for Timer2
    AT91F_AIC_ConfigureIt ( AT91C_BASE_AIC, 
                            AT91C_ID_TC2, 
                            TIMER2_INTERRUPT_LEVEL,
                            AT91C_AIC_SRCTYPE_INT_HIGH_LEVEL, 
                            (void (*)())timer2_irq_handler);
                            
                            
    // Set Timer IRQ
    AT91F_TC_InterruptEnable(AT91C_BASE_TC2, AT91C_TC_LDRAS);

    // Enable IRQ for Timer2
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2);    

    timer2_func = f;
}

void timer2_start( void )
{
    timer2_overflow = 0;

    AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKEN;
    AT91C_BASE_TC2->TC_CCR = AT91C_TC_SWTRG;
}

void timer2_stop( void )
{
    AT91C_BASE_TC2->TC_CCR = AT91C_TC_CLKDIS;

    if(timer2_overflow > 0) {
        uart_send_string((u08 *)"timer2 overflows ");
        uart_send_hex_dword_crlf(timer2_overflow);
    }
}

