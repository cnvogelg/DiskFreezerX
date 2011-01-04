/*
* uart.c - serial hw routines
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

#include <avr/io.h>

#include "uart.h"

#ifdef UBRR0H

// for atmeg644
#define UBRRH  UBRR0H
#define UBRRL  UBRR0L
#define UCSRA  UCSR0A
#define UCSRB  UCSR0B
#define UCSRC  UCSR0C
#define UDRE   UDRE0
#define UDR    UDR0

#define RXC    RXC0
#define TXC    TXC0
#define DOR    DOR0
#define PE     UPE0

#define RXEN   RXEN0
#define TXEN   TXEN0
#define UCSZ0  UCSZ00
#define UCSZ1  UCSZ01

#endif

// calc ubbr from baud rate
#define UART_UBRR   F_CPU/16/UART_BAUD-1

void uart_init(void) 
{
    // baud rate
    UBRRH = (u08)((UART_UBRR)>>8);
    UBRRL = (u08)((UART_UBRR)&0xff);

    UCSRB = _BV(RXEN) | _BV(TXEN);   // enable tranceiver and transmitter
    UCSRC = _BV(UCSZ0) | _BV(UCSZ1); // 8 bit, 1 stop, no parity, asynch. mode
}

u08 uart_read_data_available(void)
{
    return (UCSRA & _BV(RXC)) == _BV(RXC);
}

u08 uart_read(void)
{
    while(!( UCSRA & _BV(RXC)));
    return UDR;
}

void uart_send(u08 data)
{
    // wait for transmitter
    while(!( UCSRA & _BV(UDRE)));
    // send byte
    UDR = data;
}
