// buffer.h

#ifndef BUFFER_H
#define BUFFER_H

#include "board.h"

typedef int (*io_func)(const u08 *buffer, u32 size);

extern void buffer_set(u08 track, u32 size, u32 checksum);
extern int buffer_write(io_func write_func);

#endif
