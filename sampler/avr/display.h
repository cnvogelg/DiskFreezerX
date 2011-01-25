/*
 * display.h - Control display
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

#ifndef DISPLAY_H
#define DISPLAY_H

#include "global.h"

#define DISPLAY_ORIENTATION_0           0
#define DISPLAY_ORIENTATION_90          1
#define DISPLAY_ORIENTATION_180         2
#define DISPLAY_ORIENTATION_270         3

#define RGB(r,g,b)   (((r&0xF8)<<8)|((g&0xFC)<<3)|((b&0xF8)>>3)) //5 red | 6 green | 5 blue

#define COLOR_WHITE  RGB(255,255,255)
#define COLOR_BLACK  RGB(  0,  0,  0)
#define COLOR_RED    RGB(255,  0,  0)
#define COLOR_GREEN  RGB(  0,255,  0)
#define COLOR_BLUE   RGB(  0,  0,255)

// read only parameters
extern u16 display_width;
extern u16 display_height;
extern u08 display_orientation; 

extern void display_init(u08 clock_div);
extern void display_reset(void);
extern void display_set_orientation(u08 o);
extern void display_set_area(u16 x0, u16 y0, u16 x1, u16 y1);

extern void display_draw_start(void);
extern void display_draw_pixel(u16 color);
extern void display_draw_stop(void);

extern void display_clear(u16 color);

extern void display_set_color(u16 fg, u16 bg);
extern void display_set_font_half(u08 half);
extern void display_draw_char(u08 cx,u08 cy,u08 ch);
extern void display_draw_string(u08 cx,u08 cy,const u08 *str);

#endif
