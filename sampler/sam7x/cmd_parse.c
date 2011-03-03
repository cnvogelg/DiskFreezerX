#include "cmd_parse.h"

#include "util.h"
#include "trk_read.h"
#include "file.h"
#include "uart.h"
#include "uartutil.h"
#include "floppy.h"
#include "track.h"
#include "memory.h"
#include "disk.h"

#define CMD_RES_OK              0
#define CMD_RES_SYNTAX_ERROR    1

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
  uart_send_crlf();
  return rx_size;
}

static u08 get_char(void)
{
  if(in_size == 0) {
      return 0;
  }
  u08 ret = *in;
  in++;
  in_size--;
  return ret;
}

static void set_result(u08 val)
{
  byte_to_hex(val, out);
  out += 2;
  out_size += 2;
}

#if 0
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
#endif

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

// floppy commands
static void cmd_floppy(void)
{
  u08 cmd;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
    switch(cmd) {
    case 'e':
      floppy_select_on();
      set_result(CMD_RES_OK);
      break;
    case 'd':
      floppy_select_off();
      set_result(CMD_RES_OK);
      break;
    case 'o':
      floppy_motor_on();
      set_result(CMD_RES_OK);
      break;
    case 'f':
      floppy_motor_off();
      set_result(CMD_RES_OK);
      break;
    case '?':
      set_result(floppy_status());
      break;
    default:
      set_result(CMD_RES_SYNTAX_ERROR);
    case '.':
      exit = 1;
      break;
    }
    if(exit) break;
  }
}

// track commands
static void cmd_track(void)
{
  // ensure floppy is selected
  u08 did_sel = floppy_select_on();

  u08 cmd,res;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
    switch(cmd) {
    case 'i':
      track_init();
      set_result(CMD_RES_OK);
      break;
    case 'c':
      res = track_check_max();
      set_result(res);
      break;
    case 'z':
      track_zero();
      set_result(CMD_RES_OK);
      break;
    case '?':
      set_result(track_num());
      break;
    case '+':
      track_step_next(parse_hex_byte(1));
      set_result(track_num());
      break;
    case '-':
      track_step_prev(parse_hex_byte(1));
      set_result(track_num());
      break;
    case 'n':
      track_next(parse_hex_byte(1));
      set_result(track_num());
      break;
    case 'p':
      track_prev(parse_hex_byte(1));
      set_result(track_num());
      break;
    case 's':
      track_side_toggle();
      set_result(track_num());
      break;
    case 't':
      track_side_top();
      set_result(track_num());
      break;
    case 'b':
      track_side_bot();
      set_result(track_num());
      break;
    case 'm':
      track_set_max(parse_hex_byte(79));
      set_result(track_get_max());
      break;
    default:
      set_result(CMD_RES_SYNTAX_ERROR);
    case '.':
      exit = 1;
      break;
    }
    if(exit) break;
  }

  if(did_sel) {
      floppy_select_off();
  }
}

// memory commands
static void cmd_memory(void)
{
  u08 cmd,res;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
     case 'i':
       res = memory_init();
       set_result(res);
       break;
     case 'c':
       res = memory_check(parse_hex_byte(0));
       set_result(res);
       break;
     case 'd':
       memory_dump(parse_hex_byte(0),parse_hex_byte(0));
       set_result(CMD_RES_OK);
       break;
     default:
       set_result(CMD_RES_SYNTAX_ERROR);
     case '.':
       exit = 1;
       break;
     }
     if(exit) break;
   }
}

// io commands
static void cmd_io(void)
{
  u08 cmd, res;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
     case 'd':
       file_dir();
       break;
     case 's':
       {
         read_status_t *rs = trk_read_get_status();
         u32 size = rs->data_size;
         u32 check = rs->data_checksum;
         u08 t = rs->track_num;
         if(size > 0) {
             res = file_save(t,size,check,parse_hex_byte(1));
             set_result(res);
         } else {
             set_result(0);
         }
       }
       break;
     case 'f': // find next disk name dir
       {
         u32 num = file_find_disk_dir();
         set_result(num & 0xff);
       }
       break;
     case 'm': // make disk dir
       {
         res = file_make_disk_dir(parse_hex_byte(0));
         set_result(res);
       }
       break;
     default:
       set_result(CMD_RES_SYNTAX_ERROR);
     case '.':
       exit = 1;
       break;
     }
     if(exit) break;
   }
}

// sampler commands
static void cmd_sampler(void)
{
  u08 cmd, res;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
     case 'm':
       {
         u08 sel = floppy_select_on();
         u08 mot = floppy_motor_on();
         track_init();
         res = trk_read_to_spiram(parse_hex_byte(1));
         if(mot) floppy_motor_off();
         if(sel) floppy_select_off();
         set_result(res);
       }
       break;
     case 'f':
       res = trk_read_sim(parse_hex_byte(1));
       set_result(res);
       break;
     case 'v':
       res = trk_check_spiram(parse_hex_byte(1));
       set_result(res);
       break;
     default:
       set_result(CMD_RES_SYNTAX_ERROR);
     case '.':
       exit = 1;
       break;
     }
     if(exit) break;
   }
}


void cmd_parse(u08 len, const u08 *buf, u08 *result_len, u08 *res_buf)
{
  in = buf;
  in_size = len;
  out = res_buf;
  out_size = 0;

  u08 res;
  while(in_size > 0) {
      u08 cmd = get_char();

      switch(cmd) {
      // f) floppy commands
      case 'f':
        cmd_floppy();
        break;

      // t) track commands
      case 't':
        cmd_track();
        break;

      // m) memory commands
      case 'm':
        cmd_memory();
        break;

      // i) io commands
      case 'i':
        cmd_io();
        break;

      // r) sampler commands
      case 'r':
        cmd_sampler();
        break;

      // ----- TASKS -----
      // R) read disk
      case 'R':
        res = disk_read_all(parse_hex_byte(0),parse_hex_byte(160));
        set_result(res);
        break;
      }
  }

  *result_len = out_size;
}
