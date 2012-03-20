#ifndef INDEX_H
#define INDEX_H

#include "board.h"

// max number of indexes per track
#define MAX_INDEX      8

extern void index_reset(void);

extern void index_set_max_index(u32 num_index);
extern u32 index_get_max_index(void);

extern u32 index_get_total(void);
extern u32 index_get_index(u32 i);

extern void index_add(u32 pos);

extern void index_info(void);

#endif

