#ifndef SIMPLEEM_BUFFERS
#define SIMPLEEM_BUFFERS

#include <stddef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <queue>

#include "utils.hpp"

#define MTU 1500

typedef struct packet_desc_t {
    struct timespec ts;
    char* data;
    size_t size;
} packet_desc_t;

class Packet {
public:
    char* buffer;
    size_t size;
    struct timespec ts;

    Packet(const char* buf, size_t size, struct timespec ts): size(size), ts(ts) {
        buffer = (char*)malloc(size * sizeof(char));
        memcpy(buffer, buf, size);
    }

    bool operator<(const Packet& other) const {
        return nano_from_ts(ts) > nano_from_ts(other.ts);
    }

    ~Packet() {
        free(buffer);
    }
};

/*
 *
 */
class PacketsBuffer {
public:
    std::priority_queue<Packet> packets;

    void add_packet(const char* buf, size_t size, struct timespec ts) {
        packets.push(Packet(buf, size, ts));
    }

    struct timespec get_first_ts() {
        return packets.top().ts;
    }

    Packet pop_first_packet() {
        Packet first_packet = std::move(packets.top());
        packets.pop();
        return first_packet;
    }

};

#endif // SIMPLEEM_BUFFERS