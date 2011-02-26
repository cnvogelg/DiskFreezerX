#include "disk.h"
#include "floppy.h"
#include "file.h"
#include "trk_read.h"
#include "uartutil.h"
#include "track.h"

u08 disk_read_all(u08 begin, u08 end)
{
  u08 status = 0;

  uart_send_string((u08 *)"disk read");
  uart_send_crlf();

  u08 en = floppy_select_on();
  u08 mot = floppy_motor_on();

  track_zero();
  track_next(begin);
  for(u08 t=begin;t<end;t++) {
      uart_send_string(track_name(t));
      uart_send_string((u08 *)": ");

      u08 st = trk_read_to_spiram(0);

      if(st != 0) {
          uart_send_hex_byte_crlf(st);
          break;
      }

      read_status_t *rs = trk_read_get_status();
      u32 size = rs->data_size;
      u32 check = rs->data_checksum;
      st = file_save(t,size,check,0);
      uart_send_hex_byte_crlf(st);
      if(st != 0) {
          break;
      }

      track_next(1);
  }

  if(mot) floppy_motor_off();
  if(en) floppy_select_off();
  return status;
}
