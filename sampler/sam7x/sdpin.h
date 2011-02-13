// sdpin.h
// read pins from SD card: card inserted, write protect

#ifndef SDPIN_H
#define SDPIN_H

#include "board.h"

// write protect pin: PA25
#define SD_SOCKET_WP_PIN      AT91C_PIO_PA25
// card insert pin:   PA15
#define SD_SOCKET_INS_PIN     AT91C_PIO_PA15

// all pins combined
#define SD_SOCKET_PINS        (SD_SOCKET_WP_PIN | SD_SOCKET_INS_PIN)

extern void sdpin_init(void);
extern int  sdpin_no_card(void);
extern int  sdpin_write_protect(void);

#endif
