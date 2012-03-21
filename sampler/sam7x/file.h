#ifndef FILE_H
#define FILE_H

#include "board.h"
#include "error.h"

error_t file_dir(void);
error_t file_save_buffer(int verbose);
error_t file_find_disk_dir(u32 *num);
error_t file_make_disk_dir(u32 num);

void file_test(void);

#endif

