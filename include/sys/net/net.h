#ifndef NET_H
#define NET_H

#ifdef __cplusplus
extern "C" {
#endif

bool net_system_init();

void net_system_shutdown();

#ifdef __cplusplus
}
#endif

#endif // NET_H