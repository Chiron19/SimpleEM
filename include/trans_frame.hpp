#ifndef TUNTAP_PLAYGROUND_TRANS_FRAME
#define TUNTAP_PLAYGROUND_TRANS_FRAME

#define _DEFAULT_SOURCE 1

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>

#include <stdlib.h>
#include <unistd.h>

uint16_t tcp_checksum(const struct iphdr *ip, const char *buf, size_t len);
uint16_t ip4_checksum(const struct iphdr *ip);
void write_or_die(int fd, const char *buf, size_t len);

#endif