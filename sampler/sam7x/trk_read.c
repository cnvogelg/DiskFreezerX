#include "trk_read.h"

#include "spiram.h"
#include "floppy-low.h"
#include "timer.h"
#include "uartutil.h"
#include "delay.h"
#include "util.h"
#include "pit.h"
#include "track.h"
#include "status.h"
#include "index.h"
#include "spectrum.h"
#include "buffer.h"
#include "error.h"

// PIT timeout if no index was found
#define NO_IDX_COUNT 0x500000

// ------ index -----
// try to find 5 index markers on disk
// and measure their distance
static volatile u32 idx_counter;
static volatile u32 idx_flag;

static void index_func(void)
{
  index_add(pit_peek());
  idx_counter++;
  idx_flag=1;
}

error_t trk_read_count_index(void)
{
  status_reset();
  index_reset();

  // setup HW
  pit_set_max(0);
  pit_enable();
  pit_reset();
  floppy_low_enable_index_intr(index_func);

  // wait for index
  idx_counter = 0;
  u32 max_index = index_get_max_index();
  while(idx_counter<max_index) {
      // no index found
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // cleanup HW
  floppy_low_disable_index_intr();
  pit_disable();

  // show results
  index_info();

  if(idx_counter == 0) {
      return ERROR_SAMPLER_NO_INDEX;
  } else {
      return STATUS_OK;
  }
}

// ----- count data -----
// count the number of bitcells on a track

error_t trk_read_count_data(void)
{
  error_t result = STATUS_OK;

  status_reset();
  index_reset();

  // setup HW
  timer2_init();
  pit_set_max(0);
  pit_enable();
  pit_reset();
  floppy_low_enable_index_intr(index_func);

  // wait for an index
  idx_counter = 0;
  while(idx_counter < 1) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // fatal: no index found
  if(idx_counter == 1) {
    index_reset();

    timer2_enable();
    timer2_trigger();

    u32 samples = 0;
    u32 cell_overruns = 0;
    u32 max_index = index_get_max_index();
    idx_counter = 0;
    while(idx_counter < max_index) {

        // check bit cell timer
        u32 status_flag = timer2_get_status();
        if(status_flag & AT91C_TC_LDRAS) {
            // found a bitcell delta
            samples ++;
            timer2_get_capture_a();
        }
        if(status_flag & AT91C_TC_LOVRS) {
            cell_overruns ++;
        }
    }

    // update status
    status.samples = samples;
    status.cell_overruns = cell_overruns;
    if(cell_overruns > 0) {
        result = ERROR_SAMPLER_CELL_OVERRUN;
    }

    timer2_disable();
  } else {
    result = ERROR_SAMPLER_NO_INDEX;
  }

  // cleanup HW
  floppy_low_disable_index_intr();
  pit_disable();

  // show results
  index_info();
  status_info();
  return result;
}

// ----- data spectrum -----

static u16 table[256];

error_t trk_read_data_spectrum(int verbose)
{
  error_t result = STATUS_OK;

  // clear table
  for(u16 i=0;i<256;i++) {
      table[i] = 0;
  }

  status_reset();
  index_reset();

  // setup HW
  timer2_init();
  pit_set_max(0);
  pit_enable();
  pit_reset();
  floppy_low_enable_index_intr(index_func);

  // wait for an index
  idx_counter = 0;
  while(idx_counter < 1) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // fatal: no index found
  if(idx_counter == 1) {
    index_reset();

    // core loop: wait for one index -> one track
    timer2_enable();
    timer2_trigger();

    u32 cell_overruns = 0;
    u32 samples = 0;
    u32 word_values = 0;
    u32 max_index = index_get_max_index();
    idx_counter = 0;
    while(idx_counter < max_index) {
        u32 status_flag = timer2_get_status();
        if(status_flag & AT91C_TC_LDRAS) {
            u16 delta = (u16)timer2_get_capture_a();
            if(delta > 255) {
                word_values++;
            }
            else {
                table[delta]++;
            }
            samples++;
        }
        if(status_flag & AT91C_TC_LOVRS) {
            cell_overruns++;
        }
    }

    timer2_disable();

    // update values
    status.samples = samples;
    status.cell_overruns = cell_overruns;
    status.word_values = word_values;
    if(cell_overruns > 0) {
        result = ERROR_SAMPLER_CELL_OVERRUN;
    }

  } else {
    result = ERROR_SAMPLER_NO_INDEX;
  }

  // cleanup HW
  floppy_low_disable_index_intr();
  pit_disable();

  // show results
  index_info();
  status_info();
  spectrum_info(table, verbose);
  return result;
}

// ----- read a track -----

error_t trk_read_sim(int verbose)
{
  status_reset();
  index_reset();
  buffer_clear();

  if(verbose) {
      uart_send_string((u08 *)"RF: ");
      uart_send_hex_byte_crlf(track_num());
  }

  // begin bulk transfer
  spi_low_mst_init();
  spiram_multi_init();
  spiram_multi_write_begin();

  // core loop for reading a track
  u08 ch = 0;
  u32 num = SPIRAM_TOTAL_SIZE - (SPIRAM_NUM_CHIPS * 3);
  status.samples = num;
  while(num > 0) {

      spiram_multi_write_byte(ch++);
      spiram_multi_write_handle();
      delay_us(2);
      num--;

  }

  spiram_multi_write_end();

  buffer_set(track_num(), spiram_total, spiram_checksum);

  if(verbose) {
      buffer_info();
  }

  status.data_overruns = spiram_buffer_overruns;
  if(spiram_buffer_overruns > 0) {
      return ERROR_SAMPLER_DATA_OVERRUN;
  } else {
      return STATUS_OK;
  }
}

// ---------- main track sampler ----------------------------------------------

__inline void spiram_multi_write_word(u16 word)
{
  spiram_multi_write_byte((u08)(word >> 8));
  spiram_multi_write_byte((u08)(word & 0xff));
}

__inline void spiram_multi_write_dword(u32 dword)
{
  spiram_multi_write_byte((u08)(dword >> 24));
  spiram_multi_write_byte((u08)((dword >> 16)&0xff));
  spiram_multi_write_byte((u08)((dword >> 8)&0xff));
  spiram_multi_write_byte((u08)(dword & 0xff));
}

// faster index func
static void read_index_func(void)
{
  idx_flag = pit_peek();
}

error_t trk_read_to_spiram(int verbose)
{
  u32 byte_values = 0;
  u32 word_values = 0;
  u32 cell_overruns = 0;
  u32 timer_overflows = 0;
  u32 index_counter = index_get_max_index();
  u32 my_index[MAX_INDEX];

  u08 code[16];
  u32 code_size = 0;
  u32 found_index = 1;

  status_reset();
  index_reset();
  buffer_clear();

  if(verbose) {
      uart_send_string((u08 *)"rm: ");
      uart_send_hex_byte_crlf(track_num());
  }

  // begin bulk transfer
  spi_low_mst_init();
  spiram_multi_init();
  spiram_multi_write_begin();

  // setup HW
  timer2_init();
  pit_set_max(0);
  pit_enable();
  pit_reset();
  floppy_low_enable_index_intr(read_index_func);

  // wait for an index marker
  idx_flag = 0;
  while(!idx_flag) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          // set counter to zero to skip main loop
          index_counter = 0;
          found_index = 0;
          break;
      }
  }
  idx_flag = 0;

  timer2_enable();
  timer2_trigger();

  // preload to erase
  u32 status_flag = timer2_get_status();
  u32 delta = timer2_get_capture_a();

