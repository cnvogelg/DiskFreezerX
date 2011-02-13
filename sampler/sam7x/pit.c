#include "pit.h"

void pit_set_max(u32 max)
{
  if(max == 0) {
      max = AT91C_PITC_PIV;
  }
  u32 old = *AT91C_PITC_PIMR & ~ AT91C_PITC_PIV;
  *AT91C_PITC_PIMR = old | (max & AT91C_PITC_PIV);
}

void pit_enable(void)
{
  *AT91C_PITC_PIMR |= AT91C_PITC_PITEN;
}

void pit_disable(void)
{
  *AT91C_PITC_PIMR &= ~AT91C_PITC_PITEN;
}

void pit_reset(void)
{
  pit_get(); // reset PICNT
}

#define PIT_PERIOD 1000

static u32 timestamp;
static u32 second_tick;
static pit_func pit_func_10ms;
static pit_func pit_func_500ms;

__ramfunc static void pit_irq_svc(void)
{
  static unsigned int prescLED, prescDTP, mscount;
  unsigned int incr;

  // Read the PIT status register and handle if flag set
  if ( AT91F_PITGetStatus(AT91C_BASE_PITC) & AT91C_PITC_PITS ) {

      if ( prescLED++ >= 500 ) {
          prescLED = 0;
          pit_func_500ms();
      }

      if ( prescDTP++ >= 10 ) {
          prescDTP = 0;
          pit_func_10ms();
      }

      // Read the PIVR to acknowledge interrupt and get number of ticks
      incr = (AT91F_PITGetPIVR(AT91C_BASE_PITC) >> 20);

      // update timestamp
      timestamp += incr;

      // update second-counter
      mscount += incr;
      if ( mscount >= 1000) {
          second_tick++;
          mscount -= 1000;
      }
  }
}

void pit_irq_start(pit_func func_10ms, pit_func func_500ms)
{
  pit_func_10ms = func_10ms;
  pit_func_500ms = func_500ms;

  AT91F_PITInit(AT91C_BASE_PITC, PIT_PERIOD, MCK / 1000000);

  // setup irq
  AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_SYS,
      AT91C_AIC_PRIOR_LOWEST,
      AT91C_AIC_SRCTYPE_INT_POSITIVE_EDGE, (void (*)(void))pit_irq_svc);
  AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
  AT91F_PITEnableInt(AT91C_BASE_PITC);

  pit_reset();
  pit_enable();

  timestamp = 0;
}

void pit_irq_stop(void)
{
  AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_SYS);
  AT91F_PITDisableInt(AT91C_BASE_PITC);

  pit_disable();
}
