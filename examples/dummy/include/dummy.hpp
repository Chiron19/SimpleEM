#ifndef PLAYGROUND_DUMMY
#define PLAYGROUND_DUMMY

#include <netinet/in.h>

const char SERV_ADDR[] = "172.16.0.2";
const int PORT = 5555;
const int MAXLINE = 160;

int setup_socket(struct sockaddr_in* servaddr);
void sigint_handler(int signum);

#endif // PLAYGROUND_DUMMY