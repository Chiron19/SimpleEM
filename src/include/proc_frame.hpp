#pragma once

#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

void dump(const char *buf, size_t len);
void process(const char *buf, size_t len);
void process_ip6(const char *buf, size_t len);
void process_ip4(const char *buf, size_t len);
void process_ip4_tcp(const struct iphdr *ip, const char *buf, size_t len);
void process_ip4_udp(const struct iphdr *ip, const char *buf, size_t len);
