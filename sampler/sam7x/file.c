#include "file.h"
#include "uartutil.h"
#include "ff.h"
#include "diskio.h"
#include "pit.h"
#include "led.h"
#include "spiram.h"
#include "uartutil.h"

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

void file_save(u32 size)
{
  u32 checksum = 0;

  uart_send_string((u08 *)"file save");
  uart_send_crlf();

  // perform SPI reset to be sage
  spi_low_mst_init();

  // enable tick irq
  pit_irq_start(disk_timerproc, led_proc);

  uart_send_string((u08 *)"pit");
  uart_send_crlf();

  if(disk_initialize(0) & STA_NOINIT)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return ;
    }

  uart_send_string((u08 *)"mount");
  uart_send_crlf();

  if(f_mount(0, &fatfs) != FR_OK)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return;
    }

  uart_send_string((u08 *)"save");
  uart_send_crlf();

  FIL fh;
  FRESULT result = f_open(&fh, "track.dat", FA_WRITE | FA_CREATE_ALWAYS);
  if(result != FR_OK) {
      uart_send_string((u08 *)"error opening file: ");
      uart_send_hex_dword_crlf(result);
  } else {
      // work buffer for data transfer is shared with spiram's buffers
      u08 *data = &spiram_buffer[0][0];

      // setup SPIRAM
      if(spiram_multi_init()) {
          uart_send_string((u08 *)"spiram init failed");
          uart_send_crlf();
      }

      u32 bank = 0;
      u32 addr = 0;
      u32 chip_no = 0;
      spi_low_set_multi(0);
      while(size > 0) {

          // read block from SPIRAM
          spi_low_set_channel(0);
          spiram_read_begin(addr);
          spi_read_dma(data, SPIRAM_BUFFER_SIZE);
          spiram_end();

          u32 blk_size = (size > SPIRAM_BUFFER_SIZE) ? SPIRAM_BUFFER_SIZE : size;
          if((bank == (SPIRAM_NUM_BUFFER - 1)) && (blk_size == SPIRAM_BUFFER_SIZE)) {
              blk_size -= 3;
          }
          for(int i=0;i<blk_size;i++) {
              checksum += data[i];
          }

          // write to SD
          UINT written;
          result = f_write(&fh,data,blk_size,&written);
          if(result != FR_OK) {
              uart_send_string((u08 *)"write error!");
              uart_send_crlf();
          }

          size -= blk_size;
          bank ++;
          addr += SPIRAM_BUFFER_SIZE;
          if(bank == SPIRAM_NUM_BUFFER) {
              bank = 0;
              addr = 0;
              chip_no ++;

              spi_low_set_multi(chip_no);
          }
      }

      f_close(&fh);
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  uart_send_string((u08 *)"done. checksum: ");
  uart_send_hex_dword_crlf(checksum);
}

void file_dir(void)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;

  uart_send_string((u08 *)"file dir");
  uart_send_crlf();

  pit_irq_start(disk_timerproc, led_proc);

  uart_send_string((u08 *)"pit");
  uart_send_crlf();

  if(disk_initialize(0) & STA_NOINIT)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return ;
    }

  uart_send_string((u08 *)"init");
  uart_send_crlf();

  if(f_mount(0, &fatfs) != FR_OK)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return;
    }

  uart_send_string((u08 *)"dir");
  uart_send_crlf();

  // read dir
  res = f_opendir(&dir, "/");
  if(res) {
      uart_send_string((u08 *)"f_opendir failed!");
      uart_send_crlf();
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

          uart_send_hex_dword_crlf(finfo.fsize);
          uart_send_string((u08 *)"entry: ");
          uart_send_string(name);
          uart_send_crlf();
      }
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  uart_send_string((u08 *)"done");
  uart_send_crlf();
}

