#include "pit.h"

void pit_enable(void)
{
    // max PIV count
    *AT91C_PITC_PIMR = 0xfffff | AT91C_PITC_PITEN;
}

void pit_disable(void)
{
    *AT91C_PITC_PIMR &= ~AT91C_PITC_PITEN;
}

void pit_reset(void)
{
    pit_get(); // reset PICNT
}
