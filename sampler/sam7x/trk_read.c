#include "trk_read.h"
#include "spiram.h"
#include "floppy-low.h"
#include "timer.h"
#include "uartutil.h"
#include "delay.h"
#include "util.h"
#include "pit.h"
#include "track.h"

// max number of indexes per track
#define MAX_INDEX      8

// pit timer value to wait for no index
#define NO_IDX_COUNT 0x500000

// number of samples to keep in ring buffer
#define MAX_SAMPLE_BUF  16
#define SAMPLE_BUF_MASK (MAX_SAMPLE_BUF-1)

// ========== DATA ============================================================

// index store
static u32 max_index = 3;
static u32 got_index = 0;

static volatile u32 index_pos[MAX_INDEX];
static volatile u32 idx_counter;
static volatile u32 idx_flag;

// sample store
static volatile u32 sample_buf[MAX_SAMPLE_BUF];
static volatile u32 sample_put_pos = 0;
static volatile u32 sample_get_pos = 0;

static volatile u32 sample_counter = 0;
static volatile u32 sample_overruns = 0;

// external status info
static read_status_t read_status;

// ========== IRQ HANDLER =====================================================

// index irq -> store next index and set flag
static void index_func(void)
{
  index_pos[idx_counter] = pit_peek();
  idx_counter++;
  idx_flag=1;
}

// data func timer irq -> store sample (and error conditions)
static void data_func(void)
{
  // check bit cell timer
  u32 status = timer2_get_status();

  // store sample
  if(status & AT91C_TC_LDRAS) {
      u32 delta = timer2_get_capture_a();
      u32 pos = sample_put_pos;
      sample_buf[pos] = delta;
      sample_counter ++;
      sample_put_pos = (pos + 1) & SAMPLE_BUF_MASK;
  }
  // sample was overrun
  if(status & AT91C_TC_LOVRS) {
      sample_overruns ++;
  }
  // counter overflowed
  if(status & AT91C_TC_COVFS) {
      sample_buf[sample_put_pos] += 0x10000;
  }
}

// ========== FUNCTIONS =======================================================

// ----- status -----

void reset_status(void)
{
  read_status.index_overruns = 0;
  read_status.cell_overruns  = 0;
  read_status.cell_overflows = 0;
  read_status.timer_overflows = 0;
  read_status.data_size = 0;
  read_status.data_overruns = 0;
  read_status.data_checksum = 0;
  read_status.full_chips = 0;

  got_index = 0;
}

read_status_t *trk_read_get_status(void)
{
  return &read_status;
}

// ----- manage index counter -----

void trk_read_set_max_index(u32 num_index)
{
  if(num_index <= MAX_INDEX) {
      max_index = num_index;
  }
}

u32  trk_read_get_num_index(void)
{
  return got_index;
}

u32  trk_read_get_index_pos(u32 i)
{
  return index_pos[i];
}

// ------ index -----
// try to find 5 index markers on disk
// and measure their distance

u32 trk_read_count_index(void)
{
  reset_status();

  uart_send_string((u08 *)"index search. max=");
  uart_send_hex_dword_crlf(max_index);

  idx_counter = 0;

  pit_set_max(0);
  pit_enable();
  pit_reset();

  floppy_low_enable_index_intr(index_func);

  // wait for index
  while(idx_counter<max_index) {
      // no index found
      u32 p = pit_peek();
      if(p > NO_IDX_COUNT) {
          break;
      }
  }

  if(idx_counter > 0) {
      for(int i=0;i<idx_counter;i++) {
          uart_send_string((u08 *)"pos=");
          uart_send_hex_dword_crlf(index_pos[i]);
      }
  } else {
      uart_send_string((u08 *)"no index found!");
      uart_send_crlf();
  }

  floppy_low_disable_index_intr();

  pit_disable();

  // store in global status
  got_index = idx_counter;

  return idx_counter;
}

// ----- count data -----------------------------------------------------------
// count the number of bitcells on a track

