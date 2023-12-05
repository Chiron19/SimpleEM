#pragma once

#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <ctime>
#include <vector>
#include <stack>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/stat.h>

#include "utils.hpp"
#include "logger.hpp"

#define MAXLINE 10000

typedef std::pair<int, std::string> message_t; // sender em_id + message

class NetworkHelper {
public:
    int em_id, procs;
    int send_fd, recv_fd;
    std::vector<std::pair<std::string, int>> addresses;
    std::vector<std::stack<message_t>> inbox;
    
    NetworkHelper(int em_id, const std::string& config_path): em_id(em_id) {  
        // General Setup: read config
        std::ifstream config(config_path);
        std::string address;
        int port;
        while(config >> address >> port) {
            addresses.push_back(std::make_pair(address, port));
            std::stack<message_t> temp;
            inbox.push_back(temp);
        }
        procs = addresses.size();

        // for (int i = 0; i < procs; i++)
        // {
        //     std::cout << "[network-helper] " << addresses[i].first.c_str() << ' ' << addresses[i].second <<  std::endl;
        // }

        // For em_id: setup socket
        setup_recv_socket();
        setup_send_socket();
    }

    /* Dumping the buffer on printing output in formats */
    void dump(const char *buf, size_t len)
    {
        size_t i, j;

        for (i = 0; i < len; i++) {
            if ((i % 8) == 0) printf("%04hx  ", (uint16_t) i);

            printf("%02hhx", buf[i]);
            if ((i % 8) == 3) { printf("  "); }
            else if ((i % 8) == 7) {
                printf("  ");
                for (j = i - 7; j <= i; j++)
                    if ((buf[j] < 32) || (buf[j] > 126)) printf(".");
                    else printf("%c", buf[j]);
                printf("\n");
            } else { printf(" "); }
        }

        if ((i % 8) != 0) {
            for (j = i % 8; j < 8; j++) {
                printf("  ");
                if (j == 3) printf("  ");
                else printf(" ");
            }
            printf(" ");
            for (j = i - (i % 8); j < i; j++)
                if ((buf[j] < 32) || (buf[j] > 126)) printf(".");
                else printf("%c", buf[j]);
            printf("\n");
        }
    }

    /* Return local IP address in string */
    std::string getLocalIpAddress() {
        struct ifaddrs* ifAddrStruct = nullptr;
        struct ifaddrs* ifa = nullptr;
        void* tmpAddrPtr = nullptr;
        std::string localIp;

        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }

