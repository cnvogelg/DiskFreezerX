#ifndef CMD_PARSE_H
#define CMD_PARSE_H

#include "spi.h"

#define CMD_ERR_ABORTED	12
#define CMD_ERR_NO_MEM  13

typedef struct {
	char *data;
	int   size;
} parse_result_t;

/* return sum of result codes */
int cmd_parse_execute(spi_t *spi, const char *cmds, size_t max_results, parse_result_t *results);
void cmd_parse_free(int num_results, parse_result_t *results);

#endif