u32 trk_read_count_data(void)
{
  reset_status();

  timer2_init();

  idx_counter = 0;

  pit_set_max(0);
  pit_enable();
  pit_reset();

  floppy_low_enable_index_intr(index_func);

  // wait for an index
  while(idx_counter < 1) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // fatal: no index found
  if(idx_counter == 0) {
      return 0;
  }

  idx_counter = 0;
  sample_put_pos = 0;
  sample_get_pos = 0;
  sample_counter = 0;
  sample_overruns = 0;
  u32 own_counter = 0;

  timer2_enable_intr(data_func);
  timer2_enable();
  timer2_trigger();

  while(idx_counter < max_index) {
      // read sample buffer
      while(1) {
          u32 pos = sample_get_pos;
          // nothing to do
          if(pos == sample_put_pos) {
              break;
          }

          // now process sample but we ignore this here...
          own_counter ++;

          // next pos
          pos = (pos + 1) & SAMPLE_BUF_MASK;
          sample_get_pos = pos;
      }
  }

  timer2_disable();
  timer2_disable_intr();

  floppy_low_disable_index_intr();
  pit_disable();

  if(sample_overruns > 0) {
      uart_send_string((u08 *)"sample overruns: ");
      uart_send_hex_dword_crlf(sample_overruns);
  }

  uart_send_string((u08 *)"samples put=");
  uart_send_hex_dword_crlf(sample_counter);
  uart_send_string((u08 *)"samples get=");
  uart_send_hex_dword_crlf(own_counter);

  // store in global status
  read_status.cell_overruns = sample_overruns;
  got_index = idx_counter;

  return got_index;
}

// ----- data spectrum -----

static u16 table[256];

static int do_read_data_spectrum(void)
{
  reset_status();

  timer2_init();

  // reset state
  idx_counter = 0;

  pit_set_max(0);
  pit_enable();
  pit_reset();

  floppy_low_enable_index_intr(index_func);

  // wait for an index
  while(idx_counter < 1) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // fatal: no index found
  if(idx_counter == 0) {
      return 0;
  }

  idx_counter = 0;
  sample_put_pos = 0;
  sample_get_pos = 0;
  sample_counter = 0;
  sample_overruns = 0;
  u32 value_overflows = 0;

  // core loop
  timer2_enable_intr(data_func);
  timer2_enable();
  timer2_trigger();

  while(idx_counter < max_index) {
      // read sample buffer
      while(1) {
          u32 pos = sample_get_pos;

          // nothing to do
          if(pos == sample_put_pos) {
              break;
          }

          // read sample
          u32 sample = sample_buf[pos];
          if(sample > 255)
              value_overflows++;
          else {
              table[sample]++;
          }

          // next pos
          pos = (pos + 1) & SAMPLE_BUF_MASK;
          sample_get_pos = pos;
      }
  }

  timer2_disable();
  timer2_disable_intr();

  floppy_low_disable_index_intr();

  if(sample_overruns > 0) {
      uart_send_string((u08 *)"sample overruns: ");
      uart_send_hex_dword_crlf(sample_overruns);
  }
  if(value_overflows > 0) {
      uart_send_string((u08 *)"value overflows: ");
      uart_send_hex_dword_crlf(value_overflows);
  }

  read_status.cell_overruns  = sample_overruns;
  read_status.cell_overflows = value_overflows;
  got_index = idx_counter;

  return got_index;
}

