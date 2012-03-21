#ifndef ERROR_H
#define ERROR_H

#include "board.h"

typedef u08 error_t;

#define STATUS_OK                       0

// command line
#define ERROR_SYNTAX                    0x80

// sampler errors
// (can be OR'ed together)
#define ERROR_SAMPLER_NO_INDEX          0x01
#define ERROR_SAMPLER_CELL_OVERRUN      0x02
#define ERROR_SAMPLER_DATA_OVERRUN      0x04
#define ERROR_SAMPLER_CHECKSUM_MISMATCH 0x08

// file errors
#define ERROR_FILE_WRITE                0x10
#define ERROR_FILE_INIT                 0x11
#define ERROR_FILE_MOUNT                0x12
#define ERROR_FILE_OPEN                 0x13
#define ERROR_FILE_MKDIR                0x14

// memory errors
#define ERROR_MEMORY_INIT               0x20
#define ERROR_MEMORY_CHECKSUM_MISMATCH  0x21

#endif
