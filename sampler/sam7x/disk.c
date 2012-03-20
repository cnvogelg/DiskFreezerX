#include "disk.h"
#include "floppy.h"
#include "file.h"
#include "trk_read.h"
#include "uartutil.h"
#include "track.h"
#include "button.h"
#include "buffer.h"
#include "status.h"
#include "index.h"

u08 disk_read_all(u08 begin, u08 end, int do_save, int verbose)
{
  u08 error = 0;
  u32 disk_no = 0;

  if(do_save) {
    // make directory for track data
    disk_no = file_find_disk_dir();
    error = file_make_disk_dir(disk_no);
    if(error) {
        return error;
    }

    uart_send_string((u08 *)"ID: ");
    uart_send_hex_dword_crlf(disk_no);
  }

  u08 en = floppy_select_on();
  u08 mot = floppy_motor_on();

  track_zero();
  track_next(begin);
  for(u08 t=begin;t<end;t++) {

      uart_send_string((u08 *)"TR: ");
      uart_send_hex_byte_space(t);

      // read track to SPI memory
      error = trk_read_to_spiram(0);
      uart_send_hex_byte_crlf(error);
      if(error) {
          break;
      }

      // save track to file
      if(do_save) {
        uart_send_string((u08 *)"FS: ");
        uart_send_hex_byte_space(t);

        error = file_save_buffer(0);
        uart_send_hex_byte_crlf(error);
        if(error) {
            break;
        }
      }

      // give some infos
      if(verbose) {
          buffer_info();
          index_info();
          status_info();
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
  return error;
}