#define PUT_CODE(x) code[code_size++] = x

  // core loop for reading a track
  while(index_counter) {
      status_flag = timer2_get_status();

      if(status_flag & AT91C_TC_LDRAS) {
          // new cell delta
          delta = timer2_get_capture_a();

          // does not fit in a byte?
          if(delta > 0xff) {
              PUT_CODE(MARKER_WORD_VALUE);
              PUT_CODE((u08)((delta >> 8)&0xff));
              PUT_CODE((u08)(delta & 0xff));
              word_values++;
          } else {
              u08 d = (u08)(delta & 0xff);
              PUT_CODE(d);
              byte_values++;
          }
      } else {
        // timer overflow
        if(status_flag & AT91C_TC_COVFS) {
            PUT_CODE(MARKER_TIMER_OVERFLOW);
            timer_overflows++;
        }
      }

      // load overrun in capture register
      if(status_flag & AT91C_TC_LOVRS) {
          cell_overruns++;
          PUT_CODE(MARKER_OVERRUN);
      }

      // write SPI ram
      if(code_size) {
           // write to SPI ram
           for(int i=0;i<code_size;i++) {
               spiram_multi_write_byte(code[i]);
           }
           code_size = 0;

           spiram_multi_write_handle();
      }

      // handle index
      if(idx_flag) {
        PUT_CODE(MARKER_INDEX);
        index_counter--;
        my_index[index_counter] = idx_flag;
        idx_flag = 0;
      }
  }

  timer2_disable();
  floppy_low_disable_index_intr();
  pit_disable();
  spiram_multi_write_end();

  // store values
  error_t result = STATUS_OK;
  if(found_index) {
    index_counter = index_get_max_index();
    for(int i=0;i<index_counter;i++) {
        index_add(my_index[index_counter-1-i]);
    }
    status.samples = word_values + byte_values;
    status.word_values = word_values;
    status.cell_overruns = cell_overruns;
    status.data_overruns = spiram_buffer_overruns;
    status.timer_overflows = timer_overflows;
    buffer_set(track_num(), spiram_total, spiram_checksum);

    if(verbose) {
        index_info();
        status_info();
        buffer_info();
    }

    if(status.data_overruns > 0) {
        result |= ERROR_SAMPLER_DATA_OVERRUN;
    }
    if(status.cell_overruns > 0) {
        result |= ERROR_SAMPLER_CELL_OVERRUN;
    }

    result |= trk_check_spiram(verbose);

  } else {
    result = ERROR_SAMPLER_NO_INDEX;
  }
  return result;
}

