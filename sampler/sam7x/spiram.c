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
  // sbcr = MCLK / spiram_rate
  // MCLK=48 MHz -> spiram_rate = 6 Mbit/s
  spi_mst_init(8);
  spi_low_dma_init();
}

void spiram_close(void)
{
  spi_close();
}

u08 spiram_set_mode(u08 mode)
{
  u08 result;

  // set mode in status register
  spi_low_enable_cs0();
  spi_io(SPIRAM_CMD_WRITE_STATUS);
  spi_io(mode);
  spi_low_disable_cs0();

  // read mode again
  spi_low_enable_cs0();
  spi_io(SPIRAM_CMD_READ_STATUS);
  result = spi_io(0xff);
  spi_low_disable_cs0();

  return (result != mode);
}

void spiram_write_begin(u16 address)
{
  spi_low_enable_cs0();
  spi_io(SPIRAM_CMD_WRITE);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_read_begin(u16 address)
{
  spi_low_enable_cs0();
  spi_io(SPIRAM_CMD_READ);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_end(void)
{
  spi_low_disable_cs0();
}

u08 dma_dummy[SPIRAM_MAX_DMA_SIZE];
//volatile u32 dummy;

void spiram_write_dma(const u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(data,size);
  spi_low_tx_dma_enable();
  while(!spi_low_tx_dma_empty());
  spi_low_tx_dma_disable();
}

void spiram_read_dma(u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(dma_dummy,size);
  spi_low_rx_dma_set_first(data,size); // addr must be != 0 otherwise breaks
  spi_low_dma_enable();
  while(!spi_low_rx_dma_empty());
  spi_low_dma_disable();
}

void spiram_end_dma(void)
{
  spi_low_disable_cs0();
}

// ----- TESTS ----------------------------------------------------------------

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
  for(u16 i=0;i<size;i++) {
      spiram_write_byte(d);
      sum += d;
      d++;
  }
  spiram_end();

  uart_send_string((u08 *)"write ok");
  uart_send_crlf();
  uart_send_hex_dword_crlf(sum);

  /* read bytes */
  u32 sum2 = 0;
  spiram_read_begin(0);
  for(u16 i=0;i<size;i++) {
      d = spiram_read_byte();
      sum2 += d;
  }
  spiram_end();

  uart_send_string((u08 *)"read done");
  uart_send_crlf();
  uart_send_hex_dword_crlf(sum2);

  spiram_close();
  return (sum == sum2);
}

// ----- DMA TEST -----

/* write bytes */
u08 data[SPIRAM_MAX_DMA_SIZE];

u32 spiram_dma_test(u08 begin,u16 size)
{
  spiram_init();

  // set sequential mode
  u08 errors = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(errors) {
      return 0xffff;
  }

  uart_send_string((u08 *)"set mode ok");
  uart_send_crlf();

  u16 page_size = SPIRAM_MAX_DMA_SIZE;
  u16 pages  = size / page_size;
  u16 remain = size % page_size;
  uart_send_hex_word_crlf(pages);
  uart_send_hex_word_crlf(remain);

  pages = 1;
  remain = 0;

  // fill page
  u08 d = begin;
  for(int i=0;i<page_size;i++) {
      data[i] = d++;
  }

  // write
  spiram_write_begin(0);
  for(int p=0;p<pages;p++) {
      spiram_write_dma(data,page_size);
  }
  if(remain > 0) {
      spiram_write_dma(data,remain);
  }
  spiram_end_dma();

  uart_send_string((u08 *)"write ok");
  uart_send_crlf();

  // read
  u32 read_errors = 0;
  spiram_read_begin(0);
  for(int p=0;p<pages;p++) {
      spiram_read_dma(data,page_size);

      // check
      d = begin;
      for(int i=0;i<page_size;i++) {
          if(data[i] != d) {
              read_errors ++;
          }
          d++;
      }
  }
  if(remain > 0) {
      spiram_read_dma(data,remain);

      // check
      d = begin;
      for(int i=0;i<remain;i++) {
          if(data[i] != d) {
              read_errors ++;
          }
          d++;
      }
  }
  spiram_end_dma();

  uart_send_string((u08 *)"read done");
  uart_send_crlf();

  spiram_close();
  return read_errors;
}
