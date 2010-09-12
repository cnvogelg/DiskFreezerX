/*
 * dfx-capture - DiskFreezerX capture tool for Linux
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spi_bulk.h"
#include "spi.h"
#include "cmd_parse.h"
#include "config.h"

static void cmd_exec(spi_t *spi)
{
    const char *cmd = config.args[0];
    const int max_result = 10;
    parse_result_t results[max_result];
    
    printf("executing command: '%s' (timout: %d ms)\n",cmd, config.cmd_timeout);
    int result = cmd_parse_execute(spi, cmd, max_result, results);
    printf("  result: %d\n", result);
    if(result >= 0) {
    	int i;
    	int num = (result < max_result) ? result : max_result;
    	for(i=0;i<num;i++) {
    		size_t size = results[i].size;
    		printf("  size: %d",size);
    		if(size > 0) {
    			int j;
    			printf(", data: ");
    			for(j=0;j<size;j++) {
    				printf("%02x ",results[i].data[j]);
    			}
    		}
    		printf("\n");
    	}
    	cmd_parse_free(result, results);
    }
}

int main(int argc, char *argv[])
{
    if(config_parse_opts(argc, argv)) {
        exit(1);
    }

    printf("dfx-capture Linux/SPI\n");

    if(config.verbose>0) {
        printf("  [opening device '%s' with %d Hz, mode=%d]\n", config.device, config.speed, config.spi_mode);
        printf("  [wait blocks: %d, max blocks: %d]\n", config.wait_blocks, config.max_blocks);
    }
    
    spi_t *spi = spi_open(config.device, config.speed, config.spi_mode, SPI_BLOCK_SIZE);
    if(spi == NULL) {
        printf("ERROR: opening spidev...\n");
        return 1;
    }
    
    if(config.verbose>0) {
        printf("  [opened spi device]\n");
    }

    // handle command
    switch(config.cmd) {
    case CMD_EXEC:
        cmd_exec(spi);
        break;
    case CMD_READ_TRK:
        read_trk(spi);
        break;
    case CMD_READ_DSK:
        read_dsk(spi);
        break;
    }

    if(config.verbose>0) {
        printf("  [closing spi device]\n");
    }
    
    spi_close(spi);
    
    if(config.verbose>0) {
        printf("  [ready]\n");
    }
    return 0;
}
