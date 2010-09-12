#ifndef TRK_READ_H
#define TRK_READ_H

#include "board.h"

#define MARKER_INDEX            0xfc
#define MARKER_OVERFLOW         0xfb
#define LAST_VALUE              0xfa

u32  trk_read_count_index(void);
u32  trk_read_count_data(void);
u32  trk_read_data_spectrum(void);

void trk_read_dummy(u32 num);
void trk_read_real(void);

#endif