// ----- check spiram contents vs. data checksum ------------------------------

error_t trk_check_spiram(int verbose)
{
  if(verbose) {
      uart_send_string((u08 *)"rv: ");
      uart_send_hex_byte_crlf(track_num());
  }

  spiram_multi_init();

  u32 size = spiram_total + (spiram_chip_no * 3);
  u32 chip_no = 0;
  u32 bank = 0;
  u32 addr = 0;
  u32 my_checksum = 0;

  spi_low_set_ram_addr(chip_no);
  u32 total = 0;
  while(size > 0) {
      u08 *data = &spiram_buffer[0][0];
      u32  delta = (size < SPIRAM_BUFFER_SIZE) ? size : SPIRAM_BUFFER_SIZE;

      spiram_read_begin(addr);
      spi_read_dma(data,SPIRAM_BUFFER_SIZE);
      spiram_end();

      // correct check size in last bank of each chip
      // to skip the 3 bytes DMA optimization
      u32 check_size = delta;
      if((bank == (SPIRAM_NUM_BANKS-1)) && (delta > (SPIRAM_BUFFER_SIZE-3))) {
          check_size = SPIRAM_BUFFER_SIZE - 3;
      }

      u32 blk_check = 0;
      for(int i=0;i<check_size;i++) {
          my_checksum += data[i];
          blk_check += data[i];
      }
      total += check_size;

      bank++;
      addr+=SPIRAM_BUFFER_SIZE;
      if(bank == SPIRAM_NUM_BANKS) {
          chip_no ++;
          bank = 0;
          addr = 0;
          spi_low_set_ram_addr(chip_no);

#if 0
          uart_send_string((u08 *)"blk ");
          uart_send_hex_dword_crlf(blk_check);
#endif
      }
      size -= delta;
  }

  u32 checksum = buffer_get_checksum();

  if(verbose) {
      uart_send_string((u08 *)"BC: ");
      uart_send_hex_dword_space(total);
      uart_send_hex_dword_space(checksum);
      uart_send_hex_dword_space(my_checksum);

      u32 diff;
      if(my_checksum > checksum) {
          diff = my_checksum - checksum;
      } else {
          diff = checksum - my_checksum;
      }
      uart_send_hex_dword_crlf(diff);
  }

  if(my_checksum != checksum) {
      return ERROR_SAMPLER_CHECKSUM_MISMATCH;
  } else {
      return STATUS_OK;
  }
}


