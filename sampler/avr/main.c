/*
 * main.c - main loop
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

#include "global.h"
#include "uart.h"
#include "timer.h"
#include "display.h"
#include "console.h"
#include "fat/fatfs.h"

static void read_image(void)
{
	FIL fsrc;
	FRESULT res;
	char filename[] = "main.raw";

	res = f_open(&fsrc, filename, FA_OPEN_EXISTING | FA_READ);
	if(res != FR_OK)
	{
		console_puts((u08 *)"file not found!");
	    return;
	}

	for(u08 y=0;y<240;y++) {
		for(u16 x=0;x<320;x++) {
			u08 rgb[3];
			UINT read;
			res = f_read(&fsrc, &rgb, 3, &read);
			if(res != FR_OK)
				break;

			u16 color = RGB(rgb[0],rgb[1],rgb[2]);

			display_set_area(x,y,x,y);
			display_draw_start();
			display_draw_pixel(color);
			display_draw_stop();
		}
	}

	f_close(&fsrc);
}

int main (void){
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();

  // display init()
  display_init(2);
  console_init();
  console_puts((const u08 *)"AVRcon:\n");

  // sdcard/fatfs init
  fatfs_init(2);
  console_puts((const u08 *)"fatfs init\n");

  // mount sdcard
  if(fatfs_mount() == 0)
  {
	  console_puts((const u08 *)"mounted!");

	  read_image();

	  fatfs_umount();
  }
  else
  {
	  console_puts((const u08 *)"no card!");
  }

  // main loop
  while(1) {
      u08 ch = uart_read();
      console_putch(ch);
  }
  return 0;
}
