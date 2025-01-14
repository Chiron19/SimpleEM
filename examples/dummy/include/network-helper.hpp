#pragma once

#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <ctime>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <sys/socket.h>
#include <sys/stat.h>
#include <thread>
#include <chrono>

#include "utils.hpp"
#include "logger.hpp"

#define MAXLINE 10000

/** Type of a message pair <sender \p em_id, message> */
typedef std::pair<int, std::string> message_t;

/** @brief Control for above transport layer operations
 * 
 * Class responsible for sending and receiving messages / files between processes upon the transport layer, combining with the basic functions of sending and receiving UDP/TCP packets.
*/
class NetworkHelper {
public:
    int em_id, procs;
    int send_fd, recv_fd;
    std::vector<std::pair<std::string, int>> addresses;
    
    /**
     * @brief Construct a new Network Helper object
     * 
     * The constructor reads the configuration file and stores the pairs of IP address and port into vector \p addresses. And then it sets up the sockets for sending and receiving.
     * 
     * @param em_id The \p em_id of the process.
     * @param config_path The path of the configuration file.
     */
    NetworkHelper(int em_id, const std::string& config_path): em_id(em_id) {  
        // General Setup: read config
        std::ifstream config(config_path);
        std::string address;
        int port;
        while(config >> address >> port) {
            addresses.push_back(std::make_pair(address, port));
        }
        procs = addresses.size();

        // For em_id: setup socket
        setup_recv_socket();
        setup_send_socket();
    }

    /**
     * @brief Dump the buffer on printing output in formats
     * 
     * This function takes a buffer and its length as input and prints the buffer in hex format.
     * 
     * @param buf The buffer to be dumped.
     * @param len The length of the buffer.
    */
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

    /**
     * @brief Get the local IP address
     * 
     * @return The local IP address in string format. (e.g. "192.168.0.1")
    */
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

    /**
     * @brief Get the local time
     * 
     * @return The local time in preset string format "%H-%M-%S". (e.g. "12-34-56")
    */
    std::string getLocalTime() {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H-%M-%S");

        return oss.str();
    }

    /**
     * @brief Get the size of a file
     * 
     * This function takes a file path as input and returns the size of the file in bytes.
     * 
     * @param filePath The path of the file.
     * @return The size of the file in bytes.
     */
    int64_t getFileSize(const std::string& filePath) {
        try {
            return std::filesystem::file_size(filePath);
        } catch (std::filesystem::filesystem_error& e) {
            throw std::runtime_error("Error getting file size: " + std::string(e.what()));
        }
    }

    /**
     * @brief Receives data from a socket into a buffer
     * 
     * @param socketFd The file descriptor of the socket.
     * @param buffer The buffer to store the received data.
     * @param bufferSize The size of the buffer.
     * @return The number of bytes received, or -1 if an error occurred.
     */
    int recvBuffer(int socketFd, void* buffer, int bufferSize) {
        if (buffer == nullptr || bufferSize <= 0) {
            std::cerr << "(recvBuffer) Invalid buffer or buffer size." << std::endl;
            return -1;
        }

        const int l = recv(socketFd, buffer, bufferSize, 0);
        if (l < 0) { 
            std::cerr << "Error (recvBuffer): " << strerror(errno) << std::endl;      
        } // this is an error
        return l; // this is the number of bytes received
    }

    /**
     * @brief Sends a buffer of data over a socket
     *
     * @param socketFd The file descriptor of the socket.
     * @param buffer The buffer containing the data to be sent.
     * @param bufferSize The size of the buffer.
     * @return The number of bytes sent, or -1 if an error occurred.
     */
    int sendBuffer(int socketFd, const void* buffer, int bufferSize) {
        const int l = send(socketFd, buffer, bufferSize, 0);
        if (l < 0) {
            std::cerr << "Error (sendBuffer): " << strerror(errno) << std::endl;
        } // this is an error
        return l;  // this is the number of bytes sent
    }

