#include "cmd_parse.h"
#include "cmd.h"
#include "util.h"
#include "trk_read.h"
#include "spiram.h"
#include "file.h"
#include "uart.h"

#define CMD_RES_ABORTED         0x80
#define CMD_RES_FAIELD          0x40
#define CMD_RES_SIZE_MASK       0x3f

static const u08 *in;
static u08 *out;
static u08 out_size;
static u08 in_size;

static u08 rx_buf[CMD_MAX_SIZE];
static u08 rx_size;

u08 cmd_uart_get_next(u08 **data)
{
    rx_size = 0;
    while(1) {
        while(!uart_read_ready()) {}
        u08 c;
        uart_read(&c);
        uart_send(c);
        if((c == '\n')||(c == '\r')) {
            break;
        }
        rx_buf[rx_size] = c;
        rx_size++;
    }
    *data = rx_buf;
    return rx_size;
}

static void set_result(u08 val)
{
    byte_to_hex(val, out);
    out += 2;
    out_size += 2;
}

static void set_abort(void)
{
    set_result(CMD_RES_ABORTED);
    in_size = 0;
}

static void set_dword(u32 val)
{
    dword_to_hex(val, out);
    out += 8;
    out_size += 8;
}

static void set_word(u16 val)
{
    word_to_hex(val, out);
    out += 4;
    out_size += 4;
}

static u08 parse_hex_byte(u08 def)
{
    if(in_size < 2)
        return def;
    u08 val;
    if(parse_byte(in,&val)) {
        in_size -= 2;
        in += 2;
        return val;
    } else {
        return def;
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
            num = parse_hex_byte(0xff);
            if(num == 0xff) {
                set_abort();
            } else {
                cmd_step_in(num);
                set_result(0);
            }
            break;
        case '-':
            num = parse_hex_byte(0xff);
            if(num == 0xff) {
                set_abort();
            } else {
                cmd_step_out(num);
                set_result(0);
            }
            break;
        case 'z':
            num = cmd_is_track_zero();
            set_result(1);
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

        case 'i':
            num = parse_hex_byte(5);
            trk_read_set_max_index(num);
            break;
        
        case 'j':
            {
              // return all indices
              u32 num_index = trk_read_get_num_index();
              set_result(num_index * 2);
              for(u32 i=0;i<num_index;i++) {
                  set_word(trk_read_get_index_pos(i));
              }
            }
            break;

        case 'r':
            {
              // read status
              read_status_t * status = trk_read_get_status();
              set_result(5 * 4);
              set_dword(status->index_overruns);
              set_dword(status->cell_overruns);
              set_dword(status->cell_overflows);
              set_dword(status->data_size);
              set_dword(status->data_overruns);
            }
            break;

        case 'y':
          {
            cmd_floppy_enable();
            cmd_motor_on();

            for(int i=0;i<5;i++) {
                trk_read_count_data();
            }

            cmd_motor_off();
            cmd_floppy_disable();
            set_result(0);
          }
          break;

          // test SPI RAM
        case 'M':
          {
              num = parse_hex_byte(0);
              u32 errors = spiram_test(num,SPIRAM_SIZE);
              if(errors > 0) {
                  set_dword(errors);
              }
              set_result((u08)(errors > 0));
          }
          break;
        case 'N':
           {
               num = parse_hex_byte(0);
               u32 errors = spiram_dma_test(num,SPIRAM_SIZE);
               if(errors > 0) {
                   set_dword(errors);
               }
               set_result((u08)(errors > 0));
           }
           break;

           // ----- DUMP MEMORY -----
        case 'R':
          {
              num = parse_hex_byte(0);
              u08 blk = parse_hex_byte(0);
              u32 errors = spiram_dump(num,blk);
              if(errors > 0) {
                  set_dword(errors);
              }
              set_result((u08)(errors > 0));
          }
          break;

          // ----- CLEAR SPI RAM -----
        case 'C':
          {
            num = parse_hex_byte(0);
            int errors = spiram_multi_init();
            if(errors > 0) {
                set_dword(errors);
            }
            errors = spiram_multi_clear(num);
            if(errors > 0) {
                set_dword(errors);
            }
            set_result((u08)(errors > 0));
          }
          break;

          // ----- FILE -----
        case 'F':
          {
            file_test();
          }
          break;
            // ----- Track Read Commands -----
        case 'I':
            {
                cmd_floppy_enable();
                cmd_motor_on();
                u32 count = trk_read_count_index();
                cmd_motor_off();
                cmd_floppy_disable();
                set_result(4);
                set_dword(count);
            }
            break;
        case 'D':
            {
                cmd_floppy_enable();
                cmd_motor_on();
                u32 count = trk_read_count_data();
                cmd_motor_off();
                cmd_floppy_disable();
                set_result(4);
                set_dword(count);
            }
            break;
        case 'S':
            {
                cmd_floppy_enable();
                cmd_motor_on();
                u32 count = trk_read_data_spectrum();
                cmd_motor_off();
                cmd_floppy_disable();
                set_result(4);
                set_dword(count);
            }
            break;
        case 'T':
           {
                cmd_floppy_enable();
                cmd_motor_on();
                trk_read_to_spiram();
                cmd_motor_off();
                cmd_floppy_disable();
                set_result(0);
           }
        }
    }   
    
    *result_len = out_size;
    return stay;
}
