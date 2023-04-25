#ifndef PLAYGROUND_DUMMY
#define PLAYGROUND_DUMMY

#include <netinet/in.h>
#include <errno.h>
#include <vector>
#include <fstream>
#include <iostream>

#define MAXLINE 160

void sigint_handler(int signum);

typedef std::pair<int, std::string> message_t; // sender em_id + message

class NetworkInterface {
public:
    int em_id;
    int send_fd, recv_fd;
    std::vector<std::pair<std::string, int>> addresses;

    NetworkInterface(int em_id, std::string config_file): em_id(em_id) {
        std::ifstream config(config_file);
        std::string address;
        int port;
        while(config >> address >> port) {
            addresses.push_back(std::make_pair(address, port));
        }

        setup_recv_socket();
        setup_send_socket();
    }

    void send(int target_em_id, std::string message) {
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        recvaddr.sin_addr.s_addr = inet_addr(addresses[target_em_id]
            .first.c_str());
        if (sendto(send_fd, message.c_str(), message.size(), MSG_CONFIRM, 
               (const struct sockaddr *) &recvaddr, sizeof(recvaddr)) == -1) {
            printf("[DUMMY] sendto error: %d\n", errno);
        }
    }

    /*
        Returns pair - sender em_id + message or
        if nothing was received then -1 + empty
     */
    message_t recv() {
        char buffer[MAXLINE];
        struct sockaddr_in sender_addr;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);

        ssize_t n = recvfrom(recv_fd, (char *)buffer, MAXLINE, 
                     MSG_DONTWAIT, (struct sockaddr *) &sender_addr,
                     &len);

        if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return std::make_pair(-1, std::string());
        }
        buffer[n] = '\0';

        std::string message = std::string(buffer);
        for (int i = 0; i < addresses.size(); ++i) {
            if (inet_addr(addresses[i].first.c_str()) == sender_addr.sin_addr.s_addr)
            //  && htons(addresses[i].second) == sender_addr.sin_port) // FIXME for now
                return std::make_pair(i, message);
        }

        std::cout << "ERROR - SENDER DOESNT EXIST" << std::endl;
        exit(1);
    }

private:
    void setup_send_socket() {
        if ((send_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }
    }
    
    void setup_recv_socket() {        
        if ((recv_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }
        
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(addresses[em_id].second);

        if (bind(recv_fd, (const struct sockaddr *)&servaddr, 
                sizeof(servaddr)) < 0) {
            printf("Socket binding failed...\n");
            exit(0);
        }
    }
};

#endif // PLAYGROUND_DUMMY