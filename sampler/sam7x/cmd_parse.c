#include "cmd_parse.h"

#include "util.h"
#include "trk_read.h"
#include "file.h"
#include "uart.h"
#include "uartutil.h"
#include "floppy-low.h"
#include "floppy.h"
#include "track.h"
#include "memory.h"
#include "disk.h"
#include "button.h"
#include "led.h"
#include "delay.h"
#include "sdpin.h"
#include "rtc.h"
#include "wiz_low.h"
#include "wiz.h"
#include "net.h"

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
  int button_press = 0;
  while(1) {
      while(!uart_read_ready()) {
#ifdef USE_BUTTON_CONTROL
          // check buttons
          if(button1_pressed()) {
              while(button1_pressed()) {}
              rx_buf[0] = 'R';
              rx_size = 1;
              button_press = 1;
              break;
          }
          if(button2_pressed()) {
              while(button2_pressed()) {}
              rx_buf[0] = 'W';
              rx_size = 1;
              button_press = 1;
              break;
          }
#endif
      }
      if(button_press) {
          break;
      }

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

static u16 parse_hex_word(u16 def)
{
  if(in_size < 4)
    return def;
  u08 a = parse_hex_byte(0);
  u08 b = parse_hex_byte(0);
  return (u16)a << 8 | (u16)b;
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
    case 'w': // write protected?
      {
        // ensure floppy is selected
        u08 did_sel = floppy_select_on();
        set_result(floppy_low_is_write_protected());
        if(did_sel) {
            floppy_select_off();
        }
      }
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
     case 't': // test file
       file_test();
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
     case 'm': // read track to spi ram
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
     case 'f': // fake read track (only to spi ram)
       res = trk_read_sim(parse_hex_byte(1));
       set_result(res);
       break;
     case 'v': // verify track with spi ram
       res = trk_check_spiram(parse_hex_byte(1));
       set_result(res);
       break;
       // ----- checks -----
     case 'i': // index check
       {
          u08 sel = floppy_select_on();
          u08 mot = floppy_motor_on();
          track_init();
          res = trk_read_count_index();
          if(mot) floppy_motor_off();
          if(sel) floppy_select_off();
          set_result(res);
       }
       break;
     case 'd': // read data check
       {
          u08 sel = floppy_select_on();
          u08 mot = floppy_motor_on();
          track_init();
          res = trk_read_count_data();
          if(mot) floppy_motor_off();
          if(sel) floppy_select_off();
          set_result(res);
       }
       break;
     case 's': // read data spectrum
       {
          u08 sel = floppy_select_on();
          u08 mot = floppy_motor_on();
          track_init();
          res = trk_read_data_spectrum();
          if(mot) floppy_motor_off();
          if(sel) floppy_select_off();
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

// clock commands
static void cmd_clock(void)
{
  u08 cmd, value;
  u08 exit = 0;
  rtc_time t;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
     case '?': // get time (raw)
       {
         rtc_get(t);
         for(int i=0;i<7;i++) {
             uart_send_hex_byte_space(t[i]);
         }
         uart_send_crlf();
         set_result(0);
       }
       break;
     case 't': // get time
       {
         u08 *str = (u08 *)rtc_get_time_str();
         uart_send_string(str);
         uart_send_crlf();
       }
       break;
     case 'y': // set year
       value = parse_hex_byte(0);
       rtc_set_entry(RTC_INDEX_YEAR, value);
       break;
     case 'o': // set year
       value = parse_hex_byte(1);
       rtc_set_entry(RTC_INDEX_MONTH, value);
       break;
     case 'd': // set day
       value = parse_hex_byte(1);
       rtc_set_entry(RTC_INDEX_DAY, value);
       break;
     case 'h': // set hour
       value = parse_hex_byte(0);
       rtc_set_entry(RTC_INDEX_HOUR, value);
       break;
     case 'i': // set minute
       value = parse_hex_byte(1);
       rtc_set_entry(RTC_INDEX_MINUTE, value);
       break;
     case 's': // set seconds
       value = parse_hex_byte(1);
       rtc_set_entry(RTC_INDEX_SECOND, value);
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

// wiznet commands
static void cmd_wiznet(void)
{
  u08 cmd, res, val;
  u16 addr, wval;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
       // ---------- low level/debug ----------
     case 'r': // low level: read byte <addr/16>
       addr = parse_hex_byte(0) << 8 | parse_hex_byte(0);

       wiz_low_begin();
       res = wiz_low_read(addr);
       wiz_low_end();

       set_result(res);
       uart_send_hex_word_space(addr);
       uart_send_hex_byte_crlf(res);
       break;
     case 'w': // low level: write byte <addr/16><val/8>
       addr = parse_hex_byte(0) << 8 | parse_hex_byte(0);
       val = parse_hex_byte(0);

       wiz_low_begin();
       wiz_low_write(addr,val);
       wiz_low_end();

       set_result(val);
       uart_send_hex_word_space(addr);
       uart_send_hex_byte_crlf(val);
       break;
     case 'R': // low level: read word <addr/16>
       addr = parse_hex_byte(0) << 8 | parse_hex_byte(0);

       wiz_low_begin();
       res = wiz_low_read_word(addr);
       wiz_low_end();

       set_result(res);
       uart_send_hex_word_space(addr);
       uart_send_hex_word_crlf(res);
       break;
     case 'W': // low level: write byte <addr/16><val/8>
       addr = parse_hex_byte(0) << 8 | parse_hex_byte(0);
       wval = parse_hex_byte(0) << 8 | parse_hex_byte(0);

       wiz_low_begin();
       wiz_low_write_word(addr,wval);
       wiz_low_end();

       set_result(wval);
       uart_send_hex_word_space(addr);
       uart_send_hex_word_crlf(wval);
       break;

       // ---------- config ----------
     case '?':
       net_info();
       break;
     case 'm': // set mac addr 6*<8>
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         for(int i=0;i<6;i++) {
             wc->mac_addr[i] = parse_hex_byte(0);
         }
         wiz_realize_cfg();
       }
       break;
     case 's': // set src ip 4*<8>
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         for(int i=0;i<4;i++) {
             wc->src_ip[i] = parse_hex_byte(0);
         }
         wiz_realize_cfg();
       }
       break;
     case 't': // set tgt ip 4*<8>
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         for(int i=0;i<4;i++) {
             wc->tgt_ip[i] = parse_hex_byte(0);
         }
         wiz_realize_cfg();
       }
       break;
     case 'n': // set netmask 4*<8>
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         for(int i=0;i<4;i++) {
             wc->net_msk[i] = parse_hex_byte(0);
         }
         wiz_realize_cfg();
       }
       break;
     case 'g': // set gateway 4*<8>
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         for(int i=0;i<4;i++) {
             wc->gw_ip[i] = parse_hex_byte(0);
         }
         wiz_realize_cfg();
       }
       break;
     case 'o': // src port
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         wc->src_port = parse_hex_word(1234);
       }
       break;
     case 'p': // tgt port
       {
         wiz_cfg_t *wc = wiz_get_cfg();
         wc->tgt_port = parse_hex_word(6800);
       }
       break;

       // ----- load/save config -----
     case 'S': // save to sram
       wiz_save_cfg();
       break;
     case 'L': // load from sram
       wiz_load_cfg();
       wiz_realize_cfg();
       break;

     case 'c': // connect test
       {
         int result = wiz_begin_tcp_client();
         if(result == 0) {
             uart_send_string((u08 *)"connected");
             const char *hw = "hello world!\r\n";
             wiz_write_tcp((u08 *)hw, 14);
             uart_send_string((u08 *)"disconnect");
             wiz_end_tcp_client();
         }

         set_result(result);

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

// diagnose commands
static void cmd_diagnose(void)
{
  u08 cmd, res;
  u08 exit = 0;
  while((cmd = get_char()) != 0) {
     switch(cmd) {
     case 'b': // buttons -> returns 0=no button  1=button2  2=button2  3=both buttons
       res = button1_pressed() | (button2_pressed() << 1);
       set_result(res);
       break;
     case 'l': // led: param: 1=green  2=yellow  3=both
       {
         cmd = parse_hex_byte(3);
         led_green(cmd  & 1);
         led_yellow(cmd & 2);
         set_result(cmd);
         delay_ms(1000);
       }
       break;
     case 'i': // io sd diagnose: show sd detect and sd protect
       {
         res = sdpin_no_card() ? 1 :0;
         res |= sdpin_write_protect() ? 2:0;
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

      // d) diagnose commands
      case 'd':
        cmd_diagnose();
        break;

      // c) clock commands
      case 'c':
        cmd_clock();
        break;

      // n) wiznet commands
      case 'n':
        cmd_wiznet();
        break;

      // ----- TASKS -----
      // R) read disk
      case 'R':
        res = disk_read_all(parse_hex_byte(0),parse_hex_byte(160),1);
        set_result(res);
        break;
      // F) fake read disk
      case 'F':
        res = disk_read_all(parse_hex_byte(0),parse_hex_byte(160),0);
        set_result(res);
        break;
      }
  }

  *result_len = out_size;
}
