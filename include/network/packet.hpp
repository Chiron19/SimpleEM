#ifndef SIMPLEEM_PACKET
#define SIMPLEEM_PACKET

#define _DEFAULT_SOURCE 1

#include <netinet/in.h>
#include <stddef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <queue>
#include <algorithm>

#include "utils.hpp"
#include "trans_frame.hpp"

#define MTU 1500

class Packet {
public:
    char* buffer;
    size_t size;
    struct timespec ts;

    Packet(const char* buf, size_t size, struct timespec ts): size(size), ts(ts) {
        buffer = (char*)malloc(size * sizeof(char));
        memcpy(buffer, buf, size);
    }

    Packet(const Packet &other): size(other.size), ts(other.ts) {
        buffer = (char*)malloc(size * sizeof(char));
        memcpy(buffer, other.buffer, size);
    }

    Packet(Packet&& other): buffer(other.buffer), size(other.size), ts(other.ts) {
        other.buffer = nullptr;
    }

    Packet& operator=(const Packet& other) {
        if (this != &other) {
            size = other.size;
            ts = other.ts;
            buffer = (char*)malloc(size * sizeof(char));
            memcpy(buffer, other.buffer, size);
        }
        return *this;
    }

    bool operator<(const Packet& other) const {
        return ts > other.ts;
    }

    ~Packet() {
        free(buffer);
    }

    int get_version() const;

    std::string get_source_addr() const;

    std::string get_dest_addr() const;

    void swap_source_dest_addr();

    void send(int fd) const;
};

int Packet::get_version() const {
    return buffer[0] >> 4;
}

std::string Packet::get_source_addr() const {
    const struct iphdr *ip = reinterpret_cast<const struct iphdr *>(buffer);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip->saddr, buf, sizeof(buf));
    return std::string(buf);
}

std::string Packet::get_dest_addr() const {
    const struct iphdr *ip = reinterpret_cast<const struct iphdr *>(buffer);
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &ip->daddr, buf, sizeof(buf));
    return std::string(buf);
}

void Packet::swap_source_dest_addr() {
    struct iphdr *ip = reinterpret_cast<struct iphdr*>(buffer);

    int helpy = ip->daddr;
	ip->daddr = ip->saddr;
	ip->saddr = helpy;
	ip->check = htons(ip4_checksum(ip));

    for (size_t i = 0; i < sizeof(struct iphdr); ++i) {
		buffer[i] = *(((uint8_t*) ip) + i);
	}
}

void Packet::send(int fd) const {
    ssize_t ssize;
    size_t to_send = size, offset = 0;

	while (to_send > 0) {
		ssize = write(fd, buffer, to_send);
		if (ssize < 0)
			panic("write");

		offset += (size_t) ssize;
		to_send -= (size_t) ssize;
	}
}

#endif // SIMPLEEM_PACKET