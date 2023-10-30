#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>
#include <ctime>
#include <sstream>

#include "tcp_file.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP address> <port number> " << std::endl;
        return 1;
    }

    const char* ipAddress = argv[1];
    const char* portNumber = argv[2];

    // Validate IP address
    struct sockaddr_in serverAddr{};
    if (inet_pton(AF_INET, ipAddress, &(serverAddr.sin_addr)) != 1) {
        std::cerr << "Invalid IP address: " << ipAddress << std::endl;
        return 1;
    }

    // Validate port number
    int port = atoi(portNumber);
    if (port <= 0 || port > 65535) {
        std::cerr << "Invalid port number: " << portNumber << std::endl;
        return 1;
    }

    int serverSocket, newSocket;
    
    // setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (bool *) &iOptVal, sizeof(bool));
    // setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT, (bool *) &iOptVal, sizeof(bool));
    struct sockaddr_storage serverStorage{};
    socklen_t addrSize;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    int opt = 1;
    int rcvbuf = 65536;
    int rcvlowat = 128;
    struct timeval rcvtimeo = {100, 0};
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
        return 1;
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
        return 1;
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
        std::cout << "Failed to set SO_RCVBUF option. " << strerror(errno) << "\n";
        return 1;
    }
    // if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVLOWAT, &rcvlowat, sizeof(rcvlowat)) < 0) {
    //     std::cout << "Failed to set SO_RCVLOWAT option. " << strerror(errno) << "\n";
    //     return 1;
    // }

    // Bind to IP and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int bindStatus = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    while (bindStatus == -1){
        std::cout << "Failed to bind server socket." << std::endl;
        return 1;
    } 

    // Listen for connections
    while (listen(serverSocket, 0) == -1) {
        std::cout << "Error Listening, Retry!" << std::endl;
        sleep(1);
    }
    std::cout << "Server1, Listening" << std::endl;

    // Accept client connection
    addrSize = sizeof(serverStorage);
    while ((newSocket = accept(serverSocket, (struct sockaddr*)&serverStorage, &addrSize)) == -1) {
        std::cout << "Fail to accept!" << std::endl;
    }
    std::cout << "Socket on server1, Accepted" << std::endl;

    // if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    //     std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
    //     return 1;
    // }
    // if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
    //     std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
    //     return 1;
    // }
    // if (setsockopt(newSocket, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
    //     std::cout << "Failed to set SO_RCVBUF option. " << strerror(errno) << "\n";
    //     return 1;
    // }
    // if (setsockopt(newSocket, SOL_SOCKET, SO_RCVTIMEO, &rcvtimeo, sizeof(rcvtimeo)) < 0) {
    //     std::cout << "Failed to set SO_RCVTIMEO option. " << strerror(errno) << "\n";
    //     return 1;
    // }

    char buffer[256];
    
    int loopNum = 1;
    while (loopNum--) {
        // Receive and print received data
        // int bytesRecv = recv(newSocket, buffer, sizeof(buffer), 0);
        // if (bytesRecv == -1)
        // {
        //     std::cerr << "Server: There was a connection issue." << std::endl;
        //     continue;
        // }
        // if (bytesRecv == 0)
        // {
        //     std::cout << "Client Disconnected." << std::endl;
        //     break;
        // }
        
        // // display message
        // std::cout << "Received: " << std::string(buffer, 0, bytesRecv);

        // Receive TCP file
        const int64_t rc = RecvFile(newSocket, "test_file_new.pdf");
        if (rc < 0) {
            std::cout << "Failed to recv file: " << rc << std::endl;
        }
        std::cout << "[Server] File Received!" << std::endl;

        sleep(1);
        
        // Send TCP packet to server
        std::strcpy(buffer, std::string("Helloworld from server").c_str());
        std::strcat(buffer, getLocalTime().c_str());
        while (send(newSocket, buffer, strlen(buffer), 0) == -1) {
            sleep(1);
            std::cout << "Message Sending Retry!" << std::endl;
        }
        std::cout << "[Server] Message Sent!" << std::endl;
        
    }

    // sleep(5);
    // Close sockets
    // shutdown(newSocket, SHUT_RDWR);
    close(newSocket);
    std::cout << "Children socket closed!" << std::endl;
    // shutdown(serverSocket, SHUT_RDWR);
    close(serverSocket);
    std::cout << "Server socket closed!" << std::endl;
    return 0;
}
