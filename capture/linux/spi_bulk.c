#include <stdlib.h>
#include <stdio.h>

#include "spi_bulk.h"
#include "spi.h"

#define SPI_MODE        1
#define CMD_TRACK_READ  0x03

#define MAX_CODE        5  
static const char *error_string[MAX_CODE] = {
    "no error",
    "out of memory",
    "no end of transmissiton (EOT) found",
    "receive error",
    "no begin fo frame (BOF) found"
};

const char *spi_bulk_proto_error_string(int code)
{
    int offset = -code;
    if( (offset < 0) || (offset >= MAX_CODE))
        return NULL;
    
    return error_string[offset];
}

int spi_bulk_read_raw_blocks(spi_t *spi,uint32_t wait_start,uint32_t max_blocks,
							 uint8_t **result, uint32_t *result_num_blocks)
{
    // get memory for all blocks
    uint8_t *block_data = (uint8_t *)malloc(SPI_BLOCK_SIZE * max_blocks);    
    if(block_data == NULL) {
        return -ERR_NOMEM;
    }
    
    // receive loop
    *result = block_data;
    *result_num_blocks = 0;
    uint32_t num_blocks = 0;
    uint32_t i,j;
    uint8_t *ptr;
    uint8_t code;
    int found = 0;
    
    // send TRACK_READ command
    char cmd[2] = { 0, CMD_TRACK_READ };
    if(spi_transmit(spi, NULL, cmd, 2)) {
        free(block_data);
        *result = NULL;
        return -ERR_RX;
    }
    
    // wait for BOF marker in first block
    *result_num_blocks = 1;
    for(i=0;i<wait_start;i++) {

        // reset pointer in each try
        ptr = block_data;
        
        // read block via spi
        if(spi_transmit(spi,ptr,NULL,SPI_BLOCK_SIZE)) {
            return -ERR_RX;
        }

        // check if block contains BOF marker
        for(j=0;j<SPI_BLOCK_SIZE;j++) {
            code = *(ptr++);
            if(code == SPI_BULK_BOF) {
                found = 1;
                break;
            }
        }
        if(found)
            break;
    }
    
    // no BOF marker found
    if(!found) {
        return -ERR_NOBOF;
    }
    
    // commence with next block
    ptr = block_data + SPI_BLOCK_SIZE;
    num_blocks++;
    
    for(i=1;i<max_blocks;i++) {
        // read block via spi
        if(spi_transmit(spi,ptr,NULL,SPI_BLOCK_SIZE)) {
            return -ERR_RX;
        }
        
        num_blocks++;
        *result_num_blocks = num_blocks;

        // check if block contains EOT marker
        for(j=0;j<SPI_BLOCK_SIZE;j++) {
            uint8_t code = *(ptr++);
            if(code == SPI_BULK_EOT) {
                return ERR_NONE;
            }
        }
    }

    // too many blocks without EOT received
    return -ERR_NOEOT;
}

static int spi_bulk_calc_decoded_size(const uint8_t *data,uint32_t max_blocks,const uint8_t **begin)
{
    const uint8_t *ptr = data;
    int size = 0;
    int max_size = SPI_BLOCK_SIZE * max_blocks;
    const uint8_t *endptr = data + max_size;
    uint8_t code;
    int i;
    int on;
    
    // find BOT marker. needs to be in first block
    for(i=0;i<SPI_BLOCK_SIZE;i++) {
        code = *(ptr++);
        if(code == SPI_BULK_BOF)
            break;
    }
    
    // no BOT found!
    if(i==SPI_BLOCK_SIZE) {
        return -ERR_NOBOF;
    }
    
    // store begin
    *begin = ptr;
    
    // now find EOT marker
    on = 1;
    while(ptr < endptr) {
        code = *(ptr++);
        if(code == SPI_BULK_EOT)
            return size;
        else if(code == SPI_BULK_BOF)
            on = 1;
        else if(code == SPI_BULK_EOF)
            on = 0;
        else if(on)        
            size++;
    }
    
    return -ERR_NOEOT;
}

int spi_bulk_decode_raw_blocks(const uint8_t *raw_data, uint32_t max_blocks,
                               uint8_t **result, uint32_t *ret_max_frame_size)
{
    const uint8_t *raw_ptr;
    int size = spi_bulk_calc_decoded_size(raw_data, max_blocks, &raw_ptr);
    if(size <= 0)
        return size;
     
    // buffer for decoded data   
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t)*size);
    if(data == NULL) {
        return -ERR_NOMEM;
    }
    
    uint8_t *ptr = data;
    uint8_t code = *raw_ptr;
    int on = 1;
    uint32_t frame_size = 0;
    uint32_t max_frame_size = 0;
    while(code != SPI_BULK_EOT) {
        code = *(raw_ptr++);
        if(code == SPI_BULK_EOT) {
            break;
        } else if(code == SPI_BULK_BOF) {
            on = 1;
            frame_size = 0;
        } else if(code == SPI_BULK_EOF) {
            on = 0;
            if(frame_size > max_frame_size) {
                max_frame_size = frame_size;
            }
        } else if(on) {
            *(ptr++) = code;
            frame_size ++;
        }
    }
    
    // store max frame size achieved
    if(ret_max_frame_size != NULL) {
        *ret_max_frame_size = max_frame_size;
    }
    
    *result = data;
    return size;
}

