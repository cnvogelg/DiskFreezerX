#include "track.h"
#include "floppy-low.h"

#define MAX_TRACK       79

static u08 track;
static u08 side;
static u08 max_track = MAX_TRACK;
static u08 is_init = 0;

void track_init(void)
{
  if(!is_init) {
    max_track = MAX_TRACK;
    is_init = 1;
    track_zero();
  }
}

void track_zero(void)
{
  side = 0;
  track = 0;

  floppy_low_seek_zero();
  floppy_low_set_side(SIDE_BOTTOM);
}

u08 track_check_max(void)
{
  floppy_low_seek_zero();
  floppy_low_step_n_tracks(DIR_INWARD, 84);
  u08 max_steps = floppy_low_seek_zero();
  max_track = max_steps - 1;
  return max_steps;
}

void track_set_max(u08 max)
{
  if((max < 1)||(max > 85))
    return;

  max_track = max - 1;
}

u08 track_status(void)
{
  return (track) << 1 | side;
}

void track_step_next(u08 n)
{
  if((track + n) <= max_track) {
      track+=n;
      floppy_low_step_n_tracks(DIR_INWARD,n);
  }
}

void track_step_prev(u08 n)
{
  if(track >= n) {
      track-=n;
      floppy_low_step_n_tracks(DIR_OUTWARD,n);
  }
}

u08 track_side_toggle(void)
{
  if(side == 0) {
      side = 1;
      floppy_low_set_side(SIDE_TOP);
  } else {
      side = 0;
      floppy_low_set_side(SIDE_BOTTOM);
  }
  return side;
}

void track_side_bot(void)
{
  side = 0;
  floppy_low_set_side(SIDE_BOTTOM);
}

void track_side_top(void)
{
  side = 1;
  floppy_low_set_side(SIDE_TOP);
}

void track_next(void)
{
  u08 s = track_side_toggle();
  if(s == 0) {
      track_step_next(1);
  }
}

void track_prev(void)
{
  u08 s = track_side_toggle();
  if(s == 1) {
      track_step_prev(1);
  }
}
