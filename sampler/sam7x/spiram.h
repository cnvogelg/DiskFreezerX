#ifndef SPIRAM_H
#define SPIRAM_H

#include "board.h"
#include "spi.h"

/* RAM size per chip and total */
#define SPIRAM_CHIP_SIZE        32768
#define SPIRAM_NUM_CHIPS        8
#define SPIRAM_TOTAL_SIZE      (SPIRAM_NUM_CHIPS * SPIRAM_CHIP_SIZE)

/* DMA buffers here on SAM */
#define SPIRAM_BUFFER_SIZE     SPI_BUFFER_SIZE
#define SPIRAM_NUM_BUFFER       4
#define SPIRAM_NUM_BANKS       (SPIRAM_CHIP_SIZE / SPIRAM_BUFFER_SIZE)

/* SPI chip modes */
#define SPIRAM_MODE_BYTE        0x00
#define SPIRAM_MODE_PAGE        0x80
#define SPIRAM_MODE_SEQ         0x40

/* SPI commands of SRAM */
#define SPIRAM_CMD_READ         0x03
#define SPIRAM_CMD_WRITE        0x02
#define SPIRAM_CMD_READ_STATUS  0x05
#define SPIRAM_CMD_WRITE_STATUS 0x01

// ----- spiram functions -----------------------------------------------------

extern void
spiram_init(void);
extern void
spiram_close(void);

extern u08
spiram_set_mode(u08 mode);
extern void
spiram_write_begin(u16 address);
extern void
spiram_read_begin(u16 address);
extern void
spiram_end(void);

__inline void
spiram_write_byte(u08 data)
{
  spi_io(data);
}

__inline u08
spiram_read_byte(void)
{
  return spi_io(0xff);
}

// ----- multi ram ------------------------------------------------------------

extern u08
spiram_multi_init(void);
extern void
spiram_multi_write_begin(void);
extern void
spiram_multi_write_end(void);
extern u08
spiram_multi_clear(u08 value);

extern int
spiram_multi_write_next_buffer(void);

// external vars required for inlined functions
extern u08 spiram_buffer[SPIRAM_NUM_BUFFER][SPIRAM_BUFFER_SIZE];
extern u32 spiram_buffer_on_chip[SPIRAM_NUM_BUFFER];
extern u32 spiram_buffer_index;
extern u32 spiram_buffer_usage;
extern u08 *spiram_buffer_ptr;
extern u32 spiram_buffer_overruns;
extern u32 spiram_chip_no;
extern u32 spiram_bank_no;
extern u32 spiram_num_ready;
extern u32 spiram_dma_index;
extern u32 spiram_dma_chip_no;
extern u32 spiram_dma_busy;
extern u32 spiram_total;
extern u32 spiram_checksum;

/* handle DMA page flipping */
__inline void
spiram_multi_write_handle(void)
{
  if(spi_low_rx_dma_empty()) {

      // disable a running DMA
      if(spiram_dma_busy) {
          spi_low_dma_disable();
          spiram_dma_busy = 0;
      }

      // something new to fill in?
      if (spiram_num_ready > 0) {

          // get next dma buffer
          u08 *ptr = spiram_buffer[spiram_dma_index];
          u32 chip_no = spiram_buffer_on_chip[spiram_dma_index];

          // advance to new write buffer
          spiram_dma_index = (spiram_dma_index + 1) & (SPIRAM_NUM_BUFFER - 1);
          spiram_num_ready--;

          // do we need to toggle chip?
          if (spiram_dma_chip_no != chip_no)
            {
              // enable new chip
              spiram_dma_chip_no = chip_no;

              spi_low_disable_multi();
              spi_low_set_multi(chip_no);
              spi_low_enable_multi();
            }

          // start next DMA
          spi_low_rx_dma_set_first(spi_dummy_buffer, SPIRAM_BUFFER_SIZE);
          spi_low_tx_dma_set_first(ptr, SPIRAM_BUFFER_SIZE);
          spi_low_dma_enable();
          spiram_dma_busy = 1;
      }
  }
}

/* write a byte to the current write buffer */
__inline void
spiram_multi_write_byte(u08 data)
{
  // either the buffer has romm left or fetching a new buffer worked
  if ((spiram_buffer_usage != SPIRAM_BUFFER_SIZE)
      || spiram_multi_write_next_buffer())
    {
      // store in current buffer
      spiram_buffer_ptr[spiram_buffer_usage++] = data;
      spiram_total++;
      spiram_checksum += data;
    }
}

#endif
