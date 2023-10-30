#pragma once

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <fstream>
#include <errno.h>

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

std::string getLocalTime() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%H-%M-%S");

    return oss.str();
}

// 
// Reference: 
// https://stackoverflow.com/questions/63494014/sending-files-over-tcp-sockets-c-windows
// 
int64_t GetFileSize(const std::string& fileName) {
    // no idea how to get filesizes > 2.1 GB in a C++ kind-of way.
    // I will cheat and use Microsoft's C-style file API
    FILE* file;
    // https://www.mkssoftware.com/docs/man3/fopen.3.asp
    if ((file = fopen64(fileName.c_str(), "rb")) == NULL) {
        return -1;
    }
    fseeko64(file, 0, SEEK_END);
    const int64_t len = ftello64(file);
    fclose(file);
    return len;
}

//
// Recieves data in to buffer until bufferSize value is met
//
int RecvBuffer(int socketFd, char* buffer, int bufferSize, int chunkSize = 16 * 1024) {
    int i = 0;
    while (i < bufferSize) {
        // std::cout << "receiving buffer at " << i << ", length: " << std::min(chunkSize, bufferSize - i) << std::endl;
        const int l = recv(socketFd, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
        // printf("received %d\n", i);
        if (l < 0) { 
            char errBuffer[ 256 ];
            char * errorMsg = strerror_r( errno, errBuffer, 256 ); // GNU-specific version, Linux default
            printf("Error (recv) %s\n", errorMsg); //return value has to be used since buffer might not be modified
    
            std::cout << "recv() error at: " << i << std::endl;
            return i;
        } // this is an error
        if (l == 0) {
            std::cout << "recv() 0 at: " << i << std::endl;
            return i;
        } // Receive = 0
        i += l;
    }
    return i;
}

//
// Sends data in buffer until bufferSize value is met
// return size being sent
//
int SendBuffer(int socketFd, const char* buffer, int bufferSize, int chunkSize = 64 * 1024) {
    int i = 0;
    while (i < bufferSize) {
        // printf("sending buffer at %d\n", i);
        const int l = send(socketFd, &buffer[i], std::min(chunkSize, bufferSize - i), 0);
        // printf("sent %d\n", l);
        if (l < 0) {
            char errBuffer[ 256 ];
            char * errorMsg = strerror_r( errno, errBuffer, 256 ); // GNU-specific version, Linux default
            printf("Error (send) %s\n", errorMsg); //return value has to be used since buffer might not be modified
            std::cout << "send() error at: " << i << std::endl;
            // dump(&buffer[i], std::min(chunkSize, bufferSize - i));
            // puts(std::string("[EOF]").c_str());
            // break;
            // continue;
            return l; 
        } // this is an error
        i += l;
    }
    return i;
}

//
// Sends a file
// returns size of file if success
// returns -1 if file couldn't be opened for input
// returns -2 if couldn't send file length properly
// returns -3 if file couldn't be sent properly
//
int64_t SendFile(int socketFd, const std::string& fileName, int chunkSize = 32 * 1024) {

    const int64_t fileSize = GetFileSize(fileName);
    std::cout << "File: " << fileName << ", Filesize: " << fileSize << std::endl;
    if (fileSize < 0) { return -1; }

    std::ifstream file(fileName, std::ifstream::binary);
    if (file.fail()) { return -1; }

    if (SendBuffer(socketFd, reinterpret_cast<const char*>(&fileSize),
        sizeof(fileSize)) != sizeof(fileSize)) {
        return -2;
    }

    char* buffer = new char[chunkSize];
    bool errored = false;
    int64_t i = 0;
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
            std::cout << "Cannot Read at: " << i << ", Retrying" << std::endl;
            sleep(1);
            continue;
            // errored = true; 
            // std::cout << "Cannot Read at: " << i << std::endl;
            // break; 
        }
        int l = SendBuffer(socketFd, buffer, (int)ssize);
        // std::cout << "Sent bytes: " << i << std::endl;
        while (l < 0) {
            std::cout << "socket status: " << isfdtype(socketFd, S_IFSOCK) << std::endl;
            l = SendBuffer(socketFd, buffer, (int)ssize);
            std::cout << "Sent bytes: " << l << std::endl;
            // errored = true; break; 
        }
        i += l;
        std::cout << "Sent bytes: " << i << std::endl;
    }
    delete[] buffer;

    file.close();

    return errored ? -3 : fileSize;
}

//
// Receives a file
// returns size of file if success
// returns -1 if file couldn't be opened for output
// returns -2 if couldn't receive file length properly
// returns -3 if couldn't receive file properly
//
int64_t RecvFile(int socketFd, const std::string& fileName, int chunkSize = 64 * 1024) {
    std::ofstream file(fileName, std::ofstream::binary);
    if (file.fail()) { return -1; }

    int64_t fileSize;
    if (RecvBuffer(socketFd, reinterpret_cast<char*>(&fileSize),
            sizeof(fileSize)) != sizeof(fileSize)) {
        return -2;
    }
    std::cout << "[Receiver] Filesize: " << fileSize << std::endl;

    char* buffer = new char[chunkSize];
    bool errored = false;
    int64_t i = 0;
    while (i < fileSize) {
        // std::cout << "Receiving bytes at: " << i << std::endl;
        int r = RecvBuffer(socketFd, buffer, (int)std::min(fileSize - i, (int64_t)chunkSize));
        if (r == 0) { errored = true; break; }
        if ((r < 0) || !file.write(buffer, r)) { 
            std::cout << "Cannot Recv at: " << i << ", Retrying" << std::endl;
            sleep(1);
            continue;
            // errored = true; 
            // break; 
        }
        i += r;
        std::cout << "Received bytes: " << i << std::endl;
    }
    delete[] buffer;

    file.close();

    return errored ? -3 : fileSize;
}