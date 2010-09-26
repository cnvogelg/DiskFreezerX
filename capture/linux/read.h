#ifndef READ_H
#define READ_H

typedef struct {
	uint32_t 	index_overruns;
	uint32_t	cell_overruns;
	uint32_t	cell_overflows;
	uint32_t	data_size;
	uint32_t	data_overruns;
} read_status_t;

int read_trk(spi_t *spi);
int read_dsk(spi_t *spi);

#endif
