/* net.h
 * high level network commands
 */

#ifndef NET_H
#define NET_H

#include "target.h"

extern void net_init(void);
extern void net_info(void);

extern u08 net_send_buffer(void);

#endif
