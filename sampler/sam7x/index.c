#include "index.h"
#include "uart.h"
#include "uartutil.h"

// index store
static u32 index_pos[MAX_INDEX];
static u08 max_index = 5;
static u08 got_index = 0;

void index_reset(void)
{
  got_index = 0;
  for(int i=0;i<MAX_INDEX;i++) {
      index_pos[i] = 0;
  }
}

void index_set_max_index(u32 num_index)
{
  if(num_index <= MAX_INDEX) {
      max_index = num_index;
  }
}

u32 index_get_max_index(void)
{
  return max_index;
}

u32 index_get_total(void)
{
  return got_index;
}

u32 index_get_index(u32 i)
{
  return index_pos[i];
}

void index_add(u32 pos)
{
  if(got_index < MAX_INDEX) {
      index_pos[got_index++] = pos;
  }
}

void index_info(void)
{
  // get total number of indices
  uart_send_string((u08 *)"IT: ");
  uart_send_hex_byte_space(got_index);
  uart_send_hex_byte_crlf(max_index);

  // show index pos
  u32 last_pos = 0;
  for(int i=0;i<got_index;i++) {
      uart_send_string((u08 *)"ID: ");
      uart_send_hex_byte_space(i);
      uart_send_hex_dword_space(index_pos[i]);
      uart_send_hex_dword_crlf(index_pos[i] - last_pos);
      last_pos = index_pos[i];
  }
}


