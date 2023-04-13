#ifndef TUNTAP_PLAYGROUND_CLIENT
#define TUNTAP_PLAYGROUND_CLIENT

#define MAX_BUFF 128
#define PORT 8080

int set_up_client(struct sockaddr_in* servaddr_ptr);
void mes_rec_server(int sockfd);
void mes_rec_server_periodically(int sockfd);

#endif // TUNTAP_PLAYGROUND_CLIENT