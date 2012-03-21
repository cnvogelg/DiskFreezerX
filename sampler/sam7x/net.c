#include "net.h"
#include "wiz.h"

#include "uart.h"
#include "uartutil.h"
#include "buffer.h"

void net_init(void)
{
  wiz_init();
  wiz_load_cfg();
  wiz_realize_cfg();
}

void net_info(void)
{
  wiz_cfg_t *cfg = wiz_get_cfg();

  // src ip:port
  uart_send_string((u08 *)"NS: ");
  uart_send_string((u08 *)wiz_get_ip_str(cfg->src_ip));
  uart_send(':');
  uart_send_hex_word_crlf(cfg->src_port);

  // tgt ip:port
  uart_send_string((u08 *)"NT: ");
  uart_send_string((u08 *)wiz_get_ip_str(cfg->tgt_ip));
  uart_send(':');
  uart_send_hex_word_crlf(cfg->tgt_port);

  // net mask
  uart_send_string((u08 *)"NM: ");
  uart_send_string((u08 *)wiz_get_ip_str(cfg->net_msk));
  uart_send_crlf();

  // gw ip
  uart_send_string((u08 *)"NG: ");
  uart_send_string((u08 *)wiz_get_ip_str(cfg->gw_ip));
  uart_send_crlf();

  // mac addr
  uart_send_string((u08 *)"NA: ");
  uart_send_string((u08 *)wiz_get_mac_str(cfg->mac_addr));
  uart_send_crlf();
}

u08 net_send_buffer(void)
{
  return 0;
}

