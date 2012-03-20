#include "buffer.h"

#include "spiram.h"
#include "uart.h"
#include "uartutil.h"

typedef struct
{
  u08 track;
  u32 size;
  u32 checksum;
} buffer_info_t;

static buffer_info_t buf;

void buffer_clear(void)
{
  buf.track = 0;
  buf.size = 0;
  buf.checksum = 0;
}

void buffer_set(u08 track, u32 size, u32 checksum)
{
  buf.track = track;
  buf.size = size;
  buf.checksum = checksum;
}

u32 buffer_get_size(void)
{
  return buf.size;
}

u08 buffer_get_track(void)
{
  return buf.track;
}

u32 buffer_get_checksum(void)
{
  return buf.checksum;
}

void buffer_info(void)
{
  uart_send_string((u08 *)"BI: ");
  uart_send_hex_byte_space(buf.track);
  uart_send_hex_dword_space(buf.size);
  uart_send_hex_dword_crlf(buf.checksum);
}

int
buffer_write(io_func write_func)
{
  // work buffer for data transfer is shared with spiram's buffers
  u08 *data = &spiram_buffer[0][0];

  // setup SPIRAM
  u08 err = spiram_multi_init();
  if (err)
    {
      return 0x200 | err;
    }

  u32 bank = 0;
  u32 addr = 0;
  u32 chip_no = 0;
  spi_low_set_ram_addr(0);
  u32 total = 0;
  u32 checksum = 0;
  u32 size = buf.size;
  while (size > 0)
    {

      // read block from SPIRAM
      spi_low_set_channel(SPI_RAM_CHANNEL);
      if (spiram_set_mode(SPIRAM_MODE_SEQ) != SPIRAM_MODE_SEQ)
        {
          return 1;
        }
      spiram_read_begin(addr);
      spi_read_dma(data, SPIRAM_BUFFER_SIZE);
      spiram_end();

      u32 blk_check = 0;
      for (int i = 0; i < SPIRAM_BUFFER_SIZE; i++)
        {
          blk_check += data[i];
        }

      u32 blk_size = (size > SPIRAM_BUFFER_SIZE) ? SPIRAM_BUFFER_SIZE : size;
      if ((bank == (SPIRAM_NUM_BANKS - 1)) && (blk_size > (SPIRAM_BUFFER_SIZE
          - 3)))
        {
          blk_size = SPIRAM_BUFFER_SIZE - 3;
        }
      for (int i = 0; i < blk_size; i++)
        {
          checksum += data[i];
        }
      total += blk_size;

      // call write function
      int result = write_func(data, blk_size);
      if (result)
        {
          return result;
        }

      size -= blk_size;
      bank++;
      addr += SPIRAM_BUFFER_SIZE;
      if (bank == SPIRAM_NUM_BANKS)
        {
          bank = 0;
          addr = 0;
          chip_no++;

          spi_low_set_ram_addr(chip_no);
        }
    }

  // compare checksum
  if (checksum != buf.checksum)
    {
      return 0x300;
    }

  return 0;
}
