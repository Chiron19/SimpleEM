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

    /** @brief Get packet size (private variable)
     * 
     * @return Packet size
     */
    size_t get_size() const;

    /** @brief Get packet buffer (private variable)
     * 
     * @return Packet buffer
     */
    char* get_buffer() const;

    /** @brief Get packet timestamp (private variable)
     * 
     * @return Packet timestamp
     */
    struct timespec get_ts() const;

    /** @brief Get packet source address (from IPv4 header)
     * 
     * @return Packet source address (in number/dot form)
     */
    std::string get_source_addr() const;

    /** @brief Get packet destination address (from IPv4 header)
     * 
     * @return Packet destination address (in number/dot form)
     */
    std::string get_dest_addr() const;

    /** @brief Get packet source port (from UDP header)
     * 
     * @return Packet source port
     */
    int get_source_port() const;

    /** @brief Get packet destination port (from UDP header)
     * 
     * @return Packet destination port
     */
    int get_dest_port() const;

    /** @brief Get packet source port (from TCP header)
     * 
     * @return Packet source port
     */
    int get_source_port_tcp() const;

    /** @brief Get packet destination port (from TCP header)
     * 
     * @return Packet destination port
     */
    int get_dest_port_tcp() const;

    /** @brief Get packet TCP checksum (from TCP header)
     * 
     * @return Packet TCP checksum
     */
    int get_tcp_checksum() const;

    /** @brief Dump packet contents in hex format to stdout
     */
    void dump();
    
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

    /** @brief Set a new source address for the packet in TCP
     * @param addr New source address (in number/dot form)
     */
    void set_source_addr_tcp(const std::string& addr);

    /** @brief Set a new destination address for the packet in TCP
     * @param addr New destination address (in number/dot form)
     */
    void set_dest_addr_tcp(const std::string& addr);
    
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
    struct tcphdr* get_tcp() const;
    char* get_data() const;
    size_t get_data_len() const;
    char* get_data_tcp() const;
    size_t get_data_len_tcp() const;

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

