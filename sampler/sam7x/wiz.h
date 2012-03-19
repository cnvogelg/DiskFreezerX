/* wiz.h
 * wiznet ethernet stack chip API
 */

#ifndef WIZ_H
#define WIZ_H

#include "target.h"

// IP config struct
typedef struct ip_cfg {
  u16   src_port;       // +0
  u16   tgt_port;       // +2
  u08   src_ip[4];      // +4
  u08   tgt_ip[4];      // +8
  u08   net_msk[4];     // +12
  u08   gw_ip[4];       // +16
  u08   mac_addr[6];    // +20
} wiz_cfg_t;            // +26

// common

extern void wiz_init(void);

// config handling
extern wiz_cfg_t *wiz_get_cfg(void);
extern void wiz_realize_cfg(void);
extern void wiz_load_cfg(void);
extern void wiz_save_cfg(void);
extern char *wiz_get_ip_str(const u08 ip[4]);
extern char *wiz_get_mac_str(const u08 mac[6]);

// tcp client
extern int wiz_begin_tcp_client(void);
extern int wiz_end_tcp_client(void);

extern int wiz_write_tcp(const u08 *data, u32 size);

#endif
