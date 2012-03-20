#ifndef FILE_H
#define FILE_H

#include "board.h"

void file_dir(void);
u08  file_save(u08 track, int verbose);
u32  file_find_disk_dir(void);
u08  file_make_disk_dir(u32 num);

void file_test(void);

#endif

