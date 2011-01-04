/*
* uartutil.c - serial utility routines
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

#include "uart.h"
#include "util.h"

void uart_send_string(u08 *str)
{
    while(*str) {
        uart_send(*str);
        str++;
    }
}

void uart_send_data(u08 *data,u08 len)
{
    for(u08 i=0;i<len;i++) {
        uart_send(data[i]);
    }
}

void uart_send_crlf(void)
{
    uart_send_string((u08 *)"\r\n");
}

static u08 buf[6];

void uart_send_hex_byte_crlf(u08 data)
{
    byte_to_hex(data,buf);
    uart_send_data(buf,2);
    uart_send_crlf();
}

void uart_send_hex_word_crlf(u16 data)
{
    word_to_hex(data,buf);
    uart_send_data(buf,4);
    uart_send_crlf();
}

void uart_send_hex_dword6_crlf(u32 data)
{
    dword_to_hex6(data,buf);
    uart_send_data(buf,6);
    uart_send_crlf();
}

