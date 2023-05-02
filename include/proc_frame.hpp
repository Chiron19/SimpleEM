#pragma once

#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct sender_desc_t{
    struct sockaddr_in addr;
    int protocol;
} sender_desc_t;

sender_desc_t process(const char *buf, size_t len);
sender_desc_t process_ip6(const char *buf, size_t len);
sender_desc_t process_ip4(const char *buf, size_t len);
sender_desc_t process_ip4_tcp(const struct iphdr *ip, const char *buf, size_t len);
sender_desc_t process_ip4_udp(const struct iphdr *ip, const char *buf, size_t len);
