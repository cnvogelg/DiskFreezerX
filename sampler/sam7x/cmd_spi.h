#ifndef CMD_SPI_H
#define CMD_SPI_H

#include "board.h"

/* maximum size of a command or a result */
#define CMD_MAX_SIZE 0x3f

/* get next command via spi and handle internal requests from master */
extern u08  cmd_spi_get_next(u08 **data);

/* set the result the master can query with a RX command */
extern void cmd_spi_set_result(u08 *data,u08 size);

#endif
