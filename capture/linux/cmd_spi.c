#include <string.h>
#include <unistd.h>
#include "cmd_spi.h"

// READY state is marked by %10xxxxxx bits
#define READY_MASK      0xc0
#define READY_FLAG      0x80
#define SIZE_MASK       0x3f

// CMD
#define CMD_NONE        0x00
#define CMD_TX          0x01
#define CMD_RX          0x02

static char buffer[256];

static int wait_for_ready(spi_t *spi,int timeout_ms,int *result_size)
{
    int i;
    
    /* wait for READY status with timeout */
    for(i=0;i<timeout_ms;i++) {
        int result = spi_transmit(spi,buffer,NULL,2);
        if(result < 0) {
            return result;
        }

        //printf("%02x %02x\n", (int)buffer[0], (int)buffer[1]);

        // found READY ?
        if((buffer[1] & READY_MASK) == READY_FLAG) {
            if(result_size != NULL) {
                *result_size = (int)(buffer[1] & SIZE_MASK);
            }
            return 0;
        }
        
        usleep(1000);
    }
    return -CMD_ERR_NOT_READY;
}

int cmd_spi_tx(spi_t *spi,const char *data, size_t len, int timeout_ms)
{
    if(len > CMD_MAX_SIZE)
        return -CMD_ERR_TOO_LONG;
    
    /* wait for ready from slave */
    int result = wait_for_ready(spi, timeout_ms, NULL);
    if(result < 0) {
        return result;
    }
    
    /* setup TX command header with data */
    buffer[0] = CMD_TX;
    buffer[1] = (char)(len & 0xff);
    memcpy(buffer+2,data,len);
    
    /* send to slave */
    return spi_transmit(spi,NULL,buffer,len+2);
}

int cmd_spi_rx(spi_t *spi,char *data, size_t max_len, int timeout_ms)
{
    int i;

    /* wait for ready from client */
    int size;
    int result = wait_for_ready(spi, timeout_ms, &size);
    if(result < 0) {
        return result;
    }

    /* nothing to read */
    if(size == 0) {
        return 0;
    }

    /* send RX data command */
    buffer[0] = CMD_RX;
    buffer[1] = (char)(size);
    result = spi_transmit(spi,NULL,buffer,2);
    if(result < 0) {
        return result;
    }

    /* receive data */
    result = spi_transmit(spi,data,NULL,size);
    if(result < 0) {
        return result;
    }
    
    return size;
}
