#include "led.h"

// PIO pins of LEDs
#define LED_GREEN_PIN   18
#define LED_YELLOW_PIN  17

#define LED_GREEN           _BV(LED_GREEN_PIN)
#define LED_YELLOW          _BV(LED_YELLOW_PIN)
#define LED_GREEN_YELLOW    (LED_GREEN | LED_YELLOW)

extern void led_init(void)
{
    // Enable PIO
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, 1 << AT91C_ID_PIOA ) ;

    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, LED_GREEN_YELLOW );
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_GREEN_YELLOW );
}

extern void led_green(u32 on)
{
    if(on) {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, LED_GREEN) ;
    } else {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_GREEN) ;    
    }
}

extern void led_yellow(u32 on)
{
    if(on) {
        AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, LED_YELLOW) ;
    } else {
        AT91F_PIO_SetOutput( AT91C_BASE_PIOA, LED_YELLOW) ;    
    }    
}
