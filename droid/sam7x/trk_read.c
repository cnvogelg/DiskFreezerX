#include "trk_read.h"
#include "spi.h"
#include "floppy-low.h"
#include "timer.h"
#include "uartutil.h"
#include "delay.h"
#include "util.h"

// ------ index -----

static volatile u32 index_counter;

void index_counter_inc(void)
{
    index_counter++;
}

u32 trk_read_count_index(void)
{
    floppy_enable_index_intr((func)index_counter_inc);

    // sync to first index
    index_counter = 0;
    delay_ms(3000);
    
    floppy_disable_index_intr();
    
    uart_send_string((u08 *)"index found=");
    uart_send_hex_dword_crlf(index_counter);
    return index_counter;
}

// ----- count data -----

static volatile u32 data_counter;
static u32 min,max;

static void count_data_func(u16 delta)
{
    data_counter++;
    if(delta < min)
        min = delta;
    if(delta > max)
        max = delta;
}

u32 trk_read_count_data(void)
{
    floppy_enable_index_intr((func)index_counter_inc);

    timer2_init();
    timer2_irq_enable(count_data_func);
    data_counter = 0;
    min = 0xffffffff;
    max = 0;

    // wait for sync
    index_counter = 0;
    while(index_counter < 10);
    
    // wait for n syncs
    index_counter = 0;
    timer2_start();    
    while(index_counter == 0);
    timer2_stop();

    floppy_disable_index_intr();
    
    uart_send_string((u08 *)"data found=");
    uart_send_hex_dword_crlf(data_counter);
    uart_send_string((u08 *)"min=");
    uart_send_hex_word_crlf(min);
    uart_send_string((u08 *)"max=");
    uart_send_hex_word_crlf(max);
    return data_counter;
}

// ----- data spectrum -----

static u16 table[256];
static u16 overflows;
static u32 overruns;

void data_spectrum_func(u16 delta)
{
    if(delta > 255) 
        overflows++;
    else {
        table[delta]++;
    }
    data_counter++;
}

void trk_read_data_spectrum_irq(void)
{
  floppy_enable_index_intr(index_counter_inc);

  timer2_init();
  timer2_irq_enable(data_spectrum_func);

  // sync to first index
  index_counter = 0;
  while(index_counter<10);

  // core loop: wait for one index -> one track
  timer2_start();
  while(index_counter < 15) ;
  timer2_stop();

  floppy_disable_index_intr();
}

void trk_read_data_spectrum_raw(void)
{
  floppy_enable_index_intr(index_counter_inc);
  timer2_init();

  // sync to first index
  index_counter = 0;
  while(index_counter<10);

  // core loop: wait for one index -> one track
  timer2_start();
  u32 status = timer2_get_status();
  while(index_counter < 15) {
      status = timer2_get_status();
      if(status & AT91C_TC_LDRAS) {
          u16 delta = (u16)timer2_get_capture_a();
          if(delta > 255)
              overflows++;
          else {
              table[delta]++;
          }
          data_counter++;
      }
      if(status & AT91C_TC_LOVRS) {
          overruns++;
      }
  }
  timer2_stop();

  floppy_disable_index_intr();
}

u32 trk_read_data_spectrum(void)
{
    for(u16 i=0;i<256;i++) {
        table[i] = 0;
    }
    overflows = 0;
    overruns = 0;
    data_counter = 0;

    trk_read_data_spectrum_raw();
    
    uart_send_string((u08 *)"counter:   ");
    uart_send_hex_dword_crlf(data_counter);
    uart_send_string((u08 *)"overflows: ");
    uart_send_hex_word_crlf(overflows);
    uart_send_string((u08 *)"overruns: ");
    uart_send_hex_word_crlf(overruns);

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
    return data_counter;
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

volatile u08 index_flag;
volatile u08 index_total;

#define CACHE_SIZE      256

void index_counter_flag(void)
{
    index_total ++;
    index_flag = timer2_get_timer();
}

void trk_read_real(void)
{
    u32 my_overflows = 0;
    u32 my_overruns = 0;
    u32 my_data_counter = 0;
    
    uart_send_string((u08 *)"read track: ");
    uart_send_crlf();

    floppy_enable_index_intr(index_counter_flag);

    timer2_init();

    // init
    index_flag = 0;
    index_total = 0;

    spi_bulk_begin();
    while(index_total < 1) {
        spi_bulk_handle();
    }

    timer2_start();

    // preload to erase
    u32 status = timer2_get_status();
    u32 delta = timer2_get_capture_a();
    while(index_total < 6) {

        status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            delta = timer2_get_capture_a();

            // overflow?
            if(delta >= LAST_VALUE) {
                spi_bulk_write_byte(MARKER_OVERFLOW);
                my_overflows++;
            } else {
                u08 d = (u08)(delta & 0xff);
                spi_bulk_write_byte(d);
            }
            my_data_counter++;

            // index marker was found
            if(index_flag) {
                spi_bulk_write_byte(MARKER_INDEX);
                spi_bulk_write_byte((u08)index_flag);
                my_data_counter++;
                index_flag = 0;
            }

            spi_bulk_handle();
        }
        if(status & AT91C_TC_LOVRS) {
            my_overruns++;
        }
    }

    timer2_stop();
    spi_bulk_end();

    floppy_disable_index_intr();

    uart_send_string((u08 *)"data counter: ");
    uart_send_hex_dword_crlf(my_data_counter);
    uart_send_string((u08 *)"t2 overflows: ");
    uart_send_hex_dword_crlf(my_overflows);
    uart_send_string((u08 *)"t2 overruns:  ");
    uart_send_hex_dword_crlf(my_overruns);
    uart_send_string((u08 *)"spi overruns: ");
    uart_send_hex_dword_crlf(spi_write_overruns);
}

