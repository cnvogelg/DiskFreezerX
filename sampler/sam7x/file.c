#include "file.h"
#include "uartutil.h"
#include "ff.h"
#include "diskio.h"
#include "pit.h"
#include "led.h"
#include "uartutil.h"
#include "track.h"
#include "util.h"
#include "delay.h"
#include "buffer.h"

static FATFS fatfs;

#if _USE_LFN
static char lfname[512];
#endif

static void led_proc(void)
{
  static u32 on = 0;
  led_yellow(on);
  on = 1-on;
}

static u08 *dir_name  = (u08 *)"disk0000.dfx";
static u08 full_name[32];

// fake a time for FAT
DWORD get_fattime(void)
{
        return  (((DWORD)(2009-1980)) << 25) | // Year
                        (((DWORD)          6) << 21) | // Month
                        (((DWORD)         27) << 16) | // Day
                        (((DWORD)         12) << 11) | // Hour
                        (((DWORD)          0) <<  5) | // Min
                        (((DWORD)          0) >>  1);  // Sec
}

static FIL fh;

static error_t file_writer(const u08 *buffer, u32 size)
{
  UINT written;
  FRESULT result = f_write(&fh, buffer, size, &written);
  if(result != FR_OK) {
      return ERROR_FILE_WRITE;
  } else {
      return STATUS_OK;
  }
}

error_t file_save_buffer(int verbose)
{
  // compose name: dir_name/trk_name
  u08 *name = full_name;
  u08 *trk_ptr = track_name(buffer_get_track());
  u08 *dir_ptr = dir_name;
  while((*(name++) = *(dir_ptr++)));
  name--;
  *(name++) = '/';
  while((*(name++) = *(trk_ptr++)));

  if(verbose) {
      uart_send_string((u08 *)"file save: ");
      uart_send_string(full_name);
      uart_send_crlf();
  }

  // enable tick irq
  pit_irq_start(disk_timerproc, led_proc);

  // init disk
  if(disk_initialize(0) & STA_NOINIT) {
      return ERROR_FILE_INIT;
  }

  // mount disk
  if(f_mount(0, &fatfs) != FR_OK) {
      return ERROR_FILE_MOUNT;
  }

  // open track file
  error_t status = STATUS_OK;
  FRESULT result = f_open(&fh, (char *)full_name, FA_WRITE | FA_CREATE_ALWAYS);
  if(result != FR_OK) {
      status = ERROR_FILE_OPEN;
  } else {
      // write buffer to file
      status = buffer_write(file_writer);

      // close track file
      f_close(&fh);
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  if(verbose) {
      uart_send_string((u08 *)"buffer write: ");
      uart_send_hex_dword_crlf(status);
  }

  return status;
}

error_t file_dir(void)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;

  pit_irq_start(disk_timerproc, led_proc);

  // init
  if(disk_initialize(0) & STA_NOINIT) {
      return ERROR_FILE_INIT;
  }
  // mount
  if(f_mount(0, &fatfs) != FR_OK) {
      return ERROR_FILE_MOUNT;
  }

  // read dir
  error_t status = STATUS_OK;
  res = f_opendir(&dir, "/");
  if(res) {
      status = ERROR_FILE_OPEN;
  } else {

#if _USE_LFN
      finfo.lfname = lfname;
      finfo.lfsize = sizeof(lfname);
#endif

      while (((res = f_readdir(&dir, &finfo)) == FR_OK) && finfo.fname[0]) {
          u08 *name = (u08 *)finfo.fname;
          if(finfo.lfname[0]) {
              name = (u08 *)finfo.lfname;
          }

          uart_send_string((u08 *)"DE: ");
          uart_send_hex_dword_space(finfo.fsize);
          uart_send_string(name);
          uart_send_crlf();
      }
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  return status;
}

error_t file_find_disk_dir(u32 *result_num)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;
  u32 disk_num = 0;

  pit_irq_start(disk_timerproc, led_proc);

  // init
  if(disk_initialize(0) & STA_NOINIT) {
      return ERROR_FILE_INIT;
  }
  // mount
  if(f_mount(0, &fatfs) != FR_OK) {
      return ERROR_FILE_MOUNT;
  }

  // read dir
  error_t status = STATUS_OK;
  res = f_opendir(&dir, "/");
  if(res) {
      status = ERROR_FILE_OPEN;
  } else {

#if _USE_LFN
      finfo.lfname = lfname;
      finfo.lfsize = sizeof(lfname);
#endif

      while (((res = f_readdir(&dir, &finfo)) == FR_OK) && finfo.fname[0]) {
          u08 *name = (u08 *)finfo.fname;
          if(finfo.lfname[0]) {
              name = (u08 *)finfo.lfname;
          }

          //            012345678901
          // check for "diskXXXX.dfx" name (len==12)
          int len = 0;
          u08 *ptr = name;
          while(*(ptr++)!=0) { len++; }
          if(len == 12) {
              if( (name[0]=='d') && (name[1]=='i') &&
                  (name[2]=='s') && (name[3]=='k') &&
                  (name[8]=='.') && (name[9]=='d') &&
                  (name[10]=='f') && (name[11]=='x') ) {
                  u16 num;
                  parse_word(&name[4],&num);
                  num++;
                  if(num > disk_num) {
                      disk_num = num;
                  }
              }
          }
      }
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  *result_num = disk_num;
  return status;
}

error_t file_make_disk_dir(u32 num)
{
  FRESULT res;

  pit_irq_start(disk_timerproc, led_proc);
  // init
  if(disk_initialize(0) & STA_NOINIT) {
      return ERROR_FILE_INIT;
  }
  // mount
  if(f_mount(0, &fatfs) != FR_OK) {
      return ERROR_FILE_MOUNT;
  }

  // generate disk name
  word_to_hex((u16)(num & 0xffff), dir_name + 4);

  // create dir
  error_t status = STATUS_OK;
  res = f_mkdir((const char *)dir_name);
  if(res) {
      status = ERROR_FILE_MKDIR;
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  return status;
}

// ----- Test Func -----

static volatile u32 count = 0;

static void test_func(void)
{
  count ++;
}

void file_test(void)
{
  count = 0;
  pit_irq_start(test_func, led_proc);

  for(int i=0;i<10;i++) {
      uart_send_hex_byte_crlf(i);
      delay_ms(500);
  }

  pit_irq_stop();

  uart_send_string((u08 *)"ticks: ");
  uart_send_hex_dword_crlf(count);
}


