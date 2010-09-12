#ifndef CMD_H
#define CMD_H

#include "board.h"

void cmd_floppy_enable(void);
void cmd_floppy_disable(void);

void cmd_motor_on(void);
void cmd_motor_off(void);

void cmd_seek_zero(void);
void cmd_step_out(u08 num);
void cmd_step_in(u08 num);
u08  cmd_is_track_zero(void);

void cmd_side_top(void);
void cmd_side_bottom(void);

#endif