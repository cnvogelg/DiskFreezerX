#ifndef USART_H
#define USART_H

#include "board.h"

extern void uart_init(void);
extern u32 uart_send(u08 data);
extern u32 uart_read_ready(void);
extern u32 uart_send_ready(void);
extern u32 uart_read(u08 *data);

#endif
