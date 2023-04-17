#ifndef SIMPLEEM_PACKET
#define SIMPLEEM_PACKET

#include <stddef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#include <vector>
#include <queue>
#include <algorithm>

#include "utils.hpp"

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

    bool operator<(const Packet& other) const {
        return ts > other.ts;
    }

    ~Packet() {
        free(buffer);
    }
};

#endif // SIMPLEEM_PACKET