/*
 * console.c - Console on screen
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of dfx-capture-ctrl.
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

#include <string.h>
#include "console.h"
#include "display.h"

static u08 buffer[CONSOLE_TOTAL];
static u08 x = 0;
static u08 y = 0;
static u16 pos = 0;

void console_init(void)
{
    display_clear(COLOR_BLACK);
    display_set_font_half(1);
    memset(buffer,32,CONSOLE_TOTAL);
    x = y = 0;
    pos = 0;
}

static void redraw(void)
{
    u08 *ptr = buffer;
    for(u08 y=0;y<CONSOLE_HEIGHT;y++) {
        for(u08 x=0;x<CONSOLE_WIDTH;x++) {
            display_draw_char(x,y,*ptr);
            ptr++;
        }
    }
}

static void scroll_up(void)
{
    u08 *tgt = buffer;
    u08 *src = tgt + CONSOLE_WIDTH;
    for(u16 i=0;i<(CONSOLE_TOTAL-CONSOLE_WIDTH);i++) {
        *(tgt++) = *(src++);
    }
    
    memset(tgt, 32 , CONSOLE_WIDTH);
    redraw();
}

void console_puts(const u08 *str)
{
    while(*str != '\0') {
        console_putch(*str);
        str++;
    }
}

void console_putch(u08 ch)
{
    // new line
    if(ch == '\n') {
        pos -= x;
        x = 0;
        if(y == ( CONSOLE_HEIGHT-1 )) {
            scroll_up();
        } else {
            y++;
            pos += CONSOLE_WIDTH;
        }
    } 
    // visible char
    else if(ch >= ' ') {
        buffer[pos] = ch;
        display_draw_char(x,y,ch);
        
        x++;
        pos++;
        if(x == CONSOLE_WIDTH) {
            x = 0;
            pos -= CONSOLE_WIDTH;
            if(y == ( CONSOLE_HEIGHT-1 )) {
                scroll_up();
            } else {
                y++;
                pos += CONSOLE_WIDTH;
            }
        }
    }
}
