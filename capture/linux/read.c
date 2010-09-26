#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>

#include "spi.h"
#include "read.h"
#include "spi_bulk.h"
#include "config.h"
#include "cmd_parse.h"

static int control_floppy(spi_t *spi,const char *cmd)
{
    if(config.verbose>0) {
        printf("  [control floppy (cmd: '%s')]\n", cmd);
    }        
    int result = cmd_parse_execute(spi, cmd, 0, NULL);
    if(config.verbose>0) {
        printf("  [result: %d]\n", result);
    }
    return result;    
}

static int get_status(spi_t *spi,read_status_t *status)
{
	if(config.verbose>0) {
		printf("  [get status (cmd: 'r')]\n");
	}
	parse_result_t pr;
	int result = cmd_parse_execute(spi, "r", 1, &pr);
	if(config.verbose>0) {
		printf("  [result: %d]\n", result);
	}

	if(result == 1) {
		if(pr.size == 5 * 4) {
			uint32_t *ptr = (uint32_t *)pr.data;
			status->index_overruns = ntohl(ptr[0]);
			status->cell_overruns  = ntohl(ptr[1]);
			status->cell_overflows = ntohl(ptr[2]);
			status->data_size      = ntohl(ptr[3]);
			status->data_overruns  = ntohl(ptr[4]);
			if(config.verbose>0) {
				printf("  [index_ovveruns=%u, cell_overruns=%u, cell_overflows=%u, data_size=%u, data_overruns=%u]\n",
						status->index_overruns,
						status->cell_overruns,
						status->cell_overflows,
						status->data_size,
						status->data_overruns);
			}
		} else {
			printf("Invalid status size returned from sampler: %d\n", pr.size);
			return -1;
		}
	} else {
		printf("Error getting status from sampler: %d\n", result);
	}

	return result;
}

static int check_status(spi_t *spi, uint32_t decoded_size)
{
	// read status from sampler
	read_status_t status;
	if(get_status(spi, &status)!=1) {
		return -1;
	}

	// check overflows/overruns
	int fails = 0;
	if(status.index_overruns>0) {
		fails ++;
	}
	if(status.cell_overruns>0) {
		fails ++;
	}
	if(status.cell_overflows>0) {
		fails ++;
	}
	if(status.data_overruns>0) {
		fails ++;
	}
	if(fails > 0) {
		printf("\n  Overruns: index=%u, cell=%u, data=%u   Overflows: cell=%u\n",
				status.index_overruns,
				status.cell_overruns,
				status.data_overruns,
				status.cell_overflows);

	}

	// check data size
	if(status.data_size != decoded_size) {
		printf("\nData size mismatch: sent=%u != got=%u\n",status.data_size, decoded_size);
		return -1;
	}

	return fails;
}

static int start_floppy(spi_t *spi)
{
    // startup floppy command
    char side;
    if(config.sides == SIDES_TOP) {
        side = 't';
    } else {
        side = 'b';
    }
    char cmd[] = "eosz?+..";
    cmd[4] = side;
    
    int num_cmds = 6;
    if(config.begin_track>0) {
        sprintf(cmd+6,"%02x",config.begin_track);
    } else {
        cmd[5] = '\0';
        num_cmds = 5;
    }
    
    int result = control_floppy(spi,cmd);
    
    // check value
    if(result == num_cmds)
        return 0;
    else if(result < 0)
    	return result;
    else
    	return -1;
}

static int stop_floppy(spi_t *spi)
{
    return (control_floppy(spi,"fd")==2) ? 0 : -1;
}

