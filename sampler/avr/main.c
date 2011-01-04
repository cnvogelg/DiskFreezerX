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
#include "rram.h"

static void error_loop(u08 num)
{
    while(1) {
        for(u08 i=0;i<num;i++) {
            led0_on();
            timer_delay_10ms(10);
            led0_off();
            timer_delay_10ms(10);            
        }
        timer_delay_10ms(100);
    }
}

int main (void){
  // board init. e.g. switch off watchdog
  board_init();  
  // setup timer
  timer_init();
  // setup serial
  uart_init();

  // ripple RAM init
  rram_init();
  
  // test RAM
  u32 errors = rram_test();
  if(errors != 0) {
      error_loop(3);
  }

  // main loop
  while(1) {
    led0_on();
    timer_delay_10ms(100);
    led0_off();
    timer_delay_10ms(100);
  }
  
  return 0;
}
