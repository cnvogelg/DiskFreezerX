#ifndef SPIRAM_H
#define SPIRAM_H

#include "board.h"
#include "spi.h"

/* RAM size */
#define SPIRAM_SIZE             32768

#define SPIRAM_MODE_BYTE        0x00
#define SPIRAM_MODE_PAGE        0x80
#define SPIRAM_MODE_SEQ         0x40

extern void spiram_init(void);
extern void spiram_close(void);

extern u08  spiram_set_mode(u08 mode);
extern void spiram_write_begin(u16 address);
extern void spiram_read_begin(u16 address);

extern u32 spiram_test(u08 begin,u16 size);

__inline void spiram_write_byte(u08 data)
{ spi_io(data); }

__inline void spiram_write_byte_last(u08 data)
{ spi_io_last(data); }

__inline u08 spiram_read_byte(void)
{ return spi_io(0xff); }

__inline u08 spiram_read_byte_last(void)
{ return spi_io_last(0xff); }

#endif
