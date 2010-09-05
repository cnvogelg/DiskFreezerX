#include "delay.h"

// ~14 clocks per loop
#define MCLK_MS     (MCK / 1000 / 14)
#define MCLK_US     (MCK / 1000000 / 14)

extern void delay_ms(u32 ms)
{
    volatile u32 v;
    volatile u32 u;
    for(v=0;v<ms;v++) {
        for(u=0;u<MCLK_MS;u++);
    }
}

extern void delay_us(u32 us)
{
    volatile u32 v;
    volatile u32 u;
    for(v=0;v<us;v++) {
        for(u=0;u<MCLK_US;u++);
    }
}
