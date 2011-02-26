#ifndef CMD_PARSE_H
#define CMD_PARSE_H

#include "board.h"

/* maximum size of a command or a result */
#define CMD_MAX_SIZE 0x3f

/* debug command get via uart */
extern u08 cmd_uart_get_next(u08 **data);

extern void cmd_parse(u08 len, const u08 *buf, u08 *result_len, u08 *res_buf);

#endif