    /**
     * @brief Sends a file over a socket
     *
     * @param socketFd The file descriptor of the sender socket.
     * @param filePath The path of the file to send.
     * @param read_byte The starting position to read from the file. (default: 0)
     * @param chunkSize The size of each chunk to send. (default: 32768)
     * @return The number of bytes sent;
     * or -1 if an error occurred before sending any data,
     * or -3 if the file couldn't be sent properly.
     */
    int sendFile(int socketFd, const std::string& filePath, const std::streampos read_byte = 0, const std::streamsize chunkSize = 32768) {
        const std::streamsize fileSize = getFileSize(filePath);
        std::cout << "[sendFile] File: " << filePath << ", Filesize: " << fileSize << std::endl;
        if (fileSize == 0) { 
            std::cout << "[sendFile] File empty" << std::endl;
            return 0; 
        }
        if (fileSize < 0) { 
            std::cout << "[sendFile] File not exist" << std::endl;
            return 0; 
        }

        std::ifstream file(filePath, std::ifstream::binary);
        if (file.fail()) { 
            std::cout << "[sendFile] File failed" << std::endl;
            return -1; 
        }

        // Set the position in the file
        file.seekg(read_byte, std::ios::beg);

        // Check if the seek operation was successful
        if (!file) {
            std::cerr << "[sendFile] Failed to open file." << std::endl;
            return -1;
        }
            
        std::vector<char> buffer(chunkSize);
        std::streamsize totalSent = read_byte;

        while (totalSent < fileSize) {
            std::streamsize toRead = std::min(chunkSize, fileSize - totalSent);
            file.read(buffer.data(), toRead);
            if (!file) {
                std::cerr << "Failed to read from file." << std::endl;
                return -3;
            }

            std::streamsize sent = sendBuffer(socketFd, buffer.data(), toRead);
            if (sent == -1) {
                std::cerr << "Failed to send data." << std::endl;
                return -3;
            }

            totalSent += sent;
            std::cout << "Sent bytes: " << totalSent << std::endl;

            // if not doing congestion control, the recv socket will be blocked and full file could not be sent successfully
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        file.close();

        return totalSent;
    }

   /**
    * @brief Receives a file
    * 
    * @param socketFd The file descriptor of the receiver socket.
    * @param filePath The path of the file to receive.
    * @param write_byte The starting position to write to the file. (default: 0)
    * @param chunkSize The size of each chunk to receive. (default: 65536)
    * @return The number of bytes received;
    * or -1 if file couldn't be opened for output,
    * or -3 if couldn't receive file properly.
   */
    int recvFile(int socketFd, const std::string& filePath, const std::streampos write_byte = 0, const int chunkSize = 65536) {
        // Open file for output
        std::ofstream file(filePath, std::ofstream::binary);
        if (file.fail()) { 
            std::cout << "[recvFile] File failed" << std::endl;
            return -1; 
        }

        // Set the position in the file
        file.seekp(write_byte, std::ios::beg);

        // Check if the seek operation was successful
        if (!file) {
            std::cerr << "Failed to set the position in the file." << std::endl;
            return -1;
        }

        std::vector<char> buffer(chunkSize);
        bool errored = false;
        std::streamsize totalRecv = write_byte;

        while (totalRecv < fileSize_toRecv) {
            int r = recvBuffer(socketFd, buffer.data(), chunkSize);
            if (r <= 0) { 
                std::cout << "[recvFile] recvFile error at " << totalRecv << "/" << fileSize_toRecv << std::endl;
                errored = true; 
                break;
            }

            try {
                file.write(buffer.data(), r);
            } catch (std::ofstream::failure e) {
                std::cout << "[recvFile] recvFile write error at " << totalRecv << "/" << fileSize_toRecv << std::endl;
                errored = true;
                break;
            }

            totalRecv += r;
            std::cout << "Received bytes: " << totalRecv << std::endl;
        }
        file.close();

        if (errored) {
            std::cout << "Failed to receive the entire file." << std::endl;
            return -3;
        }

        const int64_t fileSize = getFileSize(filePath);
        std::cout << "Received File: " << filePath << ", Filesize: " << fileSize << std::endl;

        return totalRecv;
    }

    /**
     * @brief Send a message to a process (UDP, original)
     * 
     * @param target_em_id The \p em_id of the target process.
     * @param message The message to be sent.
    */
    void send_udp(int target_em_id, const std::string& message) {
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
    
    /**
     * @brief Receive a message from a process (UDP, original)
     * 
     * @return The received message or empty string if nothing was received.
     */
    message_t receive_udp() {
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

   /**
    * @brief Send a message to a process
    * 
    * @param target_em_id The \p em_id of the target process.
    * @param message The message to be sent.
    * @param mode 
    * 0, sending a string \p message (default).\n
    * 1, sending a file, \p message field is filepath.
    * @return 0 if success or -1 if fail.
   */
    int send_tcp(int target_em_id, const std::string& message, const int mode = 0) {
        // Setup socket for sending
        struct sockaddr_in recvaddr;
        memset(&recvaddr, 0, sizeof(recvaddr));
        if (procs <= target_em_id) return -1;
        // std::cout << "[network-helper] send tcp here " <<  std::endl;
        if (inet_pton(AF_INET, addresses[target_em_id].first.c_str(), &(recvaddr.sin_addr)) != 1) {
            std::cerr << "Invalid des IP address: " << addresses[target_em_id].first.c_str() << std::endl;
            return -1;
        }
        recvaddr.sin_family = AF_INET;
        recvaddr.sin_port = htons(addresses[target_em_id].second);
        
        std::cout << "[network-helper] Src: " << em_id << ", Des: " << target_em_id << " " <<  inet_ntoa(recvaddr.sin_addr) << ":" << ntohs(recvaddr.sin_port) <<  std::endl;

        // Connect to receiver
        if (connect(send_fd, (struct sockaddr*)&recvaddr, sizeof(recvaddr)) == -1) {    
            std::cout<< "[network-helper] sender: Fail to connect server socket" << std::endl;
            close(send_fd);
            setup_send_socket();
            return -1;
        }
        std::cout<< "[network-helper] sender: socket connected" << std::endl;

        switch (mode) {
            case 1: {  
                // Send file length to receiver
                const std::streamsize fileSize_toSend = getFileSize(message);
                if (fileSize_toSend == 0) { 
                    std::cout << "[sendFile] File empty" << std::endl;
                    return 0; 
                }

                sendBuffer(send_fd, reinterpret_cast<const void*>(&fileSize_toSend), sizeof(std::streamsize));
                std::cout << "[sendFile] Filesize: " << fileSize_toSend << std::endl;             
                
                // Send file to receiver
                int n = 0, sendsize = 0;
                do {
                    n = sendFile(send_fd, message.c_str(), sendsize);

                    if (n <= 0) {
                        std::cout << "[network-helper] Error sent: ";
                        switch (n) {
                            case -1: printf(" (file couldn't be opened for input)\n"); return -1; break;
                            case -2: printf(" (file length couldn't be sent properly)\n"); break;
                            default: printf(" (file couldn't be sent properly)\n"); break;
                        }
                        close(send_fd);
                        setup_send_socket();
                        return -1;
                    }
                    else {
                        sendsize = n;
                        std::cout << "[network-helper] sent: " << n << "/" << fileSize_toSend << std::endl;
                    }
                } while (sendsize != fileSize_toSend);
                
                logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
                    "Send file to proc %d (%s), size: %d", 
                    target_em_id, addresses[target_em_id].first.c_str(), sendsize);
                sleep(1);
                break;
            }
        
        default:
            // Send packet to receiver
            std::cout << "[network-helper] sending message: " << message.c_str() << std::endl;
            // std::cout << "[network-helper] length :" << message.size() << std::endl;
            // Simple case
            // if (send(send_fd, message.c_str(), message.size(), 0) == -1) {
            //     printf("[DUMMY] sendto error: %d\n", errno);
            //     return -1;
            // }

            if (sendBuffer(send_fd, message.c_str(), message.size()) < 0) {
                close(send_fd);
                setup_send_socket();
                return -1;
            }

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID, 
                "Send packet to proc %d (%s), message: %s", 
                target_em_id, addresses[target_em_id].first.c_str(), message.c_str());
            break;
        }

        printf("[network-helper] send tcp done \n");
        return 0;
    }

   /**
    * @brief Receive a message from a process
    * 
    * @param recvfilepath The path of the file to receive, used in mode 1. (default: "temp")
    * @param mode 
    * 0, receiving buffer and return { \p sender_id, \p buffer }. (default)\n 
    * 1, receiving a file and return { \p sender_id, \p recvfilepath }.
    * @return corresponding \p message_t if success, or {-1, ""} if fail.
    */
    message_t receive_tcp(const std::string& recvfilepath = "temp", const int mode = 0) {
        // Setup socket for receiving
        char buffer[MAXLINE];
        struct sockaddr_in sender_addr;
        int sender_id = -1;
        memset(&sender_addr, 0, sizeof(sender_addr));
        socklen_t len = sizeof(sender_addr);

        // Listen for connections
        if (listen(recv_fd, 0) == -1) {
            std::cout << "[network-helper] Error Listening, Retry!" << std::endl;
            return {-1, ""};
        }
        std::cout << "[network-helper] " << em_id << " Listening on " << addresses[em_id].first << ":" << addresses[em_id].second << std::endl;

        // Accept client connection
        int newSocket_fd;
        if ((newSocket_fd = accept(recv_fd, (struct sockaddr*)&sender_addr, &len)) == -1) {
            std::cout << "[network-helper] Error Accepting, Retry!" << std::endl;
            close(recv_fd);
            setup_recv_socket();
            return {-1, ""};
        }
        std::cout << "[network-helper] " << em_id << " Socket on server, Accepted" << std::endl;

        switch (mode) {
        case 1: {
            // Receive file length from sender
            recvBuffer(newSocket_fd, reinterpret_cast<void *>(&fileSize_toRecv), sizeof(std::streamsize));
            std::cout << "[recvFile] Filesize: " << fileSize_toRecv << std::endl;
        
            // Receive file from sender
            int n = 0, recvsize = 0;
            do {
                n = recvFile(newSocket_fd, recvfilepath, recvsize);

                if (n < 0) {
                    printf("[network-helper] Error received:");
                    switch (n) {
                        case -1: printf(" (file couldn't be opened for output)\n"); break;
                        case -2: printf(" (file length couldn't be received properly)\n"); break;
                        default: printf(" (file couldn't be received properly)\n"); break;
                    }
                    close(newSocket_fd);
                    close(recv_fd);
                    setup_recv_socket();
                    return {-1, ""};
                }
                else {
                    recvsize = n;
                    std::cout << "[network-helper] recv: " << n << "/" << fileSize_toRecv << std::endl;
                }
            } while (recvsize != fileSize_toRecv);

            // Get sender_id from address vector (by IP address)
            // If two processes are on the same IP, this will not work!
            for (sender_id = 0; sender_id < procs; ++sender_id) {
                if (inet_addr(addresses[sender_id].first.c_str()) == sender_addr.sin_addr.s_addr) break;
            }
            if (sender_id == -1) {
                std::cout << "[recvFile] sender not exist" << std::endl;
            }
            
            strcpy(buffer, recvfilepath.c_str());

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
                "Recv file from proc %d (%s), size: %d, path: %s",
                sender_id, addresses[sender_id].first.c_str(), n, buffer);
            break;
        }
        default:
            // Receive packet from sender
            // Simple case
            // Receive and print received data
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
                char errBuffer[256];
                char * errorMsg = strerror_r(errno, errBuffer, 256); // GNU-specific version, Linux default
                printf("[network-helper] Error (recv) %s\n", errorMsg); // return value has to be used since buffer might not be modified
                exit(1);

            }
            buffer[n] = '\0';

            for (sender_id = 0; sender_id < procs; ++sender_id) {
                if (inet_addr(addresses[sender_id].first.c_str()) == sender_addr.sin_addr.s_addr) break;
            }
            if (sender_id == -1) {
                std::cout << "[recvFile] sender not exist" << std::endl;
            }

            logger_ptr->log_event(CLOCK_PROCESS_CPUTIME_ID,
                "Recv packet fr proc %d (%s), message: %s",
                sender_id, addresses[sender_id].first.c_str(), buffer);
            break;
        }

        close(newSocket_fd);
        printf("[network-helper] receive tcp done \n");
        return {sender_id, buffer};
    }

private:
    std::streamsize fileSize_toRecv;

    /**
     * @brief Setup the sender socket
    */
    void setup_send_socket() {
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket
        if ((send_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, sndbuf = 1 << 30;
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
        struct linger so_linger;
        so_linger.l_onoff = 1;
        so_linger.l_linger = 0;
        setsockopt(send_fd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
    }
    
    /**
     * @brief Setup the receiver socket
    */
    void setup_recv_socket() { 
        // SOCK_STREAM is for TCP socket
        // SOCK_DGRAM  is for UDP socket       
        if ((recv_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("Socket creation failed...\n");
            exit(0);
        }

        int opt = 1, rcvbuf = 1 << 30;
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
            exit(0);
        }
    }
};
