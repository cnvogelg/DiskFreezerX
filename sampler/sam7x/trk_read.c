#include "trk_read.h"
#include "spi.h"
#include "floppy-low.h"
#include "timer.h"
#include "uartutil.h"
#include "delay.h"
#include "util.h"

// max number of index pos
#define MAX_INDEX 20

// ----- read track state -----

// index store
static u32 index_pos[MAX_INDEX];
static u32 max_index = 5;
static u32 got_index = 0;

static read_status_t read_status;

// ----- status -----

void reset_status(void)
{
  read_status.index_overruns = 0;
  read_status.cell_overruns  = 0;
  read_status.cell_overflows = 0;
  read_status.data_size = 0;
  read_status.data_overruns = 0;

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

   // setup index timer
    timer1_init();

    u32 index_counter = 0;
    u32 load_overruns = 0;
    u32 max = max_index;

    // measure 5 index runs
    timer1_enable();
    timer1_trigger();

    // wait for index
    while(index_counter < max) {
        u32 status = timer1_get_status();
        if(status & AT91C_TC_LDRAS) {
            index_pos[index_counter] = timer1_get_capture_a();
            index_counter++;
        }
        // missed a load
        if(status & AT91C_TC_LOVRS) {
            load_overruns ++;
        }
        // counter overflow!
        if(status & AT91C_TC_COVFS) {
            break;
        }
    }

    if(load_overruns > 0) {
        uart_send_string((u08 *)"overruns=");
        uart_send_hex_dword_crlf(load_overruns);
    }
    if(index_counter > 0) {
        for(int i=0;i<index_counter;i++) {
            uart_send_string((u08 *)"pos=");
            uart_send_hex_dword_crlf(index_pos[i]);
        }
    } else {
        uart_send_string((u08 *)"no index found!");
        uart_send_crlf();
    }

    timer1_disable();

    // store in global status
    read_status.index_overruns = load_overruns;
    got_index = index_counter;

    return index_counter;
}

// ----- count data -----
// count the number of bitcells on a track

u32 trk_read_count_data(void)
{
    reset_status();

    timer1_init();
    timer2_init();

    u32 max = max_index;

    // wait for an index
    timer1_enable();
    timer1_trigger();
    int index_counter = 0;
    u32 index_overruns = 0;
    while(index_counter < 1) {
        u32 status = timer1_get_status();
        if(status & AT91C_TC_LDRAS) {
            index_counter++;
            timer1_get_capture_a();
        }
        if(status & AT91C_TC_LOVRS) {
            index_overruns ++;
        }
        if(status & AT91C_TC_COVFS) {
            break;
        }
    }

    if(index_counter == 0) {
        timer1_disable();
        return 0;
    }

    timer2_enable();
    timer2_trigger();
    u32 data_counter = 0;
    u32 cell_overruns = 0;
    index_counter = 0;
    while(index_counter < max) {

        // check bit cell timer
        u32 status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            // found a bitcell delta
            data_counter ++;
            timer2_get_capture_a();

            // check index timer
            u32 statusi = timer1_get_status();
            if(statusi & AT91C_TC_LDRAS) {
                index_counter ++;
                timer1_get_capture_a();
            }
            if(statusi & AT91C_TC_LOVRS) {
                index_overruns++;
            }
        }
        if(status & AT91C_TC_LOVRS) {
            cell_overruns ++;
        }
    }

    timer2_disable();
    timer1_disable();

    if(index_overruns > 0) {
        uart_send_string((u08 *)"index overruns: ");
        uart_send_hex_dword_crlf(index_overruns);
    }
    if(cell_overruns > 0) {
        uart_send_string((u08 *)"cell  overruns: ");
        uart_send_hex_dword_crlf(cell_overruns);
    }

    uart_send_string((u08 *)"data found=");
    uart_send_hex_dword_crlf(data_counter);

    // store in global status
    read_status.index_overruns = index_overruns;
    read_status.cell_overruns = cell_overruns;
    got_index = index_counter;

    return data_counter;
}

// ----- data spectrum -----

static u16 table[256];

