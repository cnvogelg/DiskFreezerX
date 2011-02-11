#include "file.h"
#include "uartutil.h"
#include "ff.h"
#include "diskio.h"

static FATFS fatfs;

void file_test(void)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;

  uart_send_string((u08 *)"file test");
  uart_send_crlf();

  if(disk_initialize(0) & STA_NOINIT)
  {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return ;
  }

  if(f_mount(0, &fatfs) != FR_OK)
  {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return;
  }

  // read dir
  res = f_opendir(&dir, "/");
  if(res) {
      uart_send_string((u08 *)"f_opendir failed!");
      uart_send_crlf();
  } else {
      while (((res = f_readdir(&dir, &finfo)) == FR_OK) && finfo.fname[0]) {
          uart_send_string((u08 *)finfo.fname);
          uart_send_crlf();
      }
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off


}

