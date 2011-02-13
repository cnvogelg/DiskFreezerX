#include "trk_read.h"
#include "spiram.h"
#include "floppy-low.h"
#include "timer.h"
#include "uartutil.h"
#include "delay.h"
#include "util.h"
#include "pit.h"

// max number of index pos
#define MAX_INDEX      16
#define MAX_OVERFLOWS  32

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
static volatile u32 idx_counter;
static volatile u32 idx_flag;

#define NO_IDX_COUNT 0xf0000000

static void index_func(void)
{
    index_pos[idx_counter] = pit_peek();
    idx_counter++;
    idx_flag=1;
}

u32 trk_read_count_index(void)
{
    reset_status();

    uart_send_string((u08 *)"index search. max=");
    uart_send_hex_dword_crlf(max_index);

    idx_counter = 0;

    pit_set_max(0);
    pit_enable();
    pit_reset();

    floppy_enable_index_intr(index_func);

    // wait for index
    while(idx_counter<max_index) {
        // no index found
        if(pit_peek() > NO_IDX_COUNT) {
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

    floppy_disable_index_intr();

    pit_disable();

    // store in global status
    got_index = idx_counter;

    return idx_counter;
}

// ----- count data -----
// count the number of bitcells on a track

u32 trk_read_count_data(void)
{
    reset_status();

    timer2_init();

    u32 max = max_index;

    idx_counter = 0;

    pit_set_max(0);
    pit_enable();
    pit_reset();

    floppy_enable_index_intr(index_func);

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

    timer2_enable();
    timer2_trigger();
    u32 data_counter = 0;
    u32 cell_overruns = 0;
    idx_counter = 0;
    while(idx_counter < max) {

        // check bit cell timer
        u32 status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            // found a bitcell delta
            data_counter ++;
            timer2_get_capture_a();
        }
        if(status & AT91C_TC_LOVRS) {
            cell_overruns ++;
        }
    }

    timer2_disable();

    floppy_disable_index_intr();
    pit_disable();

    if(cell_overruns > 0) {
        uart_send_string((u08 *)"cell  overruns: ");
        uart_send_hex_dword_crlf(cell_overruns);
    }

    uart_send_string((u08 *)"data found=");
    uart_send_hex_dword_crlf(data_counter);

    // store in global status
    read_status.cell_overruns = cell_overruns;
    got_index = idx_counter;

    return data_counter;
}

// ----- data spectrum -----

static u16 table[256];

static int do_read_data_spectrum(void)
{
    reset_status();

    timer2_init();

    idx_counter = 0;

    pit_set_max(0);
    pit_enable();
    pit_reset();

    floppy_enable_index_intr(index_func);

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

    // core loop: wait for one index -> one track
    timer2_enable();
    timer2_trigger();
    u32 cell_overruns = 0;
    u32 data_counter = 0;
    u32 value_overflows = 0;
    idx_counter = 0;
    u32 max = max_index;
    while(idx_counter < max) {
        u32 status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            u16 delta = (u16)timer2_get_capture_a();
            if(delta > 255)
                value_overflows++;
            else {
                table[delta]++;
            }
            data_counter++;

        }
        if(status & AT91C_TC_LOVRS) {
            cell_overruns++;
        }
    }

    timer2_disable();
    floppy_disable_index_intr();

    if(cell_overruns > 0) {
        uart_send_string((u08 *)"cell  overruns: ");
        uart_send_hex_dword_crlf(cell_overruns);
    }
    if(value_overflows > 0) {
        uart_send_string((u08 *)"value overflows: ");
        uart_send_hex_dword_crlf(value_overflows);
    }

    read_status.cell_overruns  = cell_overruns;
    read_status.cell_overflows = value_overflows;
    got_index = idx_counter;

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

// ----- read a track -----

static void read_index_func(void)
{
  idx_flag = pit_peek();
}

void trk_read_sim(void)
{
    reset_status();

    uart_send_string((u08 *)"read track sim");
    uart_send_crlf();

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

    uart_send_string((u08 *)"data counter:   ");
    uart_send_hex_dword_crlf(spiram_total);
    uart_send_string((u08 *)"data checksum:  ");
    uart_send_hex_dword_crlf(spiram_checksum);
    if(spiram_buffer_overruns > 0) {
        uart_send_string((u08 *)"data overruns:  ");
        uart_send_hex_dword_crlf(spiram_buffer_overruns);
    }

    // update status
    read_status.data_size = spiram_total;
    read_status.data_overruns = spiram_buffer_overruns;
    read_status.data_checksum = spiram_checksum;
    read_status.full_chips = spiram_chip_no;

    trk_check_spiram();
}

void trk_read_to_spiram(void)
{
    u32 cell_overflows = 0;
    u32 cell_overruns = 0;
    u32 index_counter = max_index;
    u32 my_index[MAX_INDEX];
    u32 cell_overflow_values[MAX_OVERFLOWS];

    reset_status();
    
    uart_send_string((u08 *)"read track to ram");
    uart_send_crlf();

    // prepare timers
    timer2_init();

    idx_counter = 0;

    pit_set_max(0);
    pit_enable();
    pit_reset();

    floppy_enable_index_intr(read_index_func);

    // begin bulk transfer
    spi_low_mst_init();
    spiram_multi_init();
    spiram_multi_write_begin();

    // wait for an index marker
    idx_flag = 0;
    while(!idx_flag) {}
    idx_flag = 0;

    timer2_enable();
    timer2_trigger();

    // preload to erase
    u32 status = timer2_get_status();
    u32 delta = timer2_get_capture_a();

    // core loop for reading a track
    u08 ch = 0;
    while(index_counter > 0) {

        // cell sample timer
        status = timer2_get_status();
        if(status & AT91C_TC_LDRAS) {
            // new cell delta
            delta = timer2_get_capture_a();

            // overflow?
            if(delta >= LAST_VALUE) {
                spiram_multi_write_byte(MARKER_OVERFLOW);
                cell_overflow_values[cell_overflows] = delta;
                cell_overflows++;
            } else {
                //u08 d = (u08)(delta & 0xff);
                spiram_multi_write_byte(ch++);
            }

            // index marker was found
            if(idx_flag) {
                spiram_multi_write_byte(MARKER_INDEX);
                index_counter--;
                my_index[index_counter] = idx_flag;
                idx_flag = 0;
            }

            // handle SPI transfer
            spiram_multi_write_handle();
        }
        if(status & AT91C_TC_LOVRS) {
            cell_overruns++;
        }
    }

    timer2_disable();
    spiram_multi_write_end();

    floppy_disable_index_intr();
    pit_disable();

    uart_send_string((u08 *)"data counter:   ");
    uart_send_hex_dword_crlf(spiram_total);
    uart_send_string((u08 *)"data checksum:  ");
    uart_send_hex_dword_crlf(spiram_checksum);
    if(spiram_buffer_overruns > 0) {
        uart_send_string((u08 *)"data overruns:  ");
        uart_send_hex_dword_crlf(spiram_buffer_overruns);
    }
    if(cell_overflows > 0) {
        uart_send_string((u08 *)"cell overflows: ");
        uart_send_hex_dword_crlf(cell_overflows);
        for(int i=0;i<cell_overflows;i++) {
            uart_send_string((u08 *)"cell counter:   ");
            uart_send_hex_dword_crlf(cell_overflow_values[i]);
        }
    }
    if(cell_overruns > 0) {
        uart_send_string((u08 *)"cell overruns:  ");
        uart_send_hex_dword_crlf(cell_overruns);
    }

    // update status
    read_status.cell_overruns  = cell_overruns;
    read_status.cell_overflows = cell_overflows;
    read_status.data_size = spiram_total;
    read_status.data_overruns = spiram_buffer_overruns;
    read_status.data_checksum = spiram_checksum;
    read_status.full_chips = spiram_chip_no;
    got_index = idx_counter;

    // copy index pos
    for(int i=0;i<max_index;i++) {
        index_pos[i] = my_index[max_index-1-i];

        if(i > 0) {
            uart_send_string((u08 *)"    delta:  ");
            uart_send_hex_dword_crlf(index_pos[i] - index_pos[i-1]);
        }
        uart_send_string((u08 *)"index pos:  ");
        uart_send_hex_dword_crlf(index_pos[i]);
    }

    trk_check_spiram();
}

u32 trk_check_spiram(void)
{
   spiram_multi_init();
   u32 size = read_status.data_size + (read_status.full_chips * 3);
   u32 chip_no = 0;
   u32 bank = 0;
   u32 addr = 0;
   u32 my_checksum = 0;

   spi_low_set_multi(chip_no);
   while(size > 0) {
       u08 *data = &spiram_buffer[0][0];
       u32  delta = (size < SPIRAM_BUFFER_SIZE) ? size : SPIRAM_BUFFER_SIZE;

       spiram_read_begin(addr);
       spi_read_dma(data,SPIRAM_BUFFER_SIZE);
       spiram_end();

       // correct check size in last bank of each chip
       // to skip the 3 bytes DMA optimization
       u32 check_size = delta;
       if((bank == (SPIRAM_NUM_BANKS-1)) && (delta == SPIRAM_BUFFER_SIZE)) {
           check_size = delta - 3;
       }

       for(int i=0;i<check_size;i++) {
           my_checksum += data[i];
       }

       bank++;
       addr+=SPIRAM_BUFFER_SIZE;
       if(bank == SPIRAM_NUM_BANKS) {
           chip_no ++;
           bank = 0;
           addr = 0;
           spi_low_set_multi(chip_no);
       }
       size -= delta;
   }

   u32 checksum = read_status.data_checksum;
   uart_send_string((u08 *)"read checksum:  ");
   uart_send_hex_dword_crlf(checksum);
   uart_send_string((u08 *)"calc checksum:  ");
   uart_send_hex_dword_crlf(my_checksum);

   u32 diff;
   if(my_checksum > checksum) {
       diff = my_checksum - checksum;
   } else {
       diff = checksum - my_checksum;
   }
   uart_send_string((u08 *)"delta:          ");
   uart_send_hex_dword_crlf(diff);

   return (my_checksum != checksum);
}


