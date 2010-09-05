/* spi.h
   low level SPI access via spidev device on Linux
*/

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int fd;
    uint32_t clk;
    uint32_t mode;
    char *zero;
    char *dummy;
    uint32_t max_size;
} spi_t;

/*
    \brief open and configure spidev device
    \param name     name of SPI device e.g. /dev/spidev4.0
    \param clk      speed of SPI clock in Hz
    \param mode     SPI mode 0..3
    \return         >=0 file handle <0 error
*/
extern spi_t *spi_open(const char *name, uint32_t clk, uint32_t mode, uint32_t max_size);

/*
    \brief read an SPI message block
    \param spi      opened SPI device
    \param rx_data  pointer to received data
    \param tx_data  poitner to send
    \param size     number of bytes to read
    \return -1=error 0=ok
*/
extern int spi_transmit(spi_t *spi,void *rx_data, void *tx_data, size_t size);

/*
    \brief close SPI defice
    \param spi      opened SPI device
*/
extern void spi_close(spi_t *spi);

#endif
