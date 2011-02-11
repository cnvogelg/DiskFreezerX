// pit.h
// Periodic Interval Timer

#ifndef PIT_H
#define PIT_H

#include "board.h"

extern void pit_enable(void);
extern void pit_disable(void);
extern void pit_reset(void);

__inline u32 pit_get(void)
{
  return *AT91C_PITC_PIVR;
}

__inline u32 pit_peek(void)
{
  return *AT91C_PITC_PIIR;
}

typedef void (*pit_func)(void);

extern void pit_irq_start(pit_func func_10ms, pit_func func_500ms);
extern void pit_irq_stop(void);

#endif
