/*
 * arduino2009.c - arduino2009 hardware access
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of dtv2ser.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include <avr/interrupt.h>

#include "global.h"
#include "arduino2009.h"

// Board init

void ard2009_board_init(void)
{
   // disable watchdog
   cli();
   MCUSR &= ~(1<<WDRF);
   WDTCSR |= _BV(WDCE) | _BV(WDE);
   WDTCSR = 0;
   sei();
}

void led_init(void)
{
   // setup ports for LED
   LED0_DDR  |= LED0_MASK;
   LED0_PORT &= ~LED0_MASK;
}
