#include "cmd.h"

#include "floppy-low.h"
#include "uartutil.h"

void cmd_floppy_enable(void)
{
    floppy_select_on();    
}

void cmd_floppy_disable(void)
{
    floppy_select_off();
}

void cmd_motor_on(void)
{
    floppy_motor_on();
}

void cmd_motor_off(void)
{
    floppy_motor_off();
}

void cmd_seek_zero(void)
{
    floppy_seek_zero();
}

void cmd_step_out(u08 num)
{
    floppy_step_n_tracks(DIR_OUTWARD, num);

    uart_send_string((u08 *)"step out: ");
    uart_send_hex_byte_crlf(num);
}

void cmd_step_in(u08 num)
{
    floppy_step_n_tracks(DIR_INWARD, num);

    uart_send_string((u08 *)"step in: ");
    uart_send_hex_byte_crlf(num);
}

u08 cmd_is_track_zero(void)
{
    return floppy_is_track_zero();
}

void cmd_side_top(void)
{
    floppy_set_side(SIDE_TOP);
}

void cmd_side_bottom(void)
{
    floppy_set_side(SIDE_BOTTOM);
}
