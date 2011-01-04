/*
 * arduino2009.h - arduino2009 hardware access
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

#include <avr/io.h>

// PINS

// PC0  RIPPLE RESET
// PC1  /RIPPLE CLOCK
// PC2  /WRITE ENABLE SRAM
// PC3  /OUTPUT ENABLE SRAM
// PC4  /CHIP SELECT SRAM
// PC5  LED0

// PD2 ... PD7   IO DATA 3 .. IO DATA 8
// PB0 ... PB1   IO DATA 1 .. IO DATA 2

#ifndef ARDUINO2009BOARD_H
#define ARDUINO2009BOARD_H

// board setup
void ard2009_board_init(void);
#define board_init ard2009_board_init

// ----- LEDs -----
#define LED0_MASK    0x20
#define LED0_PORT    PORTC
#define LED0_DDR     DDRC

#define led0_off()       { LED0_PORT &= ~LED0_MASK; }
#define led0_on()        { LED0_PORT |= LED0_MASK; }

// ----- Ripple RAM -----
#define RRAM_CTRL_PORT          PORTC
#define RRAM_CTRL_DDR           DDRC
#define RRAM_CTRL_RESET         _BV(0)
#define RRAM_CTRL_CLOCK         _BV(1)
#define RRAM_CTRL_WRITE_ENABLE  _BV(2)
#define RRAM_CTRL_OUTPUT_ENABLE _BV(3)
#define RRAM_CTRL_CHIP_SELECT   _BV(4)
#define RRAM_CTRL_MASK          0x1f

// data bits 2..7 on this port
#define RRAM_DATA0_PORT         PORTD
#define RRAM_DATA0_PIN          PIND
#define RRAM_DATA0_DDR          DDRD
#define RRAM_DATA0_MASK         0xfc

// data bits 0..1 on this port
#define RRAM_DATA1_PORT         PORTB
#define RRAM_DATA1_PIN          PINB
#define RRAM_DATA1_DDR          DDRB
#define RRAM_DATA1_MASK         0x03

#endif

