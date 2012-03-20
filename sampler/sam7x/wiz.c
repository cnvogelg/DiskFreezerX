#include "wiz.h"
#include "wiz_low.h"
#include "util.h"
#include "rtc.h"
#include "delay.h"

#include "uartutil.h"

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

#define WIZ_IP_TYPE_GATEWAY     0x01
#define WIZ_IP_TYPE_SUBNET_MASK 0x05
#define WIZ_IP_TYPE_SOURCE      0x0f

// configuration data
static wiz_cfg_t wiz_cfg;

void wiz_init(void)
{
  wiz_low_init();

  wiz_low_begin();

  // reset
  wiz_low_write(WIZ_MR, 0x80); // set reset bit
  for(int i=0;i<1000;i++) {
    if((wiz_low_read(WIZ_MR) & 0x80) == 00) {
        break;
    }
    delay_ms(1);
  }

  wiz_low_write(WIZ_MR, 0);
  wiz_low_write(WIZ_IMR, 0);
  wiz_low_write_word(WIZ_RTR, 0xfa0); // 400ms retry time
  wiz_low_write(WIZ_RCR, 5); // retry count
  wiz_low_write(WIZ_RMSR, 0x55); // 2k each socket rx mem
  wiz_low_write(WIZ_TMSR, 0x55); // 2k each socket tx mem

  wiz_low_end();
}

static void wiz_set_ip(int type, u08 ip[4])
{
  wiz_low_begin();
  for(int i=0;i<4;i++) {
      wiz_low_write(type+i,ip[i]);
  }
  wiz_low_end();
}

static void wiz_set_mac(u08 ip[6])
{
  wiz_low_begin();
  for(int i=0;i<6;i++) {
      wiz_low_write(WIZ_SHAR+i,ip[i]);
  }
  wiz_low_end();
}

wiz_cfg_t *wiz_get_cfg(void)
{
  return &wiz_cfg;
}

void wiz_realize_cfg(void)
{
  wiz_set_ip(WIZ_IP_TYPE_GATEWAY, wiz_cfg.gw_ip);
  wiz_set_ip(WIZ_IP_TYPE_SUBNET_MASK, wiz_cfg.net_msk);
  wiz_set_ip(WIZ_IP_TYPE_SOURCE, wiz_cfg.src_ip);
  wiz_set_mac(wiz_cfg.mac_addr);
}

static char *ip_str = "00.00.00.00";
static char *mac_str = "00:00:00:00:00:00";

char *wiz_get_ip_str(const u08 ip[4])
{
  int pos = 0;
  for(int i=0;i<4;i++) {
      byte_to_hex(ip[i],(u08 *)(ip_str+pos));
      pos += 3;
  }
  return ip_str;
}

char *wiz_get_mac_str(const u08 mac[6])
{
  int pos = 0;
  for(int i=0;i<6;i++) {
      byte_to_hex(mac[i],(u08 *)(mac_str+pos));
      pos += 3;
  }
  return mac_str;
}

void wiz_load_cfg(void)
{
  // load from RTC SRAM
  rtc_read_sram(RTC_MEM_OFFSET_WIZ_CFG,(u08 *)&wiz_cfg,RTC_MEM_SIZE_WIZ_CFG);
}

void wiz_save_cfg(void)
{
  // save to RTC SRAM
  rtc_write_sram(RTC_MEM_OFFSET_WIZ_CFG, (u08 *)&wiz_cfg, RTC_MEM_SIZE_WIZ_CFG);
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

int wiz_begin_tcp_client(void)
{
  wiz_low_begin();

  if(wiz_tcp_socket_init(wiz_cfg.src_port)) {
      wiz_low_end();
      return 1;
  }

  // set destination
  for(int i=0;i<4;i++) {
      wiz_low_write(WIZ_S0(WIZ_Sx_DIPR)+i,wiz_cfg.tgt_ip[i]);
  }
  wiz_low_write_word(WIZ_S0(WIZ_Sx_DPORT),wiz_cfg.tgt_port);

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
  wiz_low_begin();
  wiz_low_write(WIZ_S0(WIZ_Sx_CR), WIZ_Sx_CR_DISCON);
  wiz_low_end();
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

// default setup
#define WIZ_S0_TX_BASE          0x4000
#define WIZ_S0_TX_MASK          0x07ff
#define WIZ_S0_TX_SIZE          (WIZ_S0_TX_MASK+1)

int wiz_write_tcp(const u08 *data, u32 size)
{
  wiz_low_begin();

  //u32 offset = 0;
  int result = 0;
  u32 offset = 0;
  while(size > 0) {

    u16 tx_size;
    if(size > WIZ_S0_TX_SIZE) {
        tx_size = WIZ_S0_TX_SIZE;
    } else {
        tx_size = (u16)size;
    }

    // first wait until free space is here
    int error = wiz_wait_tx_free(tx_size);
    if(error) {
        result = error;
        break;
    }

    // get current wr pos
    u16 wr_pos = wiz_low_read_word(WIZ_S0(WIZ_Sx_TX_WR));

    // write data with wrapping into buffer
    u16 addr = (wr_pos & WIZ_S0_TX_MASK) + WIZ_S0_TX_BASE;
    for(u16 i = 0;i<tx_size; i++) {
        wiz_low_write(addr++, data[offset++]);
        if(addr == (WIZ_S0_TX_BASE + WIZ_S0_TX_SIZE)) {
            addr = WIZ_S0_TX_BASE;
        }
    }

    // set new wr pos
    wiz_low_write_word(WIZ_S0(WIZ_Sx_TX_WR), wr_pos + tx_size);

    // set COMMAND SEND
    wiz_low_write(WIZ_S0(WIZ_Sx_CR), WIZ_Sx_CR_SEND);

    size -= tx_size;
  }

  wiz_low_end();
  return result;
}
