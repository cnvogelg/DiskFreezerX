#include "file.h"
#include "uartutil.h"
#include "ff.h"
#include "diskio.h"
#include "pit.h"
#include "led.h"
#include "spiram.h"
#include "uartutil.h"
#include "track.h"
#include "util.h"

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

u08 file_save(u08 track, u32 size, u32 check, int verbose)
{
  u32 checksum = 0;

  // compose name: dir_name/trk_name
  u08 *name = full_name;
  u08 *trk_ptr = track_name(track);
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

  // perform SPI reset to be sage
  spi_low_mst_init();

  // enable tick irq
  pit_irq_start(disk_timerproc, led_proc);

  if(disk_initialize(0) & STA_NOINIT)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return 0x10;
    }

  if(f_mount(0, &fatfs) != FR_OK)
    {
      uart_send_string((u08 *)"mound failed!");
      uart_send_crlf();
      return 0x11;
    }

  FIL fh;
  FRESULT result = f_open(&fh, (char *)full_name, FA_WRITE | FA_CREATE_ALWAYS);
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
      spi_low_set_ram_addr(0);
      u32 total = 0;
      while(size > 0) {

          // read block from SPIRAM
          spi_low_set_channel(SPI_RAM_CHANNEL);
          if(spiram_set_mode(SPIRAM_MODE_SEQ)!=SPIRAM_MODE_SEQ) {
              uart_send_string((u08 *)"set mode error!");
              uart_send_crlf();
          }
          spiram_read_begin(addr);
          spi_read_dma(data, SPIRAM_BUFFER_SIZE);
          spiram_end();

          u32 blk_check = 0;
          for(int i=0;i<SPIRAM_BUFFER_SIZE;i++) {
              blk_check += data[i];
          }

          u32 blk_size = (size > SPIRAM_BUFFER_SIZE) ? SPIRAM_BUFFER_SIZE : size;
          if((bank == (SPIRAM_NUM_BANKS - 1)) && (blk_size > (SPIRAM_BUFFER_SIZE-3))) {
              blk_size = SPIRAM_BUFFER_SIZE - 3;
          }
          for(int i=0;i<blk_size;i++) {
              checksum += data[i];
          }
          total += blk_size;

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
          if(bank == SPIRAM_NUM_BANKS) {
              bank = 0;
              addr = 0;
              chip_no ++;

              spi_low_set_ram_addr(chip_no);

#if 0
              uart_send_hex_dword_crlf(blk_check);
#endif
          }
      }

      if(verbose) {
        uart_send_string((u08 *)"read checksum:  ");
        uart_send_hex_dword_crlf(check);
        uart_send_string((u08 *)"save checksum:  ");
        uart_send_hex_dword_crlf(checksum);
        uart_send_string((u08 *)"total size:     ");
        uart_send_hex_dword_crlf(total);
      }

      f_close(&fh);
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();
  return (checksum != check);
}

void file_dir(void)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;

  pit_irq_start(disk_timerproc, led_proc);

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

u32 file_find_disk_dir(void)
{
  DIR dir;
  FRESULT res;
  FILINFO finfo;
  u32 disk_num = 0;

  pit_irq_start(disk_timerproc, led_proc);

  if(disk_initialize(0) & STA_NOINIT)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return 0;
    }

  if(f_mount(0, &fatfs) != FR_OK)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return 0;
    }

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

  return disk_num;
}

u08 file_make_disk_dir(u32 num)
{
  FRESULT res;

  pit_irq_start(disk_timerproc, led_proc);

  if(disk_initialize(0) & STA_NOINIT)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return 0;
    }

  if(f_mount(0, &fatfs) != FR_OK)
    {
      uart_send_string((u08 *)"disk_initialize failed!");
      uart_send_crlf();
      return 0;
    }

  // generate disk name
  word_to_hex((u16)(num & 0xffff), dir_name + 4);

  uart_send_string((u08 *)"mkdir: ");
  uart_send_string(dir_name);
  uart_send_crlf();

  // create dir
  res = f_mkdir((const char *)dir_name);
  if(res) {
      uart_send_string((u08 *)"f_mkdir failed!");
      uart_send_crlf();
  }

  // unmount
  f_mount(0, 0);
  disk_ioctl(0, CTRL_POWER, 0); //power off

  pit_irq_stop();

  return res & 0xff;
}