void Packet::dump()
{
    const char * buf = get_buffer();
    size_t len = get_size();
	size_t i, j;

	for (i = 0; i < len; i++) {
		if ((i % 8) == 0)
			printf("%04hx  ", (uint16_t) i);

		printf("%02hhx", buf[i]);
		if ((i % 8) == 3) {
			printf("  ");
		} else if ((i % 8) == 7) {
			printf("  ");
			for (j = i - 7; j <= i; j++)
				if ((buf[j] < 32) || (buf[j] > 126))
					printf(".");
				else
					printf("%c", buf[j]);
			printf("\n");
		} else {
			printf(" ");
		}
	}

	if ((i % 8) != 0) {
		for (j = i % 8; j < 8; j++) {
			printf("  ");
			if (j == 3)
				printf("  ");
			else
				printf(" ");
		}
		printf(" ");
		for (j = i - (i % 8); j < i; j++)
			if ((buf[j] < 32) || (buf[j] > 126))
				printf(".");
			else
				printf("%c", buf[j]);
		printf("\n");
	}
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

int Packet::get_source_port_tcp() const {
    const struct tcphdr *tcp = reinterpret_cast<const struct tcphdr *>
        (buffer + sizeof(struct iphdr));
    return ntohs(tcp->th_sport);
}

int Packet::get_dest_port_tcp() const {
    // std::cout << "SET TCP SRC ADDR here" << std::endl;
    const struct tcphdr *tcp = reinterpret_cast<const struct tcphdr *>
        (buffer + sizeof(struct iphdr));
    return ntohs(tcp->th_dport);
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

void Packet::set_source_addr_tcp(const std::string& addr) {
    struct iphdr *ip = get_iphdr();
    // std::cout << "ip hdr get here" << std::endl;
    struct tcphdr *tcp = get_tcp();

    // std::cout << "SET TCP SRC ADDR here" << std::endl;
    ip->saddr = inet_addr(addr.c_str());
    // ip->protocol = IPPROTO_TCP;
    ip->check = htons(ip4_checksum(ip));

    for (size_t i = 0; i < ip->ihl * sizeof(uint32_t); ++i) {
		buffer[i] = *(((uint8_t*) ip) + i);
	}

    if (has_transport_layer_hdr()) {
        // tcp->check = ntohs(tcp_checksum(ip, tcp, get_data_tcp(), get_data_len_tcp()));
        tcp->check = ntohs(get_tcp_checksum());
        // std::cout << "Checksum set: " << std::hex << tcp->check << std::endl;
        for (size_t i = 0; i < sizeof(*tcp); ++i) {
            buffer[ip->ihl * sizeof(uint32_t) + i] = *(((uint8_t*) tcp) + i);
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

void Packet::set_dest_addr_tcp(const std::string& addr) {
    struct iphdr *ip = get_iphdr();
    // std::cout << "ip hdr get here" << std::endl;
    struct tcphdr *tcp = get_tcp();
    // std::cout << "tcp hdr get here" << std::endl;

    ip->daddr = inet_addr(addr.c_str());
    // ip->protocol = IPPROTO_TCP;
    ip->check = htons(ip4_checksum(ip));

    // std::cout << "checksum set" << std::endl;

    for (size_t i = 0; i < ip->ihl * sizeof(uint32_t); ++i) {
		buffer[i] = *(((uint8_t*) ip) + i);
	}

    // std::cout << "buffer load" << std::endl;

    if (has_transport_layer_hdr()) {
        // std::cout << "has transport layer hdr" << std::endl;

        // std::cout << "data_len " << get_data_len_tcp() << std::endl;
        // std::cout << "tcp_checksum" << std::hex << tcp_checksum(ip, tcp, get_data_tcp(), get_data_len_tcp()) << std::endl;
        // tcp->check = ntohs(tcp_checksum(ip, tcp, get_data_tcp(), get_data_len_tcp()));
        tcp->check = ntohs(get_tcp_checksum());
        // std::cout << "Checksum set: " << std::hex << tcp->check << std::endl;
        for (size_t i = 0; i < sizeof(*tcp); ++i) {
            buffer[ip->ihl * sizeof(uint32_t) + i] = *(((uint8_t*) tcp) + i);
        }
    }
    
}

void Packet::increase_ts(struct timespec other_ts) {
    ts = ts + other_ts;
}

int Packet::get_tcp_checksum() const {
    return tcp_checksum(get_iphdr(), get_tcp(), get_data_tcp(), get_data_len_tcp());
}

/* PRIVATE METHODS */

struct iphdr* Packet::get_iphdr() const {
    return reinterpret_cast<struct iphdr*>(buffer);
}

struct udphdr* Packet::get_udp() const {
    const struct iphdr* ip = get_iphdr();
    return reinterpret_cast<struct udphdr*>(buffer + sizeof(uint32_t) * ip->ihl);
}

struct tcphdr* Packet::get_tcp() const {
    const struct iphdr* ip = get_iphdr();
    return reinterpret_cast<struct tcphdr*>(buffer + sizeof(uint32_t) * ip->ihl);
}

char* Packet::get_data() const {
    const struct iphdr* ip = get_iphdr();
    return buffer + sizeof(uint32_t) * ip->ihl + sizeof(struct udphdr); 
}

size_t Packet::get_data_len() const {
    return size - sizeof(uint32_t) * get_iphdr()->ihl - sizeof(struct udphdr);
}

char* Packet::get_data_tcp() const {
    const struct iphdr* ip = get_iphdr();
    return buffer + sizeof(uint32_t) * ip->ihl + sizeof(struct tcphdr); 
}

size_t Packet::get_data_len_tcp() const {
    // std::cout << "size: " << size << std::endl;
    // std::cout << "ip_hdr: " << sizeof(uint32_t) * get_iphdr()->ihl << std::endl;
    // std::cout << "tcp_hdr: " << sizeof(struct tcphdr) << std::endl;
    return size - sizeof(uint32_t) * get_iphdr()->ihl - sizeof(struct tcphdr);
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

// uint16_t Packet::tcp_checksum(const struct iphdr *ip, const struct tcphdr *tcp, const char *data, size_t data_len) const
// {
// 	const uint16_t *ptr = reinterpret_cast<uint16_t *>(buffer + sizeof(uint32_t) * ip->ihl);
//     data_len = size - sizeof(uint32_t) * ip->ihl;
// 	uint32_t sum = 0;
// 	size_t i;

// 	sum += ip->protocol;
// 	sum += ntohs((uint16_t) (ip->saddr & 0xffff));
// 	sum += ntohs((uint16_t) (ip->saddr >> 16));
// 	sum += ntohs((uint16_t) (ip->daddr & 0xffff));
// 	sum += ntohs((uint16_t) (ip->daddr >> 16));
// 	sum += ntohs(ip->tot_len) - (ip->ihl * sizeof (uint32_t));

//     // std::cout << "psudoheader:" << std::hex << sum << std::endl;

// 	for (i = 0; (i + sizeof (*ptr)) <= data_len; i += sizeof (*ptr))
// 		sum += ntohs(*ptr++);

// 	if (i < data_len)
// 		sum += ((uint16_t) data[i]) << 8;

// 	sum -= ntohs(tcp->th_sum);

//     // std::cout << "All:" << std::hex << sum << std::endl;

// 	while (sum > 0xffff)
// 		sum = (sum & 0xffff) + (sum >> 16);

//     // std::cout << std::hex << sum << " "<< ~((uint16_t)sum) << " " << (uint16_t)(~sum) << std::endl;
// 	return ~((uint16_t)sum);
// }

// Function to calculate the TCP checksum
uint16_t Packet::tcp_checksum(const struct iphdr *ip, const struct tcphdr *tcp, const char *data, size_t data_len) const
{
    // Pseudo-header for TCP checksum calculation
    struct pseudo_tcp_header {
        uint32_t source_address;
        uint32_t destination_address;
        uint8_t reserved;
        uint8_t protocol;
        uint16_t tcp_length;
    } pseudo_tcp_header;

    // Initialize the pseudo-header
    pseudo_tcp_header.source_address = ip->saddr;
    pseudo_tcp_header.destination_address = ip->daddr;
    pseudo_tcp_header.reserved = 0;
    pseudo_tcp_header.protocol = IPPROTO_TCP;
    pseudo_tcp_header.tcp_length = htons(sizeof(struct tcphdr) + data_len);

    // Calculate the checksum for the pseudo-header
    uint32_t sum = 0;
    const uint16_t *ptr = reinterpret_cast<const uint16_t *>(&pseudo_tcp_header);
    for (size_t i = 0; i < sizeof(pseudo_tcp_header) / 2; i++) {
        sum += ntohs(ptr[i]);
    }

    // std::cout << "psudoheader:" << std::hex << sum << std::endl;

    // Calculate the checksum for the TCP header
    ptr = reinterpret_cast<const uint16_t *>(tcp);
    for (size_t i = 0; i < sizeof(struct tcphdr) / 2; i++) {
        sum += ntohs(ptr[i]);
    }
    sum -= ntohs(tcp->th_sum);

    // std::cout << "psudo_hdr+tcp_hdr:" << std::hex << sum << std::endl;

    // Calculate the checksum for the TCP data (if any)
    ptr = reinterpret_cast<const uint16_t *>(data);
    size_t i;
    // std::cout << "data_len: " << data_len << std::endl;
    for (i = 0; (i + sizeof (*ptr)) <= data_len; i += sizeof (*ptr)) {
        sum += ntohs(*ptr++);
        // std::cout << i << ":" << std::hex << sum << std::endl;
    }
		
    // std::cout << ":" << std::hex << sum << std::endl;
	if (i < data_len)
		sum += ((uint16_t) data[i]) << 8;
    
    // std::cout << "All:" << std::hex << sum << std::endl;

    // Add the carry to the sum
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    // Take the one's complement of the sum
    uint16_t checksum = static_cast<uint16_t>(~sum);

    // std::cout << "Checksum: " << std::hex << checksum << std::endl;

    return checksum;
}

// uint16_t Packet::tcp_checksum(const struct iphdr *ip, const struct tcphdr *tcp, const char *data, size_t data_len) const
// {
//     const uint16_t *ptr = reinterpret_cast<const uint16_t *>(tcp);
//     uint32_t sum = 0;
//     size_t i;

//     sum += ip->protocol;
//     sum += ntohs((uint16_t) (ip->saddr & 0xffff));
//     sum += ntohs((uint16_t) (ip->saddr >> 16));
//     sum += ntohs((uint16_t) (ip->daddr & 0xffff));
//     sum += ntohs((uint16_t) (ip->daddr >> 16));
//     sum += ntohs(ip->tot_len) - (ip->ihl * sizeof (uint32_t));

//     for (i = 0; i < sizeof(struct tcphdr)/2; i++)
//         sum += ntohs(*ptr++);

//     ptr = reinterpret_cast<const uint16_t *>(data);
//     for (i = 0; i < data_len/2; i++)
//         sum += ntohs(*ptr++);

//     if (data_len & 1)
//         sum += ((uint16_t) data[data_len - 1]) << 8;

//     while (sum > 0xffff)
//         sum = (sum & 0xffff) + (sum >> 16);

//     return ~((uint16_t)sum);
// }

// Can be improved, I assume that if ipheader has non-zero fragment offset, then 
// transport layer header is not here
bool Packet::has_transport_layer_hdr() const {
    struct iphdr *ip = get_iphdr();
    return (ntohs(ip->frag_off) % (1<<13)) == 0;
}