            if (ifa->ifa_addr->sa_family == AF_INET) {
                // IPv4 address
                tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                if (strcmp(ifa->ifa_name, "lo") != 0) {
                    // Exclude loopback interface
                    localIp = addressBuffer;
                    break;
                }
            }
        }

        if (ifAddrStruct != nullptr) {
            freeifaddrs(ifAddrStruct);
        }

        return localIp;
    }

    /* Return local time in string */
    std::string getLocalTime() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H-%M-%S");

        return oss.str();
    }

    /*
        Return filesize in bytes
        reference: 
        // https://stackoverflow.com/questions/63494014/sending-files-over-tcp-sockets-c-windows
    */
    int64_t getFileSize(const std::string& filePath) {
        // no idea how to get filesizes > 2.1 GB in a C++ kind-of way.
        // I will cheat and use Microsoft's C-style file API
        FILE* file;
        // https://www.mkssoftware.com/docs/man3/fopen.3.asp
        if ((file = fopen64(filePath.c_str(), "rb")) == NULL) {
            return -1;
        }
        fseeko64(file, 0, SEEK_END);
        const int64_t len = ftello64(file);
        fclose(file);
        return len;
    }

    /*
        Recieves data in to buffer until bufferSize value is met
    */
    int recvBuffer(int socketFd, char* buffer, int bufferSize, int chunkSize = 65536) {
        int i = 0;
        while (i < bufferSize) {
            // std::cout << "receiving buffer at " << i << ", length: " << std::min(chunkSize, bufferSize - i) << std::endl;
            const int l = recv(socketFd, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
            // printf("received %d\n", i);
            if (l < 0) { 
                std::cout << "recv() error at: " << i << "/" << bufferSize << std::endl;
                return l;
            } // this is an error
            if (l == 0) {
                // std::cout << "recv() 0 at: " << i << std::endl;
                return i;
            } // Receive = 0, no more to receive
            i += l;
        }
        return i;
    }

    /*
        Sends data in buffer until bufferSize value is met
        return size being sent
    */
    int sendBuffer(int socketFd, const char* buffer, int bufferSize, int chunkSize = 65536) {
        int i = 0;
        while (i < bufferSize) {
            // printf("sending buffer at %d\n", i);
            const int l = send(socketFd, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
            // printf("sent %d\n", l);
            if (l < 0) {
                return l; 
            } // this is an error
            i += l;
        }
        return i;
    }

     /*  
        Sends a file
        returns <bytes sent, error code>
        returns -1 if file couldn't be opened for input
        returns -2 if couldn't send file length properly
        returns -3 if couldn't send file properly
    */    
    std::pair<int64_t, int64_t> sendFile(int socketFd, const std::string& filePath, const std::streampos read_byte = 0, const int chunkSize = 32768) {

        const int64_t fileSize = getFileSize(filePath);
        std::cout << "[sendFile] File: " << filePath << ", Filesize: " << fileSize << std::endl;
        if (fileSize < 0) { return {0, -1}; }

        std::ifstream file(filePath, std::ifstream::binary);
        if (file.fail()) { return {0, -1}; }

        // Set the position in the file
        file.seekg(read_byte, std::ios::beg);

        // Check if the seek operation was successful
        if (!file) {
            std::cerr << "Failed to set the position in the file." << std::endl;
            return {0, -1};
        }

        // Send file length (if reconnect, neglect)
        if (!read_byte)
            if (sendBuffer(socketFd, reinterpret_cast<const char*>(&fileSize),
                sizeof(fileSize)) != sizeof(fileSize)) {
                return {0, -2};
            }

        char* buffer = new char[chunkSize];
        bool errored = false;
        int64_t i = read_byte;
        while (i < fileSize) {
            const int64_t ssize = std::min(fileSize - i, (int64_t)chunkSize);
            // std::cout << "Sending bytes at: " << i << std::endl;
            if (!file.read(buffer, ssize)) {

                if ( (file.rdstate() & std::ifstream::failbit ) != 0 )
        std::cerr << "Error opening \n";
                if ( (file.rdstate() & std::ifstream::eofbit ) != 0 )
        std::cerr << "Error End-of-File reached on input operation\n";
                if ( (file.rdstate() & std::ifstream::badbit ) != 0 )
        std::cerr << "Error Read/writing on i/o operation\n";

                file.clear();
                /* retry if error occurs */
                // std::cout << "Cannot Read at: " << i << ", Retrying" << std::endl;
                // sleep(1);
                // continue;
                /* return error if error occurs */
                errored = true; 
                std::cout << "Cannot Read at: " << i << std::endl;
                break; 
            }
            int l = sendBuffer(socketFd, buffer, (int)ssize);
            // std::cout << "Sent bytes: " << i << std::endl;
            if (l < 0) {
                int fdtype = isfdtype(socketFd, S_IFSOCK);
                std::string fdtype_message = fdtype == 1 ? " (FD is open on an object of the indicated type)" : (fdtype == 0 ? " (FD is closed)" : " (errors (setting errno))");
                std::cout << "socket status: " << fdtype << fdtype_message << std::endl;
                std::cout << "Sent bytes: " << i << std::endl;
                errored = true;
                break;
            }
            i += l;
            std::cout << "Sent bytes: " << i << std::endl;
        }
        delete[] buffer;

        file.close();

        return {i, errored ? -3 : 0};
    }

    /*    
        Receives a file
        returns <bytes received, error code>
        returns -1 if file couldn't be opened for output
        returns -2 if couldn't receive file length properly
        returns -3 if couldn't receive file properly
        returns -4 if received ack message
    */    
    std::pair<int64_t, int64_t> recvFile(int socketFd, const std::string& filePath, const std::streampos write_byte = 0, const int chunkSize = 65536) {
        std::ofstream file(filePath, std::ofstream::binary);
        if (file.fail()) { return {0, -1}; }

        // Set the position in the file
        file.seekp(write_byte, std::ios::beg);

        // Check if the seek operation was successful
        if (!file) {
            std::cerr << "Failed to set the position in the file." << std::endl;
            return {0, -1};
        }

        char buffer_file_size[MAXLINE];
        // Recv file length (if reconnect, neglect)
        if (!write_byte) {
            int n = recvBuffer(socketFd, buffer_file_size, sizeof(buffer_file_size));
            // printf("file size: %ld\n", reinterpret_cast<int64_t>(&buffer_file_size));
            if (n != sizeof(fileSize_toRecv)) {
                std::string string_buffer(buffer_file_size);
                if (string_buffer.rfind("ack", 0) == 0) {
                    int ack_em_id = std::stoi(string_buffer);
                    return {ack_em_id, -4};
                }
                return {0, -2};
            }
            else fileSize_toRecv = reinterpret_cast<int64_t>(&buffer_file_size);
        }
        std::cout << "[recvFile] Filesize: " << fileSize_toRecv << std::endl;

        char* buffer = new char[chunkSize];
        bool errored = false;
        int64_t i = write_byte;
        while (i < fileSize_toRecv) {
            // std::cout << "Receiving bytes at: " << i << std::endl;
            int r = recvBuffer(socketFd, buffer, (int)std::min(fileSize_toRecv - i, (int64_t)chunkSize));
            if (r == 0) { 
                std::cout << "[recvFile] recvFile nothing at " << i << "/" << fileSize_toRecv << std::endl;
                errored = true; break;
            }
            if ((r < 0) || !file.write(buffer, r)) { 
                // std::cout << "Cannot Recv at: " << i << ", Retrying" << std::endl;
                // sleep(1);
                // continue;
                std::cout << "[recvFile] recvFile error at " << i << "/" << fileSize_toRecv << std::endl;
                errored = true; 
                break;
            }
            i += r;
            std::cout << "Received bytes: " << i << std::endl;
        }
        delete[] buffer;

        file.close();

        return {i, errored ? -3 : 0};
    }

    void send_(int target_em_id, const std::string& message) {
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        recvaddr.sin_addr.s_addr = inet_addr(addresses[target_em_id]
            .first.c_str());
        if (sendto(send_fd, message.c_str(), message.size(), 0, 
               (const struct sockaddr *) &recvaddr, sizeof(recvaddr)) == -1) {
            printf("[DUMMY] sendto error: %d\n", errno);
        }
        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
            "Send packet to proc %d (%s), message: %s", 
            target_em_id, addresses[target_em_id].first.c_str(), message.c_str());
    }

    /*
        mode 0, sending a string 'message' (default)
        mode 1, sending a file, 'message' field is filepath
    */
    int send_tcp(int target_em_id, const std::string& message, const int mode = 0) {
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        if (procs <= target_em_id) return -1;
         std::cout << "[network-helper] send tcp here " <<  std::endl;
        if (inet_pton(AF_INET, addresses[target_em_id].first.c_str(), &(recvaddr.sin_addr)) != 1) {
            // for (int i = 0; i < procs; i++)
            // {
            //     std::cout << "[network-helper] " << addresses[i].first.c_str() << ' ' << addresses[i].second <<  std::endl;
            // }
            std::cerr << "Invalid des IP address: " << addresses[target_em_id].first.c_str() << std::endl;
            return -1;
        }
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        
        std::cout << "[network-helper] Src: " << em_id << ", Des: " << target_em_id << " " <<  inet_ntoa(recvaddr.sin_addr) << ":" << ntohs(recvaddr.sin_port) <<  std::endl;

        // Connect to server
        if (connect(send_fd, (struct sockaddr*)&recvaddr, sizeof(recvaddr)) == -1) {    
            std::cout<< "[network-helper] sender: Fail to connect server socket" << std::endl;
            // sleep(1);
            // return -1;
            // // Trying to re-setup
            close(send_fd);
            setup_send_socket();
            return -1;
            // continue;
        }
        std::cout<< "[network-helper] sender: socket connected" << std::endl;

        switch (mode)
        {
            case 1: {            
                std::pair<int64_t, int64_t> res = sendFile(send_fd, message.c_str());
                while (res.second < 0) {
                    printf("[network-helper] Fail to send file.");
                    switch (res.second)
                    {
                        case -1: printf(" (file couldn't be opened for input)\n"); break;
                        case -2: printf(" (file length couldn't be sent properly)\n"); break;
                        default: printf(" (file couldn't be sent properly)\n"); break;
                    }
                    // res = sendFile(send_fd, message.c_str(), res.first);
                    close(send_fd);
                    setup_send_socket();
                    return -1;
                }
                
                logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
                    "Send file to proc %d (%s), size: %ld", 
                    target_em_id, addresses[target_em_id].first.c_str(), res.first);
                break;
            }
        
        default:
            // Send packet to server
            std::cout << "[network-helper] sending message: " << message.c_str() << std::endl;
            // std::cout << "[network-helper] length :" << message.size() << std::endl;
            // Simple case
            // if (send(send_fd, message.c_str(), message.size(), 0) == -1) {
            //     printf("[DUMMY] sendto error: %d\n", errno);
            //     return -1;
            // }

            if (sendBuffer(send_fd, message.c_str(), message.size()) < 0) {
                char errBuffer[ 256 ];
                char * errorMsg = strerror_r( errno, errBuffer, 256 ); // GNU-specific version, Linux default
                printf("[network-helper] Error (send) %s\n", errorMsg); // return value has to be used since buffer might not be modified
                close(send_fd);
                setup_send_socket();
                return -1;
            }

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
                "Send packet to proc %d (%s), message: %s", 
                target_em_id, addresses[target_em_id].first.c_str(), message.c_str());
            break;
        }

        // close(send_fd);
        return 0;
    }

    /*
        Returns received message or empty string if nothing was received
     */
    message_t receive_() {
        char buffer[MAXLINE];
        struct sockaddr_in sender_addr;
        int sender_id = -1;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);

        ssize_t n = recvfrom(recv_fd, (char *)buffer, MAXLINE, MSG_DONTWAIT, (struct sockaddr*) &sender_addr, &len);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return {-1, ""};
            std::cout << "RECVFROM ERROR" << std::endl;
            exit(1);

        }
        buffer[n] = '\0';

        for (sender_id = 0; sender_id < procs; ++sender_id) {
            if (inet_addr(addresses[sender_id].first.c_str()) == 
                sender_addr.sin_addr.s_addr) 
                break;
        }

        if (sender_id == -1) {
            std::cout << "ERROR - SENDER DOESNT EXIST" << std::endl;
            exit(1);
        }

        logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
            "Recv packet fr proc %d (%s), message: %s",
            sender_id, addresses[sender_id].first.c_str(), buffer);
        return {sender_id, buffer};
    }

    /*
        mode 0, recving buffer and return {sender_id, buffer} (default)
        mode 1, recving a file and return {sender_id, recvfilepath}, use "temp" if recvfilepath not specified
    */
    message_t receive_tcp(const int mode = 0, const std::string& recvfilepath = "temp") {
        // std::cout << "[network-helper] in receive_tcp" << std::endl;
        char buffer[MAXLINE];
        // std::cout << "[network-helper] buffer" << std::endl;
        struct sockaddr_in sender_addr;
        int sender_id = -1;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);
        // std::cout << "[network-helper] sender_sock" << std::endl;

        // Listen for connections
        if (listen(recv_fd, 0) == -1) {
            // std::cout << "[network-helper] Error Listening, Retry!" << std::endl;
            // exit(1);
            return {-1, ""};
        }
        std::cout << "[network-helper] " << em_id << " Listening on " << addresses[em_id].first << ":" << addresses[em_id].second << std::endl;

        // Accept client connection
        int newSocket_fd;
        if ((newSocket_fd = accept(recv_fd, (struct sockaddr*)&sender_addr, &len)) == -1) {
            std::cout << "[network-helper] Error Accepting, Retry!" << std::endl;
            // exit(1);
            close(recv_fd);
            setup_recv_socket();
            return {-1, ""};
            // listen(recv_fd, 0);
        }
        std::cout << "[network-helper] " << em_id << " Socket on server, Accepted" << std::endl;

        switch (mode)
        {
        case 1: {
            std::pair<int64_t, int64_t> res = recvFile(newSocket_fd, recvfilepath);
            int n = res.second;
            while (n < 0) {
                if (n == -4) {
                    printf("[network-helper] ack from %ld\n", res.first);
                    break;
                }
                printf("[network-helper] Fail to receive file.");
                switch (n)
                {
                    case -1: printf(" (file couldn't be opened for output)\n"); break;
                    case -2: printf(" (file length couldn't be received properly)\n"); break;
                    default: printf(" (file couldn't be received properly)\n"); break;
                }
                // res = recvFile(newSocket_fd, recvfilepath, res.first);
                // n = res.second;
                close(recv_fd);
                setup_recv_socket();
                return {-1, ""}; 
            }

            if (n == -4) { 
                sender_id = res.first; 
                strcpy(buffer, std::string("ack").c_str());
                break;
            }

            for (sender_id = 0; sender_id < procs; ++sender_id) {
                if (inet_addr(addresses[sender_id].first.c_str()) == sender_addr.sin_addr.s_addr) 
                    break;
            }

            if (sender_id == -1) {
                std::cout << "ERROR - SENDER DOESNT EXIST" << std::endl;
                exit(1);
            }
            
            strcpy(buffer, recvfilepath.c_str());

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
                "Recv file from proc %d (%s), size: %d, path: %s",
                sender_id, addresses[sender_id].first.c_str(), n, buffer);
            break;
        }
        default:
            // Simple case
            // Receive and print received data
            // ssize_t n = recvfrom(newSocket_fd, (char *)buffer, MAXLINE, MSG_DONTWAIT, (struct sockaddr*) &sender_addr, &len);
            // ssize_t n = recv(newSocket_fd, buffer, sizeof(buffer), 0);
            ssize_t n = recvBuffer(newSocket_fd, buffer, sizeof(buffer)); 

            if (n == -1) {
                std::cerr << "[network-helper] Server: There was a connection issue." << std::endl;
            }
            if (n == 0) {
                std::cout << "[network-helper] Nothing received. Client Disconnected." << std::endl;
            }
            
            if (n < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return {-1, ""};
                std::cout << "[network-helper] RECVFROM ERROR" << std::endl;
                char errBuffer[ 256 ];
                char * errorMsg = strerror_r( errno, errBuffer, 256 ); // GNU-specific version, Linux default
                printf("[network-helper] Error (recv) %s\n", errorMsg); // return value has to be used since buffer might not be modified
                exit(1);

            }
            buffer[n] = '\0';

            for (sender_id = 0; sender_id < procs; ++sender_id) {
                if (inet_addr(addresses[sender_id].first.c_str()) == sender_addr.sin_addr.s_addr) 
                    break;
            }

            if (sender_id == -1) {
                std::cout << "ERROR - SENDER DOESNT EXIST" << std::endl;
                exit(1);
            }

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
                "Recv packet fr proc %d (%s), message: %s",
                sender_id, addresses[sender_id].first.c_str(), buffer);
            break;
        }

        close(newSocket_fd);

        return {sender_id, buffer};
    }

