#include "cmd_parse.h"
#include "cmd.h"
#include "util.h"
#include "trk_read.h"
#include "uartutil.h"

static const u08 *in;
static u08 *out;
static u08 out_size;
static u08 in_size;

static void set_result(u08 val)
{
    byte_to_hex(val, out);
    out += 2;
    out_size += 2;
}

static void set_dword(u32 val)
{
    dword_to_hex(val, out);
    out += 8;
    out_size += 8;
}

static u08 parse_opt_hex_byte(u08 def_val)
{
    if(in_size == 0)
        return def_val;
    if(*in != ':')
        return def_val;

    in ++;
    in_size --;

    if(in_size == 0)
      return def_val;

    u08 val;
    if(in_size == 1) {
        if(parse_nybble(*in,&val)) {
            in_size --;
            in ++;
            return val;
        } else {
            return def_val;
        }
    } else {
        if(parse_byte(in,&val)) {
            in_size -= 2;
            in += 2;
            return val;
        } else {
            return def_val;
        }
    }
}

u08 cmd_parse(u08 len, const u08 *buf, u08 *result_len, u08 *res_buf)
{
    u08 num;
    u08 stay = 1;
    
    in = buf;
    in_size = len;
    out = res_buf;
    out_size = 0;
    
    while(in_size > 0) {
        u08 cmd = *in;
        in++;
        in_size--;
        
        switch(cmd) {
        case 'e':
            cmd_floppy_enable();
            set_result(0);
            break;
        case 'd':
            cmd_floppy_disable();
            set_result(0);
            break;
        case 'o':
            cmd_motor_on();
            set_result(0);
            break;
        case 'f':
            cmd_motor_off();
            set_result(0);
            break;
        case 's':
            cmd_seek_zero();
            set_result(0);
            break;
        case '+':
            num = parse_opt_hex_byte(1);
            cmd_step_in(num);
            set_result(num);
            break;
        case '-':
            num = parse_opt_hex_byte(1);
            cmd_step_out(num);
            set_result(num);
            break;
        case 'z':
            num = cmd_is_track_zero();
            set_result(num);
            break;
        case 'x':
            stay = 0;
            break;
        case 't':
            cmd_side_top();
            set_result(0);
            break;
        case 'b':
            cmd_side_bottom();
            set_result(0);
            break;
        
            // ----- Track Read Commands -----
        case 'I':
            {
                u32 count = trk_read_count_index();
                set_dword(count);
            }
            break;
        case 'D':
            {
                u32 count = trk_read_count_data();
                set_dword(count);
            }
            break;
        case 'S':
            {
                u32 count = trk_read_data_spectrum();
                set_dword(count);
            }
            break;
        }
    }   
    
    *result_len = out_size;
    return stay;
}
