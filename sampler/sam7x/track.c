#include "track.h"
#include "floppy-low.h"

#define MAX_TRACK       79

static u08 track;
static u08 side;
static u08 max_track = MAX_TRACK;
static u08 is_init = 0;

// offset                   0123456789
static u08 *name = (u08 *)"t_xx_y.trk";

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
  track = 0;

  return max_track;
}

u08 track_get_max(void)
{
  return max_track;
}

void track_set_max(u08 max)
{
  if(max > 85)
    max = 85;

  max_track = max;
}

u08 track_num(void)
{
  return (track) << 1 | side;
}

u08 *track_name(u08 num)
{
  u08 t = num >> 1;
  u08 s = num & 1;

  name[5] = s ? '1':'0';

  int td = t / 10;
  int tn = t % 10;
  name[2] = td + '0';
  name[3] = tn + '0';

  return name;
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

void track_next(u08 num)
{
  while(num > 0) {
    u08 s = track_side_toggle();
    if(s == 0) {
        track_step_next(1);
    }
    num--;
  }
}

void track_prev(u08 num)
{
  while(num > 0) {
    u08 s = track_side_toggle();
    if(s == 1) {
        track_step_prev(1);
    }
    num--;
  }
}
