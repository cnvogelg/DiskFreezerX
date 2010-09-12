#include <stdio.h>
#include <string.h>

#include "cmd_parse.h"
#include "cmd_spi.h"
#include "config.h"

#define CMD_RES_ABORT		0x80
#define CMD_RES_FAILED		0x40
#define CMD_RES_SIZE_MASK	0x3f

int cmd_parse_execute(spi_t *spi, const char *cmds, size_t max_results, parse_result_t *results)
{
    // send command sequence
    size_t len = strlen(cmds);
    int result = cmd_spi_tx(spi, cmds, len, config.cmd_timeout);
    if(result < 0) {
        return result;
    }

    // result buffer
    const int max_status_len = 128;
    char status_buf[max_status_len];
    
    // read result
    result = cmd_spi_rx(spi, status_buf, max_status_len, config.cmd_timeout);
    if(result < 0) {
        return result;
    }
    status_buf[result] = '\0';
    
    // check result codes
    char *buf = status_buf;
    int num_cmds = 0;
    int num_ok = 0;
    while(*buf != '\0') {
    	// scan result
    	int val;
        if(sscanf(buf,"%02x",&val)!=1) {
        	break;
        }
        buf+=2;

        // was aborted...
    	if(val & CMD_RES_ABORT)
    		return -CMD_ERR_ABORTED;

    	// next command
    	if((val & CMD_RES_FAILED)==0) {
    		num_ok ++;
    	}

    	// has optional bytes?
    	int size = (val & CMD_RES_SIZE_MASK);
    	if(size > 0) {
    		// store bytes?
    		char *out = NULL;
    		if((results != NULL)&&(num_cmds < max_results)) {
    			out = (char *)malloc(size);
    			results[num_cmds].data = out;
    			results[num_cmds].size = (out != NULL) ? size : 0;
    		}

    		// decode optional bytes
    		int i;
    		for(i=0;i<size;i++) {
    			if(sscanf(buf,"%02x",&val)!=1) {
    				return -CMD_ERR_ABORTED;
    			}
    			buf+=2;
    			if(out != NULL) {
    				*(out++) = (char)(val & 0xff);
    			}
    		}
    	} else {
    		if((results != NULL)&&(num_cmds < max_results)) {
    			results[num_cmds].data = NULL;
    			results[num_cmds].size = 0;
    		}
    	}

   		num_cmds++;
    }
    return num_ok;
}

void cmd_parse_free(int num_results, parse_result_t *results)
{
	int i;
	for(i=0;i<num_results;i++) {
		if(results[i].data!=NULL) {
			free(results[i].data);
			results[i].data = NULL;
			results[i].size = 0;
		}
	}
}
