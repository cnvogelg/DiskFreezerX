// sdpin.h
// read pins from SD card: card inserted, write protect

#ifndef SDPIN_H
#define SDPIN_H

#include "target.h"
#include "board.h"

extern void sdpin_init(void);
extern int  sdpin_no_card(void);
extern int  sdpin_write_protect(void);

#endif
