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

typedef struct msg_t {
    struct iphdr ip;
    struct tcphdr tcp;
    struct {
        uint8_t kind;
        uint8_t len;
        uint16_t val;
    } tcpopt_mss;
    struct {
        uint8_t kind;
        uint8_t len;
        uint8_t val;
    } tcpopt_wscale;
    uint8_t pad[1];
} msg_t;

#define MAX_CLIENT_NUM 5

typedef struct clients_desc_t {
    uint16_t c_ports[MAX_CLIENT_NUM];
    uint32_t mapped_addr[MAX_CLIENT_NUM];
    uint32_t active_clients;
} clients_desc_t;

uint16_t get_mapped_port(clients_desc_t* clients, uint32_t addr);
uint32_t get_mapped_addr(clients_desc_t* clients, uint16_t port);

uint16_t tcp_checksum(const struct iphdr *ip, const char *buf, size_t len);
uint16_t ip4_checksum(const struct iphdr *ip);
void write_or_die(int fd, const char *buf, size_t len);


void transfer_ip4_tcp(int fd, char* buf, size_t len, clients_desc_t* clients);
void transfer_ip4_udp(int fd, char* buf, size_t len);

#endif