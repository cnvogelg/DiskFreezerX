#include "disk.h"
#include "floppy.h"
#include "file.h"
#include "trk_read.h"
#include "uartutil.h"
#include "track.h"
#include "button.h"
#include "buffer.h"

u08 disk_read_all(u08 begin, u08 end, int do_save)
{
  u08 status = 0;
  u32 disk_no = 0;

  if(do_save) {
    // make directory for track data
    disk_no = file_find_disk_dir();
    status = file_make_disk_dir(disk_no);
    if(status) {
        return status;
    }
  }

  uart_send_string((u08 *)"disk read: ");
  uart_send_hex_dword_crlf(disk_no);

  u08 en = floppy_select_on();
  u08 mot = floppy_motor_on();

  track_zero();
  track_next(begin);
  for(u08 t=begin;t<end;t++) {
      uart_send_string(track_name(t));
      uart_send_string((u08 *)": ");

      u08 st = trk_read_to_spiram(0);
      uart_send_hex_byte_space(st);
      if(st != 0) {
          uart_send_crlf();
          break;
      }

      // give some sampling info
      read_status_t *rs = trk_read_get_status();
      uart_send_hex_dword_space(rs->cell_overruns);
      uart_send_hex_dword_space(rs->cell_overflows);
      uart_send_hex_dword_space(rs->timer_overflows);
      uart_send_hex_dword_space(rs->data_size);

      u32 size = rs->data_size;
      u32 check = rs->data_checksum;

      if(do_save) {
        buffer_set(t,size,check);
        st = file_save(t,0);
        uart_send_hex_byte_crlf(st);
        if(st != 0) {
            break;
        }
      } else {
        uart_send_crlf();
      }

      // abort if a button was pressed
      if(button1_pressed()) {
          while(button1_pressed()) {}
          break;
      }

      track_next(1);
  }

  if(mot) floppy_motor_off();
  if(en) floppy_select_off();
  return status;
}
