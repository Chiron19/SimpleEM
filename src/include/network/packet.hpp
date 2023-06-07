#pragma once

#define _DEFAULT_SOURCE 1

#include <stddef.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <vector>
#include <queue>
#include <algorithm>

#include "utils.hpp"

#define MTU 1500

/** @brief Class encapsulating single ip frame sent through TUN interface
 * 
 * Instances of this class are moved around the emulator system to keep track
 * of which process messaged which process and at what time. As those instances
 * would be put to different priority_queue all types of constructors need to be
 * implemented. Packets would be sorted on the @ref ts parameter (signifying 
 * virtual clock value of the sending process). 
 * 
 * It's worth noting, that in the current form packet code works ONLY for 
 * IPv4 and UDP protocol.
 */
class Packet {

public:

    /** @brief Main packet constructor from data read from TUN FD
     * 
     * Constructors copies data from the @p buf to newly allocated 
     * @ref buffer which is freed in destructor 
     * 
     * @param buf Char buffer read from TUN FD
     * @param size Number of bytes read
     * @param ts Virtual clock value of the sending process
     */
    Packet(const char* buf, size_t size, struct timespec ts);

    /** @brief Copy constructor
     * 
     * @param other Packet to copy from
     */
    Packet(const Packet &other);
    
    /** @brief Move constructor
     * 
     * @param other Packet which insides will be moved here
     */
    Packet(Packet&& other);

    /** @brief Assignment operator
     * 
     * @param other Original packet
     */
    Packet& operator=(const Packet& other);

    /** @brief Comparison operator (comparison based on timestamp)
     * 
     * @param other Packet to compare this on with
     */
    bool operator<(const Packet& other) const;
    ~Packet();

    /** @brief Get packet IPv version (4/6)
     * 
     * Its worth noting that some of packets functionalities don't work for 
     * IPv6. Especially setting source or destination addresses are specifically
     * implemented for IPv4
     * 
     * @return Packet IPv version (4/6)
     */
    int get_version() const;
    size_t get_size() const;
    char* get_buffer() const;
    struct timespec get_ts() const;
    std::string get_source_addr() const;
    std::string get_dest_addr() const;
    int get_source_port() const;
    int get_dest_port() const;

    /** @brief Set a new source address for the packet
     * 
     * Changes packets buffer value so that the source address is updated.
     * This requires the IPv4 checksum to be updated, as well as the
     * UDP checksum in udphdr to be updated.
     * 
     * @param addr New source address (in number/dot form)
     */
    void set_source_addr(const std::string& addr);
    
    /** @brief Set a new destination address for the packet
     * 
     * Changes packets buffer value so that the destination address is updated.
     * This requires the IPv4 checksum to be updated, as well as the
     * UDP checksum in udphdr to be updated.
     * 
     * @param addr New destination address (in number/dot form)
     */
    void set_dest_addr(const std::string& addr);
    
    /** @brief Increase packet's @ref ts value by @p other_ts 
     * 
     * @param other_ts Time by which to increate packet's @ref ts value
     */
    void increase_ts(struct timespec other_ts);

private:

    char* buffer; ///< Buffer in which raw data is stored
    size_t size; ///< Size of data stored in the packet
    struct timespec ts; ///< Packets timestamp

    struct iphdr* get_iphdr() const;
    struct udphdr* get_udp() const;
    char* get_data() const;
    size_t get_data_len() const;

    uint16_t ip4_checksum(const struct iphdr *ip) const;
    uint16_t udp_checksum(const struct iphdr *ip, 
        const struct udphdr* udp, const char* data, size_t data_len) const;
    uint16_t tcp_checksum(const struct iphdr *ip, 
        const struct tcphdr *tcp, const char *data, size_t data_len) const;

    bool has_transport_layer_hdr() const;
};

Packet::Packet(const char* buf, size_t size, struct timespec ts): 
        size(size), ts(ts) {
    buffer = (char*)malloc(size * sizeof(char));
    memcpy(buffer, buf, size);
}

Packet::Packet(const Packet &other): 
        size(other.size), ts(other.ts) {
    buffer = (char*)malloc(size * sizeof(char));
    memcpy(buffer, other.buffer, size);
}

Packet::Packet(Packet&& other): 
        buffer(other.buffer), size(other.size), ts(other.ts) {
    other.buffer = nullptr;
}

Packet& Packet::operator=(const Packet& other) {
    if (this != &other) {
        size = other.size;
        ts = other.ts;
        buffer = (char*)malloc(size * sizeof(char));
        memcpy(buffer, other.buffer, size);
    }
    return *this;
}

Packet::~Packet() {
    free(buffer);
}

bool Packet::operator<(const Packet& other) const {
    return ts > other.ts;
}

int Packet::get_version() const {
    return buffer[0] >> 4;
}

size_t Packet::get_size() const {
    return size;
}

char* Packet::get_buffer() const {
    return buffer;
}

