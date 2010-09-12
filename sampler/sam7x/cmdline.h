#ifndef CMDLINE_H
#define CMDLINE_H

#include "board.h"

#define CMDLINE_LEN     80

extern u08 cmdline[CMDLINE_LEN];

extern u32 cmdline_getline(void);

#endif
