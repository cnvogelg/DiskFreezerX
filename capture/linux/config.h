#ifndef CONFIG_H
#define CONFIG_H

#include "stdint.h"

#define SIDES_ALL    'a'
#define SIDES_TOP    't'
#define SIDES_BOTTOM 'b'

#define CMD_EXEC      0
#define CMD_READ_TRK  1
#define CMD_READ_DSK  2

typedef struct {
    
    int begin_track;
    int end_track;
    char sides;
    
    int cmd;
    char **args;
    int num_args;
    
    const char *device;
    uint32_t speed;
    uint32_t wait_blocks;
    uint32_t max_blocks;
    int spi_mode;
    int cmd_timeout;

    int verbose;
    
} config_t;

extern config_t config;

int config_parse_opts(int argc, char *argv[]);

#endif
