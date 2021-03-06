#include "board.h"
#include "timer.h"

#define TC_CLKS                  0x7
#define TC_CLKS_MCK2             0x0
#define TC_CLKS_MCK8             0x1
#define TC_CLKS_MCK32            0x2
#define TC_CLKS_MCK128           0x3
#define TC_CLKS_MCK1024          0x4

static volatile u32 timer_dummy;

static void AT91F_TC_Open(AT91PS_TC TC_pt, unsigned int Mode, unsigned int TimerId)
{
    // enable timer
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1<< TimerId );

    // stop timer and disable irq
    TC_pt->TC_CCR = AT91C_TC_CLKDIS;
    TC_pt->TC_IDR = 0xFFFFFFFF;

    // reset status
    timer_dummy = TC_pt->TC_SR;
    
    // set requested mode
    TC_pt->TC_CMR = Mode;

    // reset value
    TC_pt->TC_CV = 0;
}

// ----- timer1 -----
// timer1 measures index

void timer1_init(void)
{
    // Setup Timer1
    AT91F_TC_Open(AT91C_BASE_TC1,
                TC_CLKS_MCK1024 // clock/1024 = 21.308 ms
                | AT91C_TC_LDRA_FALLING
                | AT91C_TC_ETRGEDG_FALLING // reset on this edge of trigger (TIOA1)
                | AT91C_TC_ABETRG, // TIOA1 is trigger not TIOB1!
                AT91C_ID_TC1);
}

// ----- timer 2 -----
// timer 2 measures bit cells

void timer2_init( void )
{
    // Setup Timer2
    AT91F_TC_Open(AT91C_BASE_TC2,
                  TC_CLKS_MCK2 // clock/2 =  24 027 828 Hz
                  | AT91C_TC_LDRA_FALLING // capture on falling edge of trigger (TIOA2)
                  | AT91C_TC_ETRGEDG_FALLING // reset on falling edge of trigger (TIOA2)
                  | AT91C_TC_ABETRG, // TIOA2 is trigger
                  AT91C_ID_TC2);
}

#define TIMER2_INTERRUPT_LEVEL    4   // 0=lowest  7=highest

void timer2_enable_intr(irq_func f)
{
    // configure irq for TC2
    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_TC2,
                          TIMER2_INTERRUPT_LEVEL,
                          AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, f);
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_TC2); // AIC_IECR

    // force FIQ
    *AT91C_AIC_FFER = AT91C_ID_TC2;

    // enable irq: load a and overflow load and overflow counter
    AT91F_TC_InterruptEnable(AT91C_BASE_TC2, AT91C_TC_LDRAS | AT91C_TC_LOVRS | AT91C_TC_COVFS);
}

void timer2_disable_intr(void)
{
    AT91F_TC_InterruptDisable(AT91C_BASE_TC2, AT91C_TC_LDRAS | AT91C_TC_LOVRS | AT91C_TC_COVFS);

    AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_TC2); // AIC_IECR
}


