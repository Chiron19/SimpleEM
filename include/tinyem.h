#ifndef TUNTAP_PLAYGROUND_EXAMPLE
#define TUNTAP_PLAYGROUND_EXAMPLE

#define _DEFAULT_SOURCE 1

#include <arpa/inet.h>
#include <netinet/tcp.h>

typedef struct {
    int fd;
    int ifd;
} fds_t;

typedef struct {
    struct iphdr ip;
    struct tcphdr tcp;
    uint8_t tcp_opt[20];
} msg_opt20_t;

#endif