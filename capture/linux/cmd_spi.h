#ifndef CMD_H
#define CMD_H

#include <stdint.h>
#include "spi.h"

#define CMD_MAX_SIZE 254

#define CMD_ERR_TOO_LONG    10
#define CMD_ERR_NOT_READY   11

extern int cmd_tx(spi_t *spi,const char *data, size_t len, int timeout_ms);
extern int cmd_rx(spi_t *spi,char *data, size_t max_len, int timeout_ms);

#endif

