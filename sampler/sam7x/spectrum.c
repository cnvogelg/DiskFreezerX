#include "spectrum.h"
#include "uart.h"
#include "uartutil.h"
#include "util.h"

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

void spectrum_info(u16 table[256], int verbose)
{
  // dump range
  // 01234567890123
  // XE: xx: xxxxcr
  static u08 *buf = (u08 *)"XE: xx xxxx\r\n";
  static u08 *sep = (u08 *)"XE: xx ----\r\n";
  u16 sum = 0;
  for(u08 i=0;i<255;i++) {
      sum += table[i];
  }

  if(verbose) {
    for(u08 i=0;i<255;i++) {
        // draw range border
        if((i==B0)||(i==B1)||(i==B2)||(i==B3)) {
            byte_to_hex(i,sep+4);
            uart_send_data(sep,14);
        }
        // draw non zero values
        if(table[i] != 0) {
            byte_to_hex(i,buf+4);
            word_to_hex(table[i],buf+7);
            uart_send_data(buf,14);
        }
    }
  }

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

  uart_send_string((u08 *)"XS: ");
  uart_send_hex_word_space(sum);
  uart_send_hex_word_space(total);
  uart_send_hex_word_space(zero1Count);
  uart_send_hex_word_space(zero2Count);
  uart_send_hex_word_crlf(zero3Count);
}