u32 trk_read_data_spectrum(void)
{
  for(u16 i=0;i<256;i++) {
      table[i] = 0;
  }

  u32 counter = do_read_data_spectrum();
  if(counter == 0) {
      return 0;
  }

#define ONE_US   24 
#define FOUR_US  (ONE_US * 4)
#define SIX_US   (ONE_US * 6)
#define EIGHT_US (ONE_US * 8)

#define B0     (FOUR_US - ONE_US)
#define B1     (SIX_US - ONE_US)
#define B2     (EIGHT_US - ONE_US)
#define B3     (EIGHT_US + ONE_US)

  // zero delays are
  // zeros:      1       2       3
  // pattern:   10      100     1000
  // range:    [b0:b1) [b1:b2) [b2:b3)

  // dump range
  // 0123456789
  // xx: xxxxcr
  u08 *buf = (u08 *)"xx: xxxx\r\n";
  u08 *sep = (u08 *)"xx: ----\r\n";
  u16 sum = 0;
  for(u08 i=0;i<255;i++) {
      // draw range border
      if((i==B0)||(i==B1)||(i==B2)||(i==B3)) {
          byte_to_hex(i,sep);
          uart_send_data(sep,10);
      }
      // draw non zero values
      if(table[i] != 0) {
          byte_to_hex(i,buf);
          word_to_hex(table[i],buf+4);
          uart_send_data(buf,10);
          sum += table[i];
      }
  }
  uart_send_string((u08 *)"sumA:  ");
  uart_send_hex_word_crlf(sum);

  // sort in into ranges
  u16 total = 0;
  u16 zero1Count = 0;
  u16 zero2Count = 0;
  u16 zero3Count = 0;

  for(u08 i=B0;i<B1;i++) {
      zero1Count += table[i];
  }
  for(u08 i=B1;i<B2;i++) {
      zero2Count += table[i];
  }
  for(u08 i=B2;i<B3;i++) {
      zero3Count += table[i];
  }
  total = zero1Count + zero2Count + zero3Count;

  uart_send_string((u08 *)"zero1: ");
  uart_send_hex_word_crlf(zero1Count);
  uart_send_string((u08 *)"zero2: ");
  uart_send_hex_word_crlf(zero2Count);
  uart_send_string((u08 *)"zero3: ");
  uart_send_hex_word_crlf(zero3Count);
  uart_send_string((u08 *)"sumZ:  ");
  uart_send_hex_word_crlf(total);
  return counter;
}

// ----- read a track -----

u08 trk_read_sim(int verbose)
{
  reset_status();

  read_status.track_num = track_num();

  if(verbose) {
      uart_send_string((u08 *)"read sim: ");
      uart_send_string(track_name(read_status.track_num));
      uart_send_crlf();
  }

  // begin bulk transfer
  spi_low_mst_init();
  spiram_multi_init();
  spiram_multi_write_begin();

  // core loop for reading a track
  u08 ch = 0;
  u32 num = SPIRAM_TOTAL_SIZE - (SPIRAM_NUM_CHIPS * 3);
  while(num > 0) {

      spiram_multi_write_byte(ch++);
      spiram_multi_write_handle();
      delay_us(2);
      num--;

  }

  spiram_multi_write_end();

  if(verbose) {
      uart_send_string((u08 *)"data counter:   ");
      uart_send_hex_dword_crlf(spiram_total);
      uart_send_string((u08 *)"data checksum:  ");
      uart_send_hex_dword_crlf(spiram_checksum);
      if(spiram_buffer_overruns > 0) {
          uart_send_string((u08 *)"data overruns:  ");
          uart_send_hex_dword_crlf(spiram_buffer_overruns);
      }
  }

  // update status
  read_status.data_size = spiram_total;
  read_status.data_overruns = spiram_buffer_overruns;
  read_status.data_checksum = spiram_checksum;
  read_status.full_chips = spiram_chip_no;

  return trk_check_spiram(verbose);
}

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

// ---------- main track sampler ----------------------------------------------

