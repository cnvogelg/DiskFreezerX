/* spiram.c
 *
 * control a MCP23A256 32k SPI SRAM chip
 */

#include "spiram.h"
#include "uartutil.h"

/* SPI commands of SRAM */
#define SPIRAM_CMD_READ         0x03
#define SPIRAM_CMD_WRITE        0x02
#define SPIRAM_CMD_READ_STATUS  0x05
#define SPIRAM_CMD_WRITE_STATUS 0x01

void spiram_init(void)
{
  spi_mst_init(48);
}

void spiram_close(void)
{
  spi_close();
}

u08 spiram_set_mode(u08 mode)
{
  u08 result;

  // set mode in status register
  spi_io(SPIRAM_CMD_WRITE_STATUS);
  spi_io_last(mode);

  // read mode again
  spi_io(SPIRAM_CMD_READ_STATUS);
  result = spi_io_last(0xff);

  return (result != mode);
}

void spiram_write_begin(u16 address)
{
  spi_io(SPIRAM_CMD_WRITE);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_read_begin(u16 address)
{
  spi_io(SPIRAM_CMD_READ);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

u32 spiram_test(u08 begin,u16 size)
{
  spiram_init();

  // set sequential mode
  u08 errors = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(errors) {
      return 0xffff;
  }

  uart_send_string((u08 *)"set mode ok");
  uart_send_crlf();

  u08 d;

  /* write bytes */
  u32 sum = 0;
  spiram_write_begin(0);
  d = begin;
  for(u16 i=0;i<(size-1);i++) {
      spiram_write_byte(d);
      sum += d;
      d++;
  }
  spiram_write_byte_last(d);
  sum += d;

  uart_send_string((u08 *)"write ok");
  uart_send_crlf();
  uart_send_hex_dword_crlf(sum);

  /* read bytes */
  u32 sum2 = 0;
  spiram_read_begin(0);
  for(u16 i=0;i<(size-1);i++) {
      d = spiram_read_byte();
      sum2 += d;
  }
  d = spiram_read_byte_last();
  sum2 += d;

  uart_send_string((u08 *)"read done");
  uart_send_crlf();
  uart_send_hex_dword_crlf(sum2);

  spiram_close();
  return (sum == sum2);
}
