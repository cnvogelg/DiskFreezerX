#include "status.h"
#include "uart.h"
#include "uartutil.h"

status_t status;

void status_reset(void)
{
  status.samples = 0;
  status.cell_overruns  = 0;
  status.word_values = 0;
  status.timer_overflows = 0;
  status.data_overruns = 0;
}

void status_info(void)
{
  uart_send_string((u08 *)"SC: ");
  uart_send_hex_dword_space(status.samples);
  uart_send_hex_dword_space(status.word_values);
  uart_send_hex_dword_crlf(status.timer_overflows);

  uart_send_string((u08 *)"SE: ");
  uart_send_hex_dword_space(status.cell_overruns);
  uart_send_hex_dword_crlf(status.data_overruns);
}
