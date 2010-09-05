/* spi.c */

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <fcntl.h>
#include <string.h>

#include "spi.h"

spi_t *spi_open(const char *name, uint32_t clk, uint32_t mode, uint32_t max_size)
{
    /* open device */
    int fd = open(name, O_RDWR);
    if (fd < 0)
        return NULL;

    /* determine SPI mode */
    uint32_t mode_flag = 0;
    if(mode & 1)
        mode_flag |= SPI_CPHA;
    if(mode & 2)
        mode_flag |= SPI_CPOL;

    /* set spi mode */
    int ret = ioctl(fd, SPI_IOC_WR_MODE, &mode_flag);
    if (ret < 0)
        goto close;

    /* set 8 bits per word */
    uint32_t bits = 8;
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0)
        goto close;

    /* set SPI clock */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &clk);
    if (ret < 0)
        goto close;

    spi_t *spi = (spi_t *)malloc(sizeof(spi_t));
    if(spi == NULL)
        return NULL;
    
    /* setup SPI struct */
    spi->fd   = fd;
    spi->clk  = clk;
    spi->mode = mode_flag;

    /* allocate empty buffer */
    spi->max_size = max_size;
    spi->zero = (char *)malloc(max_size);
    if(spi->zero == NULL) {
        goto close;
    }
    memset(spi->zero, 0, max_size);
    
    /* allocate dummy buffer */
    spi->dummy = (char *)malloc(max_size);
    if(spi->dummy == NULL) {
        free(spi->zero);
        goto close;
    }

    return spi;
    
close:
    close(fd);
    return NULL;
}

int spi_transmit(spi_t *spi,void *rx_data, void *tx_data, size_t size)
{
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)(tx_data == NULL ? spi->zero : tx_data),
        .rx_buf = (unsigned long)(rx_data == NULL ? spi->dummy : rx_data),
        .len = size,
        .delay_usecs = 0,
        .speed_hz = spi->clk,
        .bits_per_word = 8,
    };
    int ret = ioctl(spi->fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret == 1)
        return -1;
    else
        return 0;
}

void spi_close(spi_t *spi)
{
    if(spi == NULL)
        return;
        
    if(spi->zero != NULL) {
        free(spi->zero);
    }
    if(spi->dummy != NULL) {
        free(spi->dummy);
    }
    
    close(spi->fd);
    free(spi);
}

