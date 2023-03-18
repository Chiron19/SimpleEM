#ifndef TUNTAP_PLAYGROUND_SERVER
#define TUNTAP_PLAYGROUND_SERVER

#include <netinet/in.h>

#define MAX_BUFF 128
#define PORT 8080

#define REPORT_MESSAGES true
#define REPORT_ACTIONS true
#define REPORT_ERRORS true

typedef struct conn_desc_t {
    int connfd;
    struct sockaddr_in addr;
} conn_desc_t;

int set_up_server();
void* thread_routine(void* arg);
void echo_client(conn_desc_t conn_desc);


#endif // TUNTAP_PLAYGROUND_SERVER