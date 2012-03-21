// buffer.h

#ifndef BUFFER_H
#define BUFFER_H

#include "board.h"
#include "error.h"

typedef error_t (*io_func)(const u08 *buffer, u32 size);

extern void buffer_clear(void);
extern void buffer_set(u08 track, u32 size, u32 checksum);

extern u32 buffer_get_size(void);
extern u08 buffer_get_track(void);
extern u32 buffer_get_checksum(void);

extern error_t buffer_write(io_func write_func);

extern void buffer_info(void);

#endif
