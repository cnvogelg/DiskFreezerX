#include <stdio.h>

#include "spi.h"
#include "spi_bulk.h"
#include "config.h"
#include "cmd_parse.h"

static int control_floppy(spi_t *spi,const char *cmd)
{
    if(config.verbose>0) {
        printf("  [control floppy (cmd: '%s')]\n", cmd);
    }        
    int result = cmd_parse_execute(spi, cmd);
    if(config.verbose>0) {
        printf("  [result: %d]\n", result);
    }
    return result;    
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
    char cmd[] = "eosz?+:..";
    cmd[4] = side;
    
    if(config.begin_track>0) {
        sprintf(cmd+7,"%02x",config.begin_track);
    } else {
        cmd[5] = '\0';
    }
    
    int result = control_floppy(spi,cmd);
    
    // check value
    if(result == config.begin_track + 1)
        return 0;
    
    return result;
}

static int stop_floppy(spi_t *spi)
{
    return control_floppy(spi,"fd");
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
        int num_blocks = spi_bulk_read_raw_blocks(spi, config.wait_blocks, config.max_blocks, &raw_blocks);
        if(num_blocks <= 0) {
            printf("READ ERROR: %s\n", spi_bulk_proto_error_string(num_blocks));
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
                error = control_floppy(spi,"t");
                side = 1;
            } else {
                error = control_floppy(spi,"b+");
                side = 0;
                t++;
            }
        } else {
            error = control_floppy(spi,"+");
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
    int num_blocks = spi_bulk_read_raw_blocks(spi, config.wait_blocks, config.max_blocks, &raw_blocks);

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
