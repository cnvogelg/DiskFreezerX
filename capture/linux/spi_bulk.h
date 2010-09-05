/* spi_bulk.h - spi bulk transfers */

#ifndef SPI_BULK_H
#define SPI_BULK_H

#include <stdint.h>
#include "spi.h"

#define SPI_BLOCK_SIZE      4096

#define ERR_NOBOF           4
#define ERR_RX              3
#define ERR_NOEOT           2
#define ERR_NOMEM           1
#define ERR_NONE            0

// protocol codes
#define SPI_BULK_EOT        0xff
#define SPI_BULK_EOF        0xfe
#define SPI_BULK_BOF        0xfd

extern const char *spi_bulk_error_string(int code);

extern int spi_bulk_read_raw_blocks(spi_t *spi,uint32_t wait_start,uint32_t max_blocks,
                                    uint8_t **result);

extern int spi_bulk_decode_raw_blocks(const uint8_t *data,uint32_t max_blocks,
                                      uint8_t **result, uint32_t *ret_max_frame_size);

#endif
