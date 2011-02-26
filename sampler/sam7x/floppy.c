#include "floppy.h"
#include "floppy-low.h"

static u08 status = 0;

u08 floppy_select_on(void)
{
  if((status & FLOPPY_STATUS_SELECT) == 0) {
      floppy_low_select_on();
      status |= FLOPPY_STATUS_SELECT;
      return 1;
  } else {
      return 0;
  }
}

u08 floppy_select_off(void)
{
  if((status & FLOPPY_STATUS_SELECT) == FLOPPY_STATUS_SELECT) {
      floppy_low_select_off();
      status &= ~FLOPPY_STATUS_SELECT;
      return 1;
  } else {
      return 0;
  }
}

u08 floppy_motor_on(void)
{
  if((status & FLOPPY_STATUS_MOTOR) == 0) {
      floppy_low_motor_on();
      status |= FLOPPY_STATUS_MOTOR;
      return 1;
  } else {
      return 0;
  }
}

u08 floppy_motor_off(void)
{
  if((status & FLOPPY_STATUS_MOTOR) == FLOPPY_STATUS_MOTOR) {
      floppy_low_motor_off();
      status &= ~FLOPPY_STATUS_MOTOR;
      return 1;
  } else {
      return 0;
  }
}

u08 floppy_status(void)
{
  return status;
}
