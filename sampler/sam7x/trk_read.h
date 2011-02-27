#ifndef TRK_READ_H
#define TRK_READ_H

#include "board.h"

typedef struct {
  u32 index_overruns;
  u32 cell_overruns;
  u32 cell_overflows;
  u32 timer_overflows;
  u32 data_size;
  u32 data_overruns;
  u32 data_checksum;
  u32 full_chips;
  u08 track_num;
} read_status_t;

#define MARKER_INDEX            0x01
#define MARKER_OVERFLOW         0x02
#define MARKER_OVERRUN          0x03
#define MARKER_TIMER_OVERFLOW   0x04
#define LAST_VALUE              0xff

u32  trk_read_count_index(void);
u32  trk_read_count_data(void);
u32  trk_read_data_spectrum(void);

u08  trk_read_to_spiram(int verbose);
u08  trk_read_sim(int verbose);
u08  trk_check_spiram(int verbose);

void trk_read_set_max_index(u32 num_index);
u32  trk_read_get_num_index(void);
u32  trk_read_get_index_pos(u32 i);

read_status_t *trk_read_get_status(void);

#endif
