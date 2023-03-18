#ifndef TUNTAP_PLAYGROUND_UTILS
#define TUNTAP_PLAYGROUND_UTILS

#include <stdio.h>
#include <stdlib.h>

#define panic(_str) do { perror(_str); abort(); } while (0)

#define RET_UNKNOWN 0
#define RET_IPV4_UDP 1
#define RET_IPV4_TCP 2
#define RET_IPV4_ 3
#define RET_IPV6 11

void dump(const char *buf, size_t len);

#endif