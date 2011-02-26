#ifndef TRACK_H
#define TRACK_H

#include "board.h"

void track_init(void);
void track_zero(void);

u08  track_check_max(void);
void track_set_max(u08 max);
u08  track_get_max(void);

u08  track_num(void);
u08 *track_name(u08 num);

void track_step_next(u08 num);
void track_step_prev(u08 num);

u08  track_side_toggle(void);
void track_side_bot(void);
void track_side_top(void);

void track_next(u08 num);
void track_prev(u08 num);

#endif
