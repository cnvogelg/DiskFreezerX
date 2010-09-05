#ifndef CMD_PARSE_H
#define CMD_PARSE_H

#include "board.h"

/* return 0 to quit */
u08 cmd_parse(u08 len, const u08 *buf, u08 *result_len, u08 *res_buf);

#endif