struct timespec Packet::get_ts() const {
    return ts;
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

int Packet::get_source_port() const {
    const struct udphdr *udp = reinterpret_cast<const struct udphdr *>
        (buffer + sizeof(struct iphdr));
    return ntohs(udp->uh_sport);
}

int Packet::get_dest_port() const {
    const struct udphdr *udp = reinterpret_cast<const struct udphdr *>
        (buffer + sizeof(struct iphdr));
    return ntohs(udp->uh_dport);
}

void Packet::set_source_addr(const std::string& addr) {
    struct iphdr *ip = get_iphdr();
    struct udphdr *udp = get_udp();

    ip->saddr = inet_addr(addr.c_str());
    ip->check = htons(ip4_checksum(ip));

    for (size_t i = 0; i < ip->ihl * sizeof(uint32_t); ++i) {
		buffer[i] = *(((uint8_t*) ip) + i);
	}

    if (has_transport_layer_hdr()) {
        udp->check = udp_checksum(ip, udp, get_data(), get_data_len());
        for (size_t i = 0; i < sizeof(*udp); ++i) {
            buffer[ip->ihl * sizeof(uint32_t) + i] = *(((uint8_t*)udp) + i);
        }
    }
}

void Packet::set_dest_addr(const std::string& addr) {
    struct iphdr *ip = get_iphdr();
    struct udphdr *udp = get_udp();

    ip->daddr = inet_addr(addr.c_str());
    ip->check = htons(ip4_checksum(ip));

    for (size_t i = 0; i < ip->ihl * sizeof(uint32_t); ++i) {
		buffer[i] = *(((uint8_t*) ip) + i);
	}

    if (has_transport_layer_hdr()) {
        udp->check = udp_checksum(ip, udp, get_data(), get_data_len());
        for (size_t i = 0; i < sizeof(*udp); ++i) {
            buffer[ip->ihl * sizeof(uint32_t) + i] = *(((uint8_t*)udp) + i);
        }
    }
}

void Packet::increase_ts(struct timespec other_ts) {
    ts = ts + other_ts;
}


/* PRIVATE METHODS */

struct iphdr* Packet::get_iphdr() const {
    return reinterpret_cast<struct iphdr*>(buffer);
}

struct udphdr* Packet::get_udp() const {
    const struct iphdr* ip = get_iphdr();
    return reinterpret_cast<struct udphdr*>(buffer + sizeof(uint32_t) * ip->ihl);
}

char* Packet::get_data() const {
    const struct iphdr* ip = get_iphdr();
    return buffer + sizeof(uint32_t) * ip->ihl + sizeof(struct udphdr); 
}

size_t Packet::get_data_len() const {
    return size - sizeof(uint32_t) * get_iphdr()->ihl - sizeof(struct udphdr);
}

uint16_t Packet::ip4_checksum(const struct iphdr *ip) const {
	const uint16_t *ptr;
	uint32_t sum = 0;
	size_t i, len;

	ptr = (const uint16_t *) ip;
	len = ip->ihl * sizeof (uint32_t);

	assert((len % sizeof (*ptr)) == 0);
	for (i = 0; i < len; i += sizeof (*ptr))
		sum += ntohs(*ptr++);

	sum -= ntohs(ip->check);

	while (sum > 0xffff)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~((uint16_t) sum);
}

uint16_t Packet::udp_checksum(const struct iphdr *ip, 
                              const struct udphdr* udp, 
                              const char* data, size_t data_len) const {
    const uint16_t *ptr;
	uint32_t sum = 0;

    /* Add the pseudo header */
    const uint16_t* ipsaddr = (const uint16_t*)&(ip->saddr);
    sum += *ipsaddr + *(ipsaddr + 1);

    const uint16_t* ipdaddr = (const uint16_t*)&(ip->daddr);
    sum += *ipdaddr + *(ipdaddr + 1);

    sum += htons(IPPROTO_UDP);
    sum += udp->uh_ulen;

    /* Add udphdr */
    sum += udp->uh_sport + udp->uh_dport + udp->uh_ulen;

    /* Add data */
    ptr = (uint16_t*)data;
    while(data_len > 1) {
        sum += (*ptr++);
        data_len -= sizeof(*ptr);
    }
    if (data_len == 1) { // Uneven length of data
        sum += (*(uint8_t*)ptr);
    }   

    /* Carries */
    while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
    }

    return ~((uint16_t)sum);
}

uint16_t Packet::tcp_checksum(const struct iphdr *ip, 
                              const struct tcphdr *tcp,
                              const char *data, size_t data_len) const {
    return 0; // TODO
}

// Can be improved, I assume that if ipheader has non-zero fragment offset, then 
// transport layer header is not here
bool Packet::has_transport_layer_hdr() const {
    struct iphdr *ip = get_iphdr();
    return (ntohs(ip->frag_off) % (1<<13)) == 0;
}
