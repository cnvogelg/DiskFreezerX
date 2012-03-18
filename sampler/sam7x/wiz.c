#include "wiz.h"
#include "wiz_low.h"
#include "util.h"
#include "rtc.h"
#include "delay.h"

#define WIZ_MR          0x0000
#define WIZ_GAR         0x0001
#define WIZ_SUBR        0x0005
#define WIZ_SHAR        0x0009
#define WIZ_SIPR        0x000f
#define WIZ_IR          0x0015
#define WIZ_IMR         0x0016
#define WIZ_RTR         0x0017
#define WIZ_RCR         0x0019
#define WIZ_RMSR        0x001a
#define WIZ_TMSR        0x001b
#define WIZ_UIPR        0x002a
#define WIZ_UPORT       0x002e

#define WIZ_Sx_MR       0x00
#define WIZ_Sx_CR       0x01
#define WIZ_Sx_IR       0x02
#define WIZ_Sx_SR       0x03
#define WIZ_Sx_PORT     0x04
#define WIZ_Sx_DHAR     0x06
#define WIZ_Sx_DIPR     0x0c
#define WIZ_Sx_DPORT    0x10
#define WIZ_Sx_MSSR     0x12
#define WIZ_Sx_PROTO    0x14
#define WIZ_Sx_TOS      0x15
#define WIZ_Sx_TTL      0x16
#define WIZ_Sx_TX_FSR   0x20
#define WIZ_Sx_TX_RD    0x22
#define WIZ_Sx_TX_WR    0x24
#define WIZ_Sx_RX_RSR   0x26
#define WIZ_Sx_RX_RD    0x28

// flags for registers
#define WIZ_Sx_MR_MULTI         0x80
#define WIZ_Sx_MR_NO_DELAY_ACK  0x20
#define WIZ_Sx_MR_TCP           0x01
#define WIZ_Sx_MR_UDP           0x02
#define WIZ_Sx_MR_IPRAW         0x03
#define WIZ_Sx_MR_MACRAW        0x04

#define WIZ_Sx_CR_OPEN          0x01
#define WIZ_Sx_CR_LISTEN        0x02
#define WIZ_Sx_CR_CONNECT       0x04
#define WIZ_Sx_CR_DISCON        0x08
#define WIZ_Sx_CR_CLOSE         0x10
#define WIZ_Sx_CR_SEND          0x20
#define WIZ_Sx_CR_SEND_MAC      0x21
#define WIZ_Sx_CR_SEND_KEEP     0x22
#define WIZ_Sx_CR_RECV          0x40

#define WIZ_Sx_IR_SEND_OK       0x10
#define WIZ_Sx_IR_TIMEOUT       0x08
#define WIZ_Sx_IR_RECV          0x04
#define WIZ_Sx_IR_DISCON        0x02
#define WIZ_Sx_IR_CON           0x01

#define WIZ_Sx_SR_SOCK_CLOSED   0x00
#define WIZ_Sx_SR_SOCK_INIT     0x13
#define WIZ_Sx_SR_SOCK_LISTEN   0x14
#define WIZ_Sx_SR_SOCK_ESTABLISHED 0x17
#define WIZ_Sx_SR_SOCK_CLOSE_WAIT 0x1c
#define WIZ_Sx_SR_SOCK_UDP      0x22
#define WIZ_Sx_SR_SOCK_IPRAW    0x32
#define WIZ_Sx_SR_SOCK_MACRAW   0x42
#define WIZ_Sx_SR_SOCK_PPOE     0x52

#define WIZ_Sx_SR_SOCK_SYNSENT  0x15
#define WIZ_Sx_SR_SOCK_SYNRECV  0x16
#define WIZ_Sx_SR_SOCK_FIN_WAIT 0x18
#define WIZ_Sx_SR_SOCK_CLOSING  0x1a
#define WIz_Sx_SR_SOCK_TIME_WAIT 0x1b
#define WIZ_Sx_SR_SOCK_LAST_ACK 0x1d
#define WIZ_Sx_SR_SOCK_ARP      0x01

// memory map
#define WIZ_SOCKET0_BASE        0x0400
#define WIZ_SOCKET1_BASE        0x0500
#define WIZ_SOCKET2_BASE        0x0600
#define WIZ_SOCKET3_BASE        0x0700

#define WIZ_S0(x) (WIZ_SOCKET0_BASE + x)
#define WIZ_S1(x) (WIZ_SOCKET1_BASE + x)
#define WIZ_S2(x) (WIZ_SOCKET2_BASE + x)
#define WIZ_S3(x) (WIZ_SOCKET3_BASE + x)

#define WIZ_TX_BASE             0x4000
#define WIZ_RX_BASE             0x6000

void wiz_init(void)
{
  wiz_low_init();

  // setup common
  wiz_low_begin();
  wiz_low_write(WIZ_MR, 0);
  wiz_low_write(WIZ_IMR, 0);
  wiz_low_write_word(WIZ_RTR, 0xfa0); // 400ms retry time
  wiz_low_write(WIZ_RCR, 5); // retry count
  wiz_low_write(WIZ_RMSR, 0x55); // 2k each socket rx mem
  wiz_low_write(WIZ_TMSR, 0x55); // 2k each socket tx mem
  wiz_low_end();

  wiz_load_from_sram();
}

void wiz_set_ip(int type, u08 ip[4])
{
  wiz_low_begin();
  for(int i=0;i<4;i++) {
      wiz_low_write(type+i,ip[i]);
  }
  wiz_low_end();
}

void wiz_set_mac(u08 ip[6])
{
  wiz_low_begin();
  for(int i=0;i<6;i++) {
      wiz_low_write(WIZ_SHAR+i,ip[i]);
  }
  wiz_low_end();
}

void wiz_get_ip(int type, u08 ip[4])
{
  wiz_low_begin();
  for(int i=0;i<4;i++) {
      ip[i] = wiz_low_read(type+i);
  }
  wiz_low_end();
}

void wiz_get_mac(u08 mac[6])
{
  wiz_low_begin();
  for(int i=0;i<6;i++) {
      mac[i] = wiz_low_read(WIZ_SHAR+i);
  }
  wiz_low_end();
}

static char *ip_str = "00.00.00.00";
static char *mac_str = "00:00:00:00:00:00";

char *wiz_get_ip_str(int type)
{
  u08 ip[4];
  wiz_get_ip(type, ip);
  int pos = 0;
  for(int i=0;i<4;i++) {
      byte_to_hex(ip[i],(u08 *)(ip_str+pos));
      pos += 3;
  }
  return ip_str;
}

char *wiz_get_mac_str(void)
{
  u08 mac[6];
  wiz_get_mac(mac);
  int pos = 0;
  for(int i=0;i<6;i++) {
      byte_to_hex(mac[i],(u08 *)(mac_str+pos));
      pos += 3;
  }
  return mac_str;
}

void wiz_load_from_sram(void)
{
  // write gw addr, subnet mask, mac, source ip: 4 * ip + 1 * mac = 18
  u08 data[18];
  rtc_read_sram(RTC_MEM_OFFSET_WIZ_IP,data,RTC_MEM_SIZE_WIZ_IP);

  // write to wiznet
  u16 addr = 1;
  wiz_low_begin();
  for(int i=0;i<18;i++) {
      wiz_low_write(addr, data[i]);
      addr++;
  }
  wiz_low_end();
}

void wiz_save_to_sram(void)
{
  u08 data[18];

  // read from wiznet
  u16 addr = 1;
  wiz_low_begin();
  for(int i=0;i<18;i++) {
      data[i] = wiz_low_read(addr);
      addr++;
  }
  wiz_low_end();

  // save to sram
  rtc_write_sram(RTC_MEM_OFFSET_WIZ_IP, data, RTC_MEM_SIZE_WIZ_IP);
}

static int wiz_tcp_socket_init(u16 src_port)
{
  int r;

  // retries
  for(r = 0;r < 5;r++) {
    // set TCP mode
    wiz_low_write(WIZ_S0(WIZ_Sx_MR), WIZ_Sx_MR_TCP);
    // set src port
    wiz_low_write_word(WIZ_S0(WIZ_Sx_PORT), src_port);

    // set open command
    wiz_low_write(WIZ_S0(WIZ_Sx_CR), WIZ_Sx_CR_OPEN);

    // make sure init state is reached
    u08 status = wiz_low_read(WIZ_S0(WIZ_Sx_SR));
    if(status == WIZ_Sx_SR_SOCK_INIT) {
       return 0;
    }
  }
  return 1;
}

int wiz_begin_tcp_client(u16 src_port, u08 dst_ip[4], u16 dst_port)
{
  wiz_low_begin();

  if(wiz_tcp_socket_init(src_port)) {
      wiz_low_end();
      return 1;
  }

  // set destination
  for(int i=0;i<4;i++) {
      wiz_low_write(WIZ_S0(WIZ_Sx_DIPR)+i,dst_ip[i]);
  }
  wiz_low_write_word(WIZ_S0(WIZ_Sx_DPORT),dst_port);

  // connect
  wiz_low_write(WIZ_S0(WIZ_Sx_CR), WIZ_Sx_CR_CONNECT);

  // wait for established
  for(int i=0;i<1000;i++) {
      u08 status = wiz_low_read(WIZ_S0(WIZ_Sx_SR));
      if(status == WIZ_Sx_SR_SOCK_ESTABLISHED) {
          wiz_low_end();
          return 0;
      }
      delay_ms(1);
  }
  wiz_low_end();
  return 2;
}

int wiz_end_tcp_client(void)
{
  // disconnect
  wiz_low_write(WIZ_S0(WIZ_Sx_CR), WIZ_Sx_CR_DISCON);
  return 0;
}

static int wiz_wait_tx_free(u16 expect)
{
  // get free buffer
  for(int i=0;i<1000;i++) {
      // make sure to check status
      u08 status = wiz_low_read(WIZ_S0(WIZ_Sx_SR));
      if(status != WIZ_Sx_SR_SOCK_ESTABLISHED) {
          return 1;
      }

      // get free buffer size
      u16 free = wiz_low_read_word(WIZ_S0(WIZ_Sx_TX_FSR));
      if(free >= expect) {
          return 0; // got it!
      }

      // wait a bit and try again
      delay_ms(1);
  }
  return 2;
}

int wiz_write_tcp(u08 *data, u16 size)
{
  int error = wiz_wait_tx_free(size);
  if(error) {
      return error;
  }

  return 0;
}
