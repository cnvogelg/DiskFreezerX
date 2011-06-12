/* spiram.c
 *
 * control a MCP23A256 32k SPI SRAM chip
 */

#include "spiram.h"

void spiram_init(void)
{
  spi_low_init_channel(SPI_RAM_CHANNEL,8,1,0); // NCPHA, 48/8=6 MHz -> Clock for SPI RAM

  spi_low_dma_init();
  spi_low_set_channel(SPI_RAM_CHANNEL);
  spi_low_enable();
}

void spiram_close(void)
{
  spi_low_disable();
}

u08 spiram_set_mode(u08 mode)
{
  u08 result;

  // set mode in status register
  spi_low_enable_cs(SPI_RAM_CS_MASK);
  spi_io(SPIRAM_CMD_WRITE_STATUS);
  spi_io(mode);
  spi_low_disable_cs(SPI_RAM_CS_MASK);

  // read mode again
  spi_low_enable_cs(SPI_RAM_CS_MASK);
  spi_io(SPIRAM_CMD_READ_STATUS);
  result = spi_io(0xff);
  spi_low_disable_cs(SPI_RAM_CS_MASK);

  return result;
}

void spiram_write_begin(u16 address)
{
  spi_low_enable_cs(SPI_RAM_CS_MASK);
  spi_io(SPIRAM_CMD_WRITE);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_read_begin(u16 address)
{
  spi_low_enable_cs(SPI_RAM_CS_MASK);
  spi_io(SPIRAM_CMD_READ);
  spi_io((u08)((address >> 8)& 0xff)); // hi byte of addr
  spi_io((u08)(address & 0xff));       // lo byte of addr
  // now bytes will follow until CS is raised again...
}

void spiram_end(void)
{
  spi_low_disable_cs(SPI_RAM_CS_MASK);
}

// ----- MULTI RAM ------------------------------------------------------------

u08  spiram_buffer[SPIRAM_NUM_BUFFER][SPIRAM_BUFFER_SIZE];

u32  spiram_buffer_index;
u32  spiram_buffer_usage;
u08 *spiram_buffer_ptr;

u32  spiram_buffer_overruns;

u32  spiram_chip_no;
u32  spiram_bank_no;
u32  spiram_buffer_on_chip[SPIRAM_NUM_BUFFER];

u32  spiram_num_ready;
u32  spiram_dma_index;
u32  spiram_dma_chip_no;
u32  spiram_dma_busy;

u32  spiram_total;
u32  spiram_checksum;

u08 spiram_multi_init(void)
{
  spiram_init();

  // set SEQ mode for all chips
  u08 error_flag = 0;
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      u08 mode = spiram_set_mode(SPIRAM_MODE_SEQ);
      if(mode != SPIRAM_MODE_SEQ) {
          error_flag |= (1<<i);
      }
  }

  return error_flag;
}

// clear all chip rams
u08 spiram_multi_clear(u08 value)
{
  // clear buffer
  u08 *buf = spiram_buffer[0];
  for(int i=0;i<SPIRAM_BUFFER_SIZE;i++) {
      buf[i] = value;
  }

  // write buffer to all banks/chips
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      spi_low_set_ram_addr(i);
      spiram_write_begin(0);
      for(int j=0;j<SPIRAM_NUM_BANKS;j++) {
          spi_write_dma(buf, SPIRAM_BUFFER_SIZE);
      }
      spiram_end();
  }

  // verify loop
  u08 error_flag = 0;
  for(int i=0;i<SPIRAM_NUM_CHIPS;i++) {
      spi_low_set_ram_addr(i);
      spiram_read_begin(0);
      for(int j=0;j<SPIRAM_NUM_BANKS;j++) {
          spi_read_dma(buf, SPIRAM_BUFFER_SIZE);
          for(int k=0;k<SPIRAM_BUFFER_SIZE;k++) {
              if(buf[k]!=value) {
                  error_flag |= (1<<i);
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
  spiram_buffer_ptr   = spiram_buffer[0];

  // reset error state
  spiram_buffer_overruns = 0;

  // init bank/chip checking
  spiram_chip_no = 0;
  spiram_bank_no = 0;
  spiram_buffer_on_chip[0] = 0;

  // setup DMA buffer index
  spiram_num_ready = 0;
  spiram_dma_index = 0;
  spiram_dma_chip_no = 0;
  spiram_dma_busy = 0;

  spiram_total = 0;
  spiram_checksum = 0;

  // pre-select chip 0
  spi_low_set_ram_addr(0);
  spi_low_dma_enable();
  spi_low_enable_cs(SPI_RAM_CS_MASK);
}

int spiram_multi_write_next_buffer(void)
{
  // buffer is full -> get next
  u32 next_index = (spiram_buffer_index + 1) & (SPIRAM_NUM_BUFFER-1);

  // oops! -> DMA is using this buffer :( overflow!
  if(next_index == spiram_dma_index) {
      spiram_buffer_overruns ++;
      return 0;
  } else {
      spiram_buffer_usage = 0;
      spiram_buffer_index = next_index;
      spiram_buffer_ptr   = spiram_buffer[spiram_buffer_index];

      // determine chip
      spiram_bank_no ++;
      if(spiram_bank_no == SPIRAM_NUM_BANKS) {
          spiram_chip_no++;
          spiram_bank_no = 0;

          // write initial WRITE command for new chip
          spiram_buffer_ptr[0] = SPIRAM_CMD_WRITE;
          spiram_buffer_ptr[1] = 0;
          spiram_buffer_ptr[2] = 0;
          spiram_buffer_usage  = 3;
      }

      // store for each buffer the chip
      spiram_buffer_on_chip[next_index] = spiram_chip_no;

      // another buffer is ready
      spiram_num_ready ++;
      return 1;
  }
}

void spiram_multi_write_end(void)
{
  // make last buffer ready
  if(spiram_buffer_usage > 0) {
      spiram_num_ready++;
  }

  // make sure all buffers are transmitted
  while(spiram_num_ready > 0) {
      spiram_multi_write_handle();
  }

  // make sure last DMA finished
  while(!spi_low_rx_dma_empty());

  spi_low_dma_disable();
  spi_low_disable_cs(SPI_RAM_CS_MASK);
}
