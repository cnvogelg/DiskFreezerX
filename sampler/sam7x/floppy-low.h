#ifndef FLOPPY_LOW_H
#define FLOPPY_LOW_H

#include "board.h"

// ---- move direction of head ----
#define DIR_OUTWARD  1
#define DIR_INWARD   0

#define SIDE_TOP     0
#define SIDE_BOTTOM  1

// ---- types ----
typedef void (*func)(void);

// ---- prototypes ----
extern void floppy_low_init(void);

extern void floppy_low_enable_index_intr(func f);
extern void floppy_low_disable_index_intr(void);

#define ACK_INDEX_IRQ AT91C_BASE_PIOA->PIO_ISR;

extern void floppy_low_init_data(void);

extern void floppy_low_select_on(void);
extern void floppy_low_select_off(void);

extern void floppy_low_motor_on(void);
extern void floppy_low_motor_off(void);

extern u32  floppy_low_is_track_zero(void);
extern void floppy_low_set_dir(u32 dir);
extern void floppy_low_step_track(void);
extern void floppy_low_step_n_tracks(u32 dir,u32 n);
extern u08  floppy_low_seek_zero(void);

extern void floppy_low_set_side(u32 dir);

#endif







