#ifndef STATUS_H
#define STATUS_H

#include "board.h"

typedef struct {
  u32 samples;
  u32 cell_overruns;
  u32 word_values;
  u32 timer_overflows;
  u32 data_overruns;
} status_t;

extern status_t status;

extern void status_reset(void);
extern void status_info(void);

#endif

