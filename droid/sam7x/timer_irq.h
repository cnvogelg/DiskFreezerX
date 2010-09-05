#ifndef TIMER_IRQ_H
#define TIMER_IRQ_H

#include "board.h"

extern __ramfunc void timer0_irq_handler(void);
extern __ramfunc void timer1_irq_handler(void);
extern __ramfunc void timer2_irq_handler(void);

#endif
