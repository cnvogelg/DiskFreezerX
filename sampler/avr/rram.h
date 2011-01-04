/*
 * rram.h - Ripple SRAM Control
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

#ifndef RRAM_H
#define RRAM_H

#include "global.h"

// size of RAM: 512K
#define RRAM_SIZE_BYTES     (512U * 1024U)

// setup ports of RAM
extern void rram_init(void);

// reset address counter of ripple ram
inline void rram_addr_reset(void)
{
    // set reset to HI and then LO again
    RRAM_CTRL_PORT |= RRAM_CTRL_RESET;
    RRAM_CTRL_PORT &= ~RRAM_CTRL_RESET;
}

// increment adress with ripple counter
inline void rram_addr_inc(void)
{
    // toggle clock from HI -> LO -> HI
    RRAM_CTRL_PORT &= ~RRAM_CTRL_CLOCK;
    RRAM_CTRL_PORT &= RRAM_CTRL_CLOCK;
}

extern void rram_write_begin(void);
extern void rram_write_end(void);
extern void rram_read_begin(void);
extern void rram_read_end(void);

inline void rram_write_byte(u08 b)
{
    // write byte to data port
    RRAM_DATA0_PORT = (RRAM_DATA0_PORT & ~RRAM_DATA0_MASK) | (b & RRAM_DATA0_MASK);
    RRAM_DATA1_PORT = (RRAM_DATA1_PORT & ~RRAM_DATA1_MASK) | (b & RRAM_DATA1_MASK);
    
    // toggle WE: HI -> LO -> HI
    RRAM_CTRL_PORT &= ~RRAM_CTRL_WRITE_ENABLE;
    RRAM_CTRL_PORT |=  RRAM_CTRL_WRITE_ENABLE;
    
    rram_addr_inc();
}

inline u08 rram_read_byte(void)
{
    u08 data = (RRAM_DATA0_PIN & RRAM_DATA0_MASK) | (RRAM_DATA1_PIN & RRAM_DATA1_MASK);

    rram_addr_inc();

    return data;
}

// test memory and return 0 if no errors
extern u32 rram_test(void);

#endif
