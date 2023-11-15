#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "network/packet.hpp"
#include "time.cpp"

// Function to convert a hex char to an integer value
unsigned char hexCharToByte(unsigned char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0; // Invalid character
}

// Function to convert a hex string to a char array buffer
void hexStringToCharArray(const std::string& hexString, char* buffer) {
    size_t strSize = hexString.length();
    for (size_t i = 0; i < strSize; i += 2) {
        unsigned char highNibble = hexCharToByte(hexString[i]);
        unsigned char lowNibble = hexCharToByte(hexString[i + 1]);
        *(buffer + i / 2) = static_cast<char>((highNibble << 4) | lowNibble);
    }
}

int main()
{
    std::string hexString = 
    // "45000028000040004006e2acac100002ac10000115b3b9dc000000008345188250140000bb550000";
    "4500003cd123400040061175ac100002ac1000019d9615b403925a6e00000000a002faf0a9590000020405b40402080a4e0cec380000000001030307";
    std::cout << hexString << std::endl;
    char buf[256];
    hexStringToCharArray(hexString, buf);
    for (int i=0; i < 256; ++i) printf("%d ", (unsigned char)buf[i]);
    putchar(10); 
    size_t ssize = hexString.length() / 2;
    printf("length : %ld\n", ssize);
    struct timespec ts = {0, 0};
    /* Public Methods */

    Packet packet(buf, ssize, ts);
    // packet.set_source_addr_tcp("172.16.0.1");
    // packet.set_dest_addr_tcp("172.16.0.2");
    packet.dump();

    // int get_version() const;
    std::cout << "get_version()  " << packet.get_version() << std::endl;
    // size_t get_size() const;
    std::cout << "get_size()  " << packet.get_size() << std::endl;
    // char* get_buffer() const;
    std::cout << "get_buffer()  " << packet.get_buffer() << std::endl;
    // struct timespec get_ts() const;
    // std::cout << packet.get_ts() << std::endl;
    // std::string get_source_addr() const;
    std::cout << "get_source_addr()  " << packet.get_source_addr() << std::endl;
    // std::string get_dest_addr() const;
    std::cout << "get_dest_addr()  " << packet.get_dest_addr() << std::endl;
    // int get_source_port() const;
    std::cout << "get_source_port()  " << packet.get_source_port() << std::endl;
    // int get_dest_port() const;
    std::cout << "get_dest_port()  " << packet.get_dest_port() << std::endl;

    /* Private Methods */
    std::cout << "get_tcp_checksum()   " << std::hex << packet.get_tcp_checksum() << std::endl;
    std::cout << "get_tcp_checksum2()  " << std::hex << packet.get_tcp_checksum2() << std::endl;
    std::cout << "get_tcp_checksum3()  " << std::hex << packet.get_tcp_checksum3() << std::endl;
    return 0;
}