int read_dsk(spi_t *spi)
{
    int result = start_floppy(spi);
    if(result < 0) {
        return result;
    }

    int num_tracks = config.end_track - config.begin_track + 1;
    int side;
    switch(config.sides) {
        case SIDES_TOP:
            side = 1;
            break;
        case SIDES_BOTTOM:
            side = 0;
            break;
        case SIDES_ALL:
            side = 0; 
            num_tracks *= 2;
            break;
    }

    int i;
    int t = config.begin_track;
    int error = 0;
    for(i=0;i<num_tracks;i++) {
        // determine file name
        const char *output_file = config.args[0];
        char name[256];
        snprintf(name,255,"%s_%02d_%d",output_file,t,side);

        printf("reading track %02d.%d to '%s'\r", t, side, name);
        fflush(stdout);
        
        // receive SPI raw blocks with track samples
        if(config.verbose>0) {
            printf("\n  [waiting for raw blocks... wait_blocks=%d max_blocks=%d]\n", config.wait_blocks, config.max_blocks);
        }
        uint8_t *raw_blocks;
        uint32_t num_blocks;
        error = spi_bulk_read_raw_blocks(spi, config.wait_blocks, config.max_blocks, &raw_blocks, &num_blocks);

        // reading from SPI failed!
        if(error < 0) {
            printf("READ ERROR: %s (got %d blocks)\n", spi_bulk_proto_error_string(error), num_blocks);
            if(raw_blocks != NULL) {

            	/* write error dump */
            	FILE *fh = fopen("error.dump","w");
                if(fh != NULL) {
                     if(config.verbose) {
                         printf("  [writing 'error.dump']\n");
                     }
                     fwrite(raw_blocks, num_blocks * SPI_BLOCK_SIZE,1,fh);
                     fclose(fh);
                 }

            	free(raw_blocks);
            }
            error = -1;
            break;
        }
        
        // decode blocks
        uint8_t *data;
        uint32_t max_frame_size;
        int size = spi_bulk_decode_raw_blocks(raw_blocks, config.max_blocks, &data, &max_frame_size);
        if(size <= 0) {
            printf("DECODE ERROR: %s\n", spi_bulk_proto_error_string(size));
            error = -2;
            break;
        }
        if(config.verbose) {
            printf("  [decoded %d/%x bytes, max frame size: %d]\n", size, size, max_frame_size);
        }
        free(raw_blocks);

        // get status
        if(check_status(spi, size)<0) {
        	printf("SAMPLER FAILED!\n");
        	error = -3;
        	break;
        }

        // write raw track data
        FILE *fh = fopen(name,"w");
        if(fh != NULL) {
            if(config.verbose) {
                printf("  [writing track to file '%s']\n", name);
            }
            fwrite(data,size,1,fh);
            fclose(fh);
        } else {
            printf("ERROR writing to '%s'\n", name);
            error = -3;
            break;
        }
        free(data);
        
        // ready?
        if(i == (num_tracks-1))
            break;
        
        // next track
        if(config.sides == SIDES_ALL) {
            // toggle side
            if(i % 2 == 0) {
                error = (control_floppy(spi,"t") != 1) ? -1 : 0;
                side = 1;
            } else {
                error = (control_floppy(spi,"b+01") != 2) ? -1 : 0;
                side = 0;
                t++;
            }
        } else {
            error = (control_floppy(spi,"+01") != 1) ? -1 : 0;
            t++;
        }
        if(error<0) {
            break;
        }
    }
    
    result = stop_floppy(spi);
    if(result < 0) {
        return result;
    }
    
    if(error==0) {
        printf("\nwrote %d tracks successfully.\n", num_tracks);
    } else {
        printf("\nFAILED in track %02d.%d!\n",t, side);
    }
    return 0;
}

int read_trk(spi_t *spi)
{
    int result;

    result = start_floppy(spi);
    if(result < 0) {
        return result;
    }

    printf("reading track %d side %c\n", config.begin_track, config.sides);
    
    if(config.verbose>0) {
        printf("  [waiting for raw blocks... wait_blocks=%d max_blocks=%d]\n", config.wait_blocks, config.max_blocks);
    }
    uint8_t *raw_blocks;
    uint32_t num_blocks;
    result = spi_bulk_read_raw_blocks(spi, config.wait_blocks, config.max_blocks, &raw_blocks, &num_blocks);

    result = stop_floppy(spi);
    if(result < 0) {
        return result;
    }

    if(num_blocks <= 0) {
        printf("READ ERROR: %s\n", spi_bulk_proto_error_string(num_blocks));
        return num_blocks;
    }

    printf("track data: received %d blocks, %d bytes\n", num_blocks, num_blocks * 4096);
    
    // write raw (debug) data
    if(config.num_args>1) {
        const char *raw_output = config.args[1];
        FILE *fh = fopen(raw_output,"w");
        if(fh != NULL) {
            printf("writing to raw data to '%s'\n", raw_output);
            fwrite(raw_blocks,num_blocks * 4096,1,fh);
            fclose(fh);
        } else {
            printf("ERROR writing to '%s'\n", raw_output);
        }
    }

    // decode blocks
    uint8_t *data;
    uint32_t max_frame_size;
    int size = spi_bulk_decode_raw_blocks(raw_blocks, config.max_blocks, &data, &max_frame_size);
    if(size <= 0) {
        printf("DECODE ERROR: %s\n", spi_bulk_proto_error_string(size));
        return -2;
    }
    if(config.verbose) {
        printf("  [decoded %d/%x bytes, max frame size: %d]\n", size, size, max_frame_size);
    }
    free(raw_blocks);

    // check status
    if(check_status(spi, size)<0) {
    	printf("SAMPLER FAILED\n");
    	return -3;
    }

    // write data
    const char *output_file = config.args[0];
    FILE *fh = fopen(output_file,"w");
    if(fh != NULL) {
        if(config.verbose) {
            printf("writing to track to '%s'\n", output_file);
        }
        fwrite(data,size,1,fh);
        fclose(fh);
    } else {
        printf("ERROR writing to '%s'\n", output_file);
    }
    
    free(data);
    return 0;
}
