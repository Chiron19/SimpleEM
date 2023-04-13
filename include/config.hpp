#ifndef SIMPLEEM_CONFIG
#define SIMPLEEM_CONFIG

#include <stddef.h>

#define EXTRA_PROCESSES 1

const char PROGRAM_PATH[] = "dummy"; 
const char *PROGRAM_ARGV[] = {"dummy", NULL};

char TUN_DEV_NAME[] = "tun0";
const char TUN_ADDR[] = "172.16.0.1";
const char TUN_MASK[] = "255.240.0.0";

// #define _DEFAULT_SOURCE 1

// #include <arpa/inet.h>
// #include <netinet/tcp.h>

// typedef struct {
//     int fd;
//     int ifd;
// } fds_t;

// typedef struct {
//     struct iphdr ip;
//     struct tcphdr tcp;
//     uint8_t tcp_opt[20];
// } msg_opt20_t;

#endif // SIMPLEEM_CONFIG