#include "memory.h"

#include "spiram.h"
#include "uartutil.h"
#include "util.h"

static u08 memory_test_no_dma(u08 chip_no)
{
  spiram_init();
  spi_low_set_ram_addr(chip_no);

  /* set sequential mode */
  u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(mode != SPIRAM_MODE_SEQ) {
      return 0xff;
  }

  u08 d;

  /* write bytes */
  u32 sum = 0;
  spiram_write_begin(0);
  d = 0;
  for(u16 i=0;i<SPIRAM_CHIP_SIZE;i++) {
      spiram_write_byte(d);
      sum += d;
      d++;
  }
  spiram_end();

  /* read bytes */
  u32 sum2 = 0;
  spiram_read_begin(0);
  for(u16 i=0;i<SPIRAM_CHIP_SIZE;i++) {
      d = spiram_read_byte();
      sum2 += d;
  }
  spiram_end();

  spiram_close();
  return (sum != sum2);
}

static u08 memory_test_dma(u08 chip_no)
{
  spiram_init();
  spi_low_set_ram_addr(chip_no);

  // set sequential mode
  u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(mode != SPIRAM_MODE_SEQ) {
      return 0xff;
  }

  u16 page_size = SPIRAM_BUFFER_SIZE;
  u16 banks = SPIRAM_NUM_BANKS;
  u08 *data = &spiram_buffer[0][0];

  // fill page
  u08 d = 0;
  for(int i=0;i<page_size;i++) {
      data[i] = d++;
  }

  // write
  spiram_write_begin(0);
  for(int b=0;b<banks;b++) {
      spi_write_dma(data,page_size);
  }
  spiram_end();

  // read
  u32 read_errors = 0;
  spiram_read_begin(0);
  for(int b=0;b<banks;b++) {
      spi_read_dma(data,page_size);

      // check
      d = 0;
      for(int i=0;i<page_size;i++) {
          if(data[i] != d) {
              read_errors ++;
          }
          d++;
      }
  }
  spiram_end();
  spiram_close();
  return (read_errors!=0);
}

u08 memory_init()
{
  // init chip
  int errors = spiram_multi_init();
  uart_send_string((u08 *)"init: ");
  uart_send_hex_byte_crlf(errors);
  if(errors > 0) {
      return errors;
  }

  // clear chip
  errors = spiram_multi_clear(0);
  uart_send_string((u08 *)"clr:  ");
  uart_send_hex_byte_crlf(errors);
  if(errors > 0) {
      return errors;
  }
  return 0;
}

u08 memory_check(u08 mode)
{
  u08 *result = (u08 *)"#xx: xx";

  u08 total = 0;
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      u08 errors = 0;
      if(mode == 0) {
          errors = memory_test_dma(i);
      } else {
          errors = memory_test_no_dma(i);
      }
      total |= errors;

      byte_to_hex(i,result+1);
      byte_to_hex(errors,result+5);
      uart_send_string(result);
      uart_send_crlf();
  }
  return total;
}

void memory_dump(u08 chip_no,u08 bank)
{
  u32 buf_no = 0;
  if(chip_no < SPIRAM_NUM_CHIPS) {
      spiram_init();
      spi_low_set_ram_addr(chip_no);
  } else {
      buf_no = ( chip_no - SPIRAM_NUM_CHIPS) % SPIRAM_NUM_BUFFER;
  }

  u08 *buf = spiram_buffer[buf_no];
  u32 addr;

  if(chip_no < SPIRAM_NUM_CHIPS) {
      // read bytes from ram
      addr = bank * SPIRAM_BUFFER_SIZE;
      spiram_read_begin(addr);
      for(int i=0;i<SPIRAM_BUFFER_SIZE;i++) {
          buf[i] = spiram_read_byte();
      }
      spiram_end();
      addr += chip_no * SPIRAM_CHIP_SIZE;
  } else {
      // read from internal buffer
      addr = 0xf0000000 | (buf_no << 24);
  }

  // dump to serial
  const u32 line_size = 16;
  u32 lines = SPIRAM_BUFFER_SIZE / line_size;
  for(u32 i=0;i<lines;i++) {
      uart_send_hex_line_crlf(addr,buf,line_size);
      addr += line_size;
      buf += line_size;
  }
}

