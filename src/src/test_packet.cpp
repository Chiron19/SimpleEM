#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "network/packet.hpp"
#include "time.cpp"

/* 
Add fowllowing arg of include path to g++ compiler:
    "-I${workspaceFolder}/src/include",
    "-I${workspaceFolder}/src/src",
*/

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
    std::cout << "Please input hex string: " << std::endl;
    std::string hexString;
    std::cin >> hexString;
    if (hexString.length() < 8) {
        hexString = "45000028000040004006e2acac100002ac10000115b3b9dc000000008345188250140000bb550000";
    }
    // "450000537248400040067039ac100002ac100001837a15b3323a775ef06481e9801801f6e08400000101080aa692a3cea692a3ce48656c6c6f776f726c642066726f6d20636c69656e742031372d32312d3536"; correct checksum 0xaa84
    std::cout << hexString << std::endl;
    char buf[256];
    hexStringToCharArray(hexString, buf);
    for (int i = 0; i < 256; ) {
        printf("%4d", (unsigned char)buf[i]);
        if (! ((++i) % 16)) putchar(10); 
    }
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
    std::cout << "ntohs(get_tcp_checksum())" << std::hex << ntohs(packet.get_tcp_checksum()) << std::endl;
    return 0;
}