#ifndef TRK_READ_H
#define TRK_READ_H

#include "board.h"

typedef struct {
  u32 index_overruns;
  u32 cell_overruns;
  u32 cell_overflows;
  u32 data_size;
  u32 data_overruns;
} read_status_t;

#define MARKER_INDEX            0xfc
#define MARKER_OVERFLOW         0xfb
#define LAST_VALUE              0xfa

u32  trk_read_count_index(void);
u32  trk_read_count_data(void);
u32  trk_read_data_spectrum(void);

void trk_read_dummy(u32 num);
void trk_read_real(void);

void trk_read_set_max_index(u32 num_index);
u32  trk_read_get_num_index(void);
u32  trk_read_get_index_pos(u32 i);

read_status_t *trk_read_get_status(void);

#endif
