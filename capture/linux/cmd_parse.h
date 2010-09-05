#ifndef CMD_PARSE_H
#define CMD_PARSE_H

#include "spi.h"

/* return sum of result codes */
int cmd_parse_execute(spi_t *spi, const char *cmds);

#endif