private:
    int64_t fileSize_toRecv;

    void setup_send_socket() {
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket
        if ((send_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, sndbuf = 32768;
        if (setsockopt(send_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(send_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(send_fd, SOL_SOCKET, SO_SNDBUF, &sndbuf, sizeof(sndbuf)) < 0) {
            std::cout << "Failed to set SO_SNDBUF option. " << strerror(errno) << "\n";
            exit(1);
        }
    }
    
    void setup_recv_socket() { 
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket       
        if ((recv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, rcvbuf = 65536;
        if (setsockopt(recv_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEADDR option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(recv_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cout << "Failed to set SO_REUSEPORT option. " << strerror(errno) << "\n";
            exit(1);
        }
        if (setsockopt(recv_fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
            std::cout << "Failed to set SO_RCVBUF option. " << strerror(errno) << "\n";
            exit(1);
        }
            
        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET; 
        servaddr.sin_addr.s_addr = INADDR_ANY;
        servaddr.sin_port = htons(addresses[em_id].second);

        if (bind(recv_fd, (const struct sockaddr *)&servaddr, 
                sizeof(servaddr)) < 0) {
            printf("FAIL %d ", em_id);
            perror("Socket binding failed");
            printf("Value of errno: %d\n", errno); // this will print the error code to stdout
            exit(0);
        }
    }
};
