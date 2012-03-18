/* wiz.h
 * wiznet ethernet stack chip API
 */

#ifndef WIZ_H
#define WIZ_H

#include "target.h"

// common

extern void wiz_init(void);

#define WIZ_IP_TYPE_GATEWAY     0x01
#define WIZ_IP_TYPE_SUBNET_MASK 0x05
#define WIZ_IP_TYPE_SOURCE      0x0f

extern void wiz_set_ip(int type, u08 ip[4]);
extern void wiz_set_mac(u08 ip[6]);

extern void wiz_get_ip(int type, u08 ip[4]);
extern void wiz_get_mac(u08 mac[6]);

extern char *wiz_get_ip_str(int type);
extern char *wiz_get_mac_str(void);

extern void wiz_load_from_sram(void);
extern void wiz_save_to_sram(void);

extern int wiz_begin_tcp_client(u16 src_port, u08 dst_ip[4], u16 dst_port);
extern int wiz_end_tcp_client(void);

#endif
