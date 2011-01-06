/*
 * console.h - Console on screen
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

#ifndef CONSOLE_H
#define CONSOLE_H

#include "global.h"

#define CONSOLE_WIDTH   40
#define CONSOLE_HEIGHT  30
#define CONSOLE_TOTAL   (CONSOLE_WIDTH * CONSOLE_HEIGHT)

extern void console_init(void);
extern void console_putch(u08 ch);
extern void console_puts(const u08 *str);

#endif

