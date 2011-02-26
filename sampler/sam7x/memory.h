#ifndef MEMORY_H
#define MEMORY_H

#include "board.h"

u08  memory_init(void);
u08  memory_check(u08 mode);
void memory_dump(u08 chip_no,u08 bank);

#endif
