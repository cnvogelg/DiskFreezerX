/* wiz-low.h
 * low-level SPI wiznet access
 */

#ifndef WIZ_LOW_H
#define WIZ_LOW_H

#include "target.h"

extern void wiz_low_init(void);
extern void wiz_low_begin(void);
extern void wiz_low_end(void);
extern void wiz_low_write(u16 addr, u08 value);
extern u08  wiz_low_read(u16 addr);

#endif