u08 trk_read_to_spiram(int verbose)
{
  reset_status();

  read_status.track_num = track_num();

  if(verbose) {
      uart_send_string((u08 *)"read ram: ");
      uart_send_string(track_name(read_status.track_num));
      uart_send_crlf();
  }

  // prepare timers
  timer2_init();

  idx_counter = 0;

  pit_set_max(0);
  pit_enable();
  pit_reset();

  floppy_low_enable_index_intr(index_func);

#if 0
  // wait for an index
  while(idx_counter < 1) {
      // no index found!
      if(pit_peek() > NO_IDX_COUNT) {
          break;
      }
  }

  // fatal: no index found
  if(idx_counter == 0) {
      return 0;
  }

  idx_counter = 0;
#endif

  // begin bulk transfer
  spi_low_mst_init();
  spiram_multi_init();
  spiram_multi_write_begin();

  sample_put_pos = 0;
  sample_get_pos = 0;
  sample_counter = 0;
  sample_overruns = 0;
  u32 value_overflows = 0;

  // core loop
  timer2_enable_intr(data_func);
  timer2_enable();
  timer2_trigger();

  u32 first = 1;

  // core loop for reading a track
  while(idx_counter < max_index) {

      // read sample buffer
      while(1) {
          // index marker was found
          if(idx_flag) {
              spiram_multi_write_byte(MARKER_INDEX);
              idx_flag = 0;
          }

          u32 pos = sample_get_pos;
          // nothing to do
          if(pos == sample_put_pos) {
              break;
          }

          if(first) {
              first = 0;
          } else {
            // read sample
            u32 delta = sample_buf[pos];

            // overflow?
            if(delta > LAST_VALUE) {
                spiram_multi_write_byte(MARKER_OVERFLOW);
                spiram_multi_write_byte((u08)((delta >> 8)&0xff));
                spiram_multi_write_byte((u08)(delta & 0xff));
                value_overflows++;
            } else {
                u08 d = (u08)(delta & 0xff);
                spiram_multi_write_byte(d);
            }
          }

          // next pos
          pos = (pos + 1) & SAMPLE_BUF_MASK;
          sample_get_pos = pos;
      }

      // handle SPI transfer
      spiram_multi_write_handle();
  }

  timer2_disable();
  timer2_disable_intr();
  spiram_multi_write_end();

  floppy_low_disable_index_intr();
  pit_disable();

  if(verbose) {
      uart_send_string((u08 *)"data counter:   ");
      uart_send_hex_dword_crlf(spiram_total);
      uart_send_string((u08 *)"data checksum:  ");
      uart_send_hex_dword_crlf(spiram_checksum);
      if(spiram_buffer_overruns > 0) {
          uart_send_string((u08 *)"data overruns:  ");
          uart_send_hex_dword_crlf(spiram_buffer_overruns);
      }
      if(value_overflows > 0) {
          uart_send_string((u08 *)"value overflows: ");
          uart_send_hex_dword_crlf(value_overflows);
      }
      if(sample_overruns > 0) {
          uart_send_string((u08 *)"sample overruns: ");
          uart_send_hex_dword_crlf(sample_overruns);
      }
  }

  // update status
  read_status.cell_overruns  = sample_overruns;
  read_status.cell_overflows = value_overflows;
  read_status.data_size = spiram_total;
  read_status.data_overruns = spiram_buffer_overruns;
  read_status.data_checksum = spiram_checksum;
  read_status.full_chips = spiram_chip_no;
  got_index = idx_counter;

  if(verbose) {
      uart_send_string((u08 *)"    index pos:  ");
      for(int i=0;i<idx_counter;i++) {
          uart_send_hex_dword_space(index_pos[i]);
      }
      uart_send_crlf();
      uart_send_string((u08 *)"        delta:  ");
      for(int i=1;i<max_index;i++) {
          uart_send_hex_dword_space(index_pos[i] - index_pos[i-1]);
      }
      uart_send_crlf();
  }

  return trk_check_spiram(verbose);
}

// ----- check spiram contents vs. data checksum ------------------------------

u08 trk_check_spiram(int verbose)
{
  if(verbose) {
      uart_send_string((u08 *)"check ram: ");
      uart_send_string(track_name(read_status.track_num));
      uart_send_crlf();
  }

  spiram_multi_init();

  u32 size = read_status.data_size + (read_status.full_chips * 3);
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

  u32 checksum = read_status.data_checksum;

  if(verbose) {
      uart_send_string((u08 *)"read checksum:  ");
      uart_send_hex_dword_crlf(checksum);
      uart_send_string((u08 *)"calc checksum:  ");
      uart_send_hex_dword_crlf(my_checksum);
      uart_send_string((u08 *)"total size:     ");
      uart_send_hex_dword_crlf(total);

      u32 diff;
      if(my_checksum > checksum) {
          diff = my_checksum - checksum;
      } else {
          diff = checksum - my_checksum;
      }
      uart_send_string((u08 *)"delta:          ");
      uart_send_hex_dword_crlf(diff);
  }

  return (my_checksum != checksum);
}


