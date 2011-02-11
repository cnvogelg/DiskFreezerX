/* spiram.c
 *
 * control a MCP23A256 32k SPI SRAM chip
 */

#include "spiram.h"
#include "uartutil.h"

u08 spiram_dummy_buffer[SPIRAM_BUFFER_SIZE];

void spiram_init(void)
{
  // sbcr = MCLK / spiram_rate
  // MCLK=48 MHz -> spiram_rate = 6 Mbit/s
  spi_low_mst_init(8);
  spi_low_dma_init();
}

void spiram_close(void)
{
  spi_low_close();
}

u08 spiram_set_mode(u08 mode)
{
  u08 result;

  // set mode in status register
  spi_low_enable_multi();
  spi_low_io(SPIRAM_CMD_WRITE_STATUS);
  spi_low_io(mode);
  spi_low_disable_multi();

  // read mode again
  spi_low_enable_multi();
  spi_low_io(SPIRAM_CMD_READ_STATUS);
  result = spi_low_io(0xff);
  spi_low_disable_multi();

  return result;
}

void spiram_write_begin(u16 address)
{
  spi_low_enable_multi();
  spi_low_io(SPIRAM_CMD_WRITE);
  spi_low_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_low_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_read_begin(u16 address)
{
  spi_low_enable_multi();
  spi_low_io(SPIRAM_CMD_READ);
  spi_low_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_low_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_end(void)
{
  spi_low_disable_multi();
}

void spiram_write_dma(const u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(data,size);
  spi_low_rx_dma_set_first(spiram_dummy_buffer,size);
  spi_low_dma_enable();
  while(!spi_low_rx_dma_empty());
  spi_low_dma_disable();
}

void spiram_read_dma(u08 *data, u16 size)
{
  spi_low_tx_dma_set_first(spiram_dummy_buffer,size);
  spi_low_rx_dma_set_first(data,size); // addr must be != 0 otherwise breaks
  spi_low_dma_enable();
  while(!spi_low_rx_dma_empty());
  spi_low_dma_disable();
}

// ----- MULTI RAM ------------------------------------------------------------

u08  spiram_buffer[SPIRAM_NUM_BUFFER][SPIRAM_BUFFER_SIZE];

u32  spiram_buffer_index;
u32  spiram_buffer_usage;
u08 *spiram_buffer_ptr;

u32  spiram_buffer_overflows;

u32  spiram_chip_no;
u32  spiram_bank_no;
u32  spiram_buffer_on_chip[SPIRAM_NUM_BUFFER];

u32  spiram_num_ready;
u32  spiram_dma_index;
u32  spiram_dma_chip_no;

u32  spiram_total;

int spiram_multi_init(void)
{
  spiram_init();

  // set SEQ mode for all chips
  int error_flag = 0;
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
      if(mode != SPIRAM_MODE_SEQ) {
          error_flag |= (1<<i);
      }
  }

  return error_flag;
}

// clear all chip rams
int spiram_multi_clear(u08 value)
{
  // clear buffer
  u08 *buf = spiram_buffer[0];
  for(int i=0;i<SPIRAM_BUFFER_SIZE;i++) {
      buf[i] = value;
  }

  // write buffer to all banks/chips
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      spi_low_set_multi(i);
      spiram_write_begin(0);
      for(int j=0;j<SPIRAM_NUM_BANKS;j++) {
          spiram_write_dma(buf, SPIRAM_BUFFER_SIZE);
      }
      spiram_end();
  }

  // verify loop
  int error_flag = 0;
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      spi_low_set_multi(i);
      spiram_read_begin(0);
      for(int j=0;j<SPIRAM_NUM_BANKS;j++) {
          spiram_read_dma(buf, SPIRAM_BUFFER_SIZE);
          for(int k=0;k<SPIRAM_BUFFER_SIZE;k++) {
              if(buf[k]!=value) {
                  error_flag ++;
              }
          }
      }
      spiram_end();
  }
  return error_flag;
}

void spiram_multi_write_begin(void)
{
  // fill buffer with initial WRITE command
  spiram_buffer[0][0] = SPIRAM_CMD_WRITE;
  spiram_buffer[0][1] = 0;
  spiram_buffer[0][2] = 0;

  // setup write buffer state
  spiram_buffer_index = 0;
  spiram_buffer_usage = 3;
  spiram_buffer_ptr   = &spiram_buffer[0][0] + 3;

  // reset error state
  spiram_buffer_overflows = 0;

  // init bank/chip checking
  spiram_chip_no = 0;
  spiram_bank_no = 0;
  spiram_buffer_on_chip[0] = 0;

  // setup DMA buffer index
  spiram_num_ready = 0;
  spiram_dma_index = 0;
  spiram_dma_chip_no = 0;

  spiram_total = 0;

  // pre-select chip 0
  spi_low_set_multi(0);
  spi_low_dma_enable();
  spi_low_enable_multi();
}

int spiram_multi_write_next_buffer(void)
{
  // buffer is full -> get next
  u32 next_index = (spiram_buffer_index + 1) & (SPIRAM_NUM_BUFFER-1);

  // oops! -> DMA is using this buffer :( overflow!
  if(next_index == spiram_dma_index) {
      spiram_buffer_overflows ++;
      return 0;
  } else {
      spiram_buffer_usage = 0;
      spiram_buffer_index = next_index;
      spiram_buffer_ptr   = &spiram_buffer[spiram_buffer_index][0];

      // determine chip
      spiram_bank_no ++;
      if(spiram_bank_no == SPIRAM_NUM_BANKS) {
          spiram_chip_no++;
          spiram_bank_no = 0;

          // write initial WRITE command for new chip
          *(spiram_buffer_ptr++) = SPIRAM_CMD_WRITE;
          *(spiram_buffer_ptr++) = 0;
          *(spiram_buffer_ptr++) = 0;
          spiram_buffer_usage    = 3;
      }

      // store for each buffer the chip
      spiram_buffer_on_chip[next_index] = spiram_chip_no;

      // another buffer is reay
      spiram_num_ready ++;
      return 1;
  }
}

void spiram_multi_write_end(void)
{
  // make sure all buffers are transmitted
  while(spiram_num_ready > 0) {
      spiram_multi_write_handle();
  }

  // make sure last DMA finished
  while(!spi_low_rx_dma_empty());

  spi_low_dma_disable();
  spi_low_disable_multi();
}

// ----- TESTS ----------------------------------------------------------------

u32 spiram_test(u08 begin,u16 size)
{
  spiram_init();
  spi_low_set_multi(begin);

  // set sequential mode
  u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(mode != SPIRAM_MODE_SEQ) {
      return mode;
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
  return (sum != sum2);
}

// ----- DMA TEST -----

u32 spiram_dma_test(u08 begin,u16 size)
{
  spiram_init();
  spi_low_set_multi(begin);

  // set sequential mode
  u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
  if(mode != SPIRAM_MODE_SEQ) {
      return mode;
  }

  uart_send_string((u08 *)"set mode ok");
  uart_send_crlf();

  u16 page_size = SPIRAM_BUFFER_SIZE;
  u16 pages  = size / page_size;
  u16 remain = size % page_size;

  pages = 1;
  remain = 0;

  u08 *data = &spiram_buffer[0][0];

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
  spiram_end();

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
  spiram_end();

  uart_send_string((u08 *)"read done");
  uart_send_crlf();

  spiram_close();
  return read_errors;
}

// ----- Read & Write Test -----

u32 spiram_dump(u08 chip_no,u08 bank)
{
  if(chip_no >= SPIRAM_NUM_CHIPS) {
      return 1;
  }

  spiram_init();
  spi_low_set_multi(chip_no);

  u32 addr = bank * SPIRAM_BUFFER_SIZE;

  // read bytes from ram
  u08 *buf = spiram_buffer[0];
  spiram_read_begin(addr);
  for(int i=0;i<SPIRAM_BUFFER_SIZE;i++) {
     buf[i] = spiram_read_byte();
  }
  spiram_end();

  // dump to serial
  addr += chip_no * SPIRAM_SIZE;
  const u32 line_size = 16;
  u32 lines = SPIRAM_BUFFER_SIZE / line_size;
  for(u32 i=0;i<lines;i++) {
      uart_send_hex_line_crlf(addr,buf,line_size);
      addr += line_size;
      buf += line_size;
  }

  return 0;
}
