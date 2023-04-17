#pragma once

#include <vector>
#include <algorithm>

#include "network/packet.hpp"

/*
 *
 */
class PacketsBuffer {
public:
    std::vector<Packet> packets;

    void add_packet(const char* buf, size_t size, struct timespec ts) {
        packets.push_back(Packet(buf, size, ts));
    }

    void sort_by_ts() {
        sort(packets.begin(), packets.end());
    }

    struct timespec get_first_ts() {
        return packets.back().ts;
    }

    Packet pop_first_packet() {
        Packet first_packet = std::move(packets.back());
        packets.pop_back();
        return first_packet;
    }

};