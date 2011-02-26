#ifndef FLOPPY_H
#define FLOPPY_H

#include "board.h"

#define FLOPPY_STATUS_NONE      0
#define FLOPPY_STATUS_SELECT    1
#define FLOPPY_STATUS_MOTOR     2

u08 floppy_select_on(void);
u08 floppy_select_off(void);
u08 floppy_motor_on(void);
u08 floppy_motor_off(void);
u08 floppy_status(void);

#endif
