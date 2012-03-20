#ifndef TRK_READ_H
#define TRK_READ_H

#include "board.h"

#define MARKER_INDEX            0x01
#define MARKER_WORD_VALUE       0x02
#define MARKER_OVERRUN          0x03
#define MARKER_TIMER_OVERFLOW   0x04
#define MARKER_LAST             0x04

u32  trk_read_count_index(void);
u32  trk_read_count_data(void);
u32  trk_read_data_spectrum(int verbose);

u08  trk_read_to_spiram(int verbose);
u08  trk_read_sim(int verbose);
u08  trk_check_spiram(int verbose);

void trk_read_set_max_index(u32 num_index);
u32  trk_read_get_num_index(void);
u32  trk_read_get_index_pos(u32 i);

#endif