static int do_read_data_spectrum(void)
{
  reset_status();

  timer1_init();
  timer2_init();

  // wait for an index
  timer1_enable();
  timer1_trigger();
  int index_counter = 0;
  u32 index_overruns = 0;
  while(index_counter < 1) {
      u32 status = timer1_get_status();
      if(status & AT91C_TC_LDRAS) {
          index_counter++;
          timer1_get_capture_a();
      }
      if(status & AT91C_TC_LOVRS) {
          index_overruns ++;
      }
      if(status & AT91C_TC_COVFS) {
          break;
      }
  }

  // no index found!
  if(index_counter == 0) {
      timer1_disable();
      return 0;
  }

  // core loop: wait for one index -> one track
  timer2_enable();
  timer2_trigger();
  u32 cell_overruns = 0;
  u32 data_counter = 0;
  u32 value_overflows = 0;
  index_counter = 0;
  u32 max = max_index;
  while(index_counter < max) {
      u32 status = timer2_get_status();
      if(status & AT91C_TC_LDRAS) {
          u16 delta = (u16)timer2_get_capture_a();
          if(delta > 255)
              value_overflows++;
          else {
              table[delta]++;
          }
          data_counter++;

          u32 statusi = timer1_get_status();
          if(statusi & AT91C_TC_LDRAS) {
              timer1_get_capture_a();
              index_counter++;
          }
          if(statusi & AT91C_TC_LOVRS) {
              index_overruns++;
          }

      }
      if(status & AT91C_TC_LOVRS) {
          cell_overruns++;
      }
  }

  timer2_disable();
  timer1_disable();

  if(index_overruns > 0) {
      uart_send_string((u08 *)"index overruns: ");
      uart_send_hex_dword_crlf(index_overruns);
  }
  if(cell_overruns > 0) {
      uart_send_string((u08 *)"cell  overruns: ");
      uart_send_hex_dword_crlf(cell_overruns);
  }
  if(value_overflows > 0) {
      uart_send_string((u08 *)"value overflows: ");
      uart_send_hex_dword_crlf(value_overflows);
  }

  read_status.index_overruns = index_overruns;
  read_status.cell_overruns  = cell_overruns;
  read_status.cell_overflows = value_overflows;
  got_index = index_counter;

  return data_counter;
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

// ----- spi slave test -----

#define SPI_BLOCK_SIZE  4096

void trk_read_dummy(u32 num)
{
    uart_send_string((u08 *)"size/blk: ");
    uart_send_hex_dword_crlf(SPI_BLOCK_SIZE);
    uart_send_string((u08 *)"blocks:   ");
    uart_send_hex_dword_crlf(num);
    
    spi_bulk_begin();
    for(u32 j=0;j<num;j++) {
        for(u32 i=0;i<SPI_BLOCK_SIZE;i++) {
            // retry if not ready
            u08 d = (u08)(i & 0x7f) +1;
            spi_bulk_write_byte(d);
            spi_bulk_handle();
            delay_us(2);
        }        
    }
    spi_bulk_end();
    
    uart_send_string((u08 *)"done");
    uart_send_crlf();

    
}

// ----- read a track -----

void trk_read_real(void)
{
    u32 cell_overflows = 0;
    u32 cell_overruns = 0;
    u32 my_data_counter = 0;
    u32 index_counter = max_index;
    u32 index_overruns = 0;
    u32 idx_pos[MAX_INDEX];

    reset_status();
    
    uart_send_string((u08 *)"read track: ");
    uart_send_crlf();

    // prepare timers
    timer1_init();
    timer2_init();

    // begin bulk transfer
    spi_bulk_begin();

    // wait for an index marker
    timer1_enable();
    timer1_trigger();
    while(1) {
        u32 status = timer1_get_status();
        if(status & AT91C_TC_LDRAS) {
            timer1_get_capture_a();
            break;
        }
        if(status & AT91C_TC_LOVRS) {
            index_overruns ++;
        }
        if(status & AT91C_TC_COVFS) {
            break;
        }

        spi_bulk_handle();
    }

    timer2_enable();
    timer2_trigger();

    // preload to erase
    u32 status = timer2_get_status();
    u32 delta = timer2_get_capture_a();

    // core loop for reading a track
    while(index_counter > 0) {

        // cell sample timer
        status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            // new cell delta
            delta = timer2_get_capture_a();

            // overflow?
            if(delta >= LAST_VALUE) {
                spi_bulk_write_byte(MARKER_OVERFLOW);
                cell_overflows++;
            } else {
                u08 d = (u08)(delta & 0xff);
                spi_bulk_write_byte(d);
            }
            my_data_counter++;

            // index marker was found
            u32 statusi = timer1_get_status();
            if(statusi & AT91C_TC_LDRAS) {
                u32 index_delta = timer1_get_capture_a();
                spi_bulk_write_byte(MARKER_INDEX);
                my_data_counter++;
                index_counter--;
                idx_pos[index_counter] = index_delta;
            }
            // oops. index overun?!
            if(statusi & AT91C_TC_LOVRS) {
                index_overruns++;
            }

            // handle SPI transfer
            spi_bulk_handle();
        }
        if(status & AT91C_TC_LOVRS) {
            cell_overruns++;
        }
    }

    timer2_disable();
    timer1_disable();
    spi_bulk_end();

    uart_send_string((u08 *)"data counter: ");
    uart_send_hex_dword_crlf(my_data_counter);
    if(index_overruns > 0) {
        uart_send_string((u08 *)"index overruns: ");
        uart_send_hex_dword_crlf(index_overruns);
    }
    if(cell_overflows > 0) {
        uart_send_string((u08 *)"cell overflows: ");
        uart_send_hex_dword_crlf(cell_overflows);
    }
    if(cell_overruns > 0) {
        uart_send_string((u08 *)"cell overruns:  ");
        uart_send_hex_dword_crlf(cell_overruns);
    }

    // update status
    read_status.index_overruns = index_overruns;
    read_status.cell_overruns  = cell_overruns;
    read_status.cell_overflows = cell_overflows;
    read_status.data_size = my_data_counter;
    read_status.data_overruns = spi_write_overruns;
    got_index = index_counter;

    for(int i=0;i<index_counter;i++) {
        index_pos[i] = idx_pos[i];
    }
}

