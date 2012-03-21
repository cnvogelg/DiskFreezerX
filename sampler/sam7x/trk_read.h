#ifndef TRK_READ_H
#define TRK_READ_H

#include "board.h"
#include "error.h"

#define MARKER_INDEX            0x01
#define MARKER_WORD_VALUE       0x02
#define MARKER_OVERRUN          0x03
#define MARKER_TIMER_OVERFLOW   0x04
#define MARKER_LAST             0x04

error_t trk_read_count_index(void);
error_t trk_read_count_data(void);
error_t trk_read_data_spectrum(int verbose);

error_t trk_read_to_spiram(int verbose);
error_t trk_read_sim(int verbose);
error_t trk_check_spiram(int verbose);

#endif
