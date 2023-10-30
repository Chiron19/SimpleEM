#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <sstream>

#include "tcp_file.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <des IP address> <des port number>" << std::endl;
        return 1;
    }

    // const char* srcIpAddress = argv[1];
    // const char* srcPortNumber = argv[2];
    const char* desIpAddress = argv[1];
    const char* desPortNumber = argv[2];

    // Validate IP address
    // struct sockaddr_in clientAddr{};
    // if (inet_pton(AF_INET, srcIpAddress, &(clientAddr.sin_addr)) != 1) {   
    //     std::cerr << "Invalid src IP address: " << srcIpAddress << std::endl;
    //     return 1;
    // }
    struct sockaddr_in serverAddr{};
    if (inet_pton(AF_INET, desIpAddress, &(serverAddr.sin_addr)) != 1) {
        std::cerr << "Invalid des IP address: " << desIpAddress << std::endl;
        return 1;
    }

    // Validate port number
    // int srcPort = atoi(srcPortNumber);
    // if (srcPort <= 0 || srcPort > 65535) {
    //     std::cerr << "Invalid src port number: " << srcPortNumber << std::endl;
    //     return 1;
    // }
    int desPort = atoi(desPortNumber);
    if (desPort <= 0 || desPort > 65535) {
        std::cerr << "Invalid des port number: " << desPortNumber << std::endl;
        return 1;
    }

    int clientSocket;

    // setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, (bool *) &iOptVal, sizeof(bool));
    std::string localIpAddress = getLocalIpAddress();
    // std::cout << "local IP address: " << localIpAddress << std::endl;
    char buffer[256] = "Hello from Client, Token";
    // std::strcat(buffer, localIpAddress.c_str());
    // std::strcat(buffer, srcIpAddress);
    std::cout << "Messsage buffer: " << buffer << std::endl;
    
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Socket Created!" << std::endl;
    // clientAddr.sin_family = AF_INET;
    // clientAddr.sin_port = htons(srcPort);
    // bind(clientSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
    // std::cout << "local IP address: " << localIpAddress << std::endl;

    int opt = 1;
    int sndbuf = 32768;
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
        return 1;
    }
    if (setsockopt(clientSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
        return 1;
    }
    if (setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
        std::cout << "Failed to set SO_SNDBUF option. " << strerror(errno) << "\n";
        return 1;
    }

    // Connect to server
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(desPort);
    while (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cout << "Failed to connect client socket." << std::endl;
        return 1;
    }
    std::cout << "Socket Connected!" << std::endl;

    int loopNum = 1;
    while (loopNum--) {
        // Send TCP packet to server
        // std::strcpy(buffer, std::string("Helloworld from client").c_str());
        // std::strcat(buffer, getLocalTime().c_str());
        // int sendStatus = send(clientSocket, buffer, strlen(buffer), 0);
        // if (sendStatus == -1) {
        //     std::cout << "Message Sending Retry!" << std::endl;
        //     sleep(1);
        //     continue;
        // }
        // std::cout << "Client Message Sent!" << std::endl;

        // Sending file through TCP
        int64_t rc = SendFile(clientSocket, "test_file.pdf");
        if (rc < 0) {
            std::cout << "Failed to send file: " << rc << std::endl;
        }
        std::cout << "[Client] File Sent!" << std::endl;

        sleep(1);

        // wait for a message
        int bytesRecv;
        while ((bytesRecv = recv(clientSocket, buffer, sizeof(buffer), 0)) == -1)  {
            std::cerr << "Client: There was a connection issue." << std::endl;
        }
        if (bytesRecv == 0)
        {
            std::cout << "Server Disconnected." << std::endl;
            break;
        }
        
        // display message
        std::cout << "[Client] Received: " << std::string(buffer, 0, bytesRecv) << std::endl;
  
    }

    // sleep(1);
    shutdown(clientSocket, SHUT_RDWR);
    // Close socket
    close(clientSocket);
    std::cout << "Client socket closed!" << std::endl;

    return 0;
}
