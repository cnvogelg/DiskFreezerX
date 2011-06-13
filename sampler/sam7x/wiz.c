#include "wiz.h"
#include "wiz_low.h"
#include "util.h"

#define WIZ_MAC_ADDR    0x0009

void wiz_init(void)
{
  wiz_low_init();
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
      wiz_low_write(WIZ_MAC_ADDR+i,ip[i]);
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
      mac[i] = wiz_low_read(WIZ_MAC_ADDR+i);
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

