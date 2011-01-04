/*
* rram.c - Ripple SRAM Control
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

#include "rram.h"

void rram_init(void)
{
    // set control lines as output and all to HI
    RRAM_CTRL_DDR  |= RRAM_CTRL_MASK;
    RRAM_CTRL_PORT |= RRAM_CTRL_MASK;
    // release reset
    RRAM_CTRL_PORT &= ~RRAM_CTRL_RESET;
}

void rram_write_begin(void)
{
    // data port to output
    RRAM_DATA0_DDR |= RRAM_DATA0_MASK;
    RRAM_DATA1_DDR |= RRAM_DATA1_MASK;
    // enable chip
    RRAM_CTRL_PORT &= ~RRAM_CTRL_CHIP_SELECT;

    rram_addr_reset();
}

void rram_write_end(void)
{
    // disable chip
    RRAM_CTRL_PORT |= RRAM_CTRL_CHIP_SELECT;    
    // data port to input
    RRAM_DATA0_DDR &= ~RRAM_DATA0_MASK;
    RRAM_DATA1_DDR &= ~RRAM_DATA1_MASK;
}

void rram_read_begin(void)
{
    // data port to input
    RRAM_DATA0_DDR &= ~RRAM_DATA0_MASK;
    RRAM_DATA1_DDR &= ~RRAM_DATA1_MASK;
    // pull up
    RRAM_DATA0_PORT |= RRAM_DATA0_MASK;
    RRAM_DATA1_PORT |= RRAM_DATA1_MASK;
    // enable chip
    RRAM_CTRL_PORT &= ~RRAM_CTRL_CHIP_SELECT;

    rram_addr_reset();
}

void rram_read_end(void)
{
    // disable chip
    RRAM_CTRL_PORT |= RRAM_CTRL_CHIP_SELECT;    
}

u32 rram_test(void)
{
    u32 i;

    // write to ram
    rram_write_begin();
    for(i = 0 ; i < RRAM_SIZE_BYTES ; i++) {
        u08 d = (u08)(i & 0xff);
        rram_write_byte(d);
    }
    rram_write_end();

    // read again
    u32 errors = 0;
    rram_read_begin();
    for(i = 0; i < RRAM_SIZE_BYTES ; i++) {
        u08 d = (u08)(i & 0xff);
        u08 v = rram_read_byte();
        if(v != d) {
            errors++;
        }
    }
    rram_read_end();
    
    return errors;
}


