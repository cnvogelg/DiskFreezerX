#include <stdio.h>
#include <string.h>

#include "cmd_parse.h"
#include "cmd_spi.h"
#include "config.h"

int cmd_parse_execute(spi_t *spi, const char *cmds)
{
    int num_cmds = 0;
    const char *c = cmds;
    int len = 0;
    
    // determine number of commands (excluding args)
    while(*c != '\0') {
        // skip byte arg
        if(*c == ':') {
            c += 3;
            len += 3;
        } else {
            c++;
            len++;
            num_cmds++;
        }
    }
    
    // send command sequence
    int result = cmd_spi_tx(spi, cmds, len, config.cmd_timeout);
    if(result < 0) {
        return result;
    }
    
    // get result
    int result_size = num_cmds * 2;
    char *res = (char *)malloc(result_size);
    if(res == NULL) {
        return -1;
    }
    
    result = cmd_spi_rx(spi, res, result_size, config.cmd_timeout);
    if(result < 0) {
        return result;
    }
    
    if(result != result_size) {
        return -1;
    }
    
    // check result codes
    int sum = 0;
    char *buf = res;
    int i;
    for(i=0;i<num_cmds;i++) {
        int val;
        if(sscanf(buf,"%02x",&val)!=1) {
            free(res);
            return -1;
        }
        sum += val;
        buf += 2;
    }
    
    free(res);
    return sum;
}
