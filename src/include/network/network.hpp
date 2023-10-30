#pragma once

#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "packet.hpp"
#include "config-parser.hpp"
#include "logger.hpp"

/** @brief Class responsible for all network control and communication
 * 
 * Class builds and interacts with TUN interface. It sets all necessary 
 * interface settings and allows to send and receive packets through 
 * TUN file descriptor. This class also allows translation between emulator's
 * internal process id and process address and port in the TUN subnetwork. 
 * Additionally the pairwise processes latencies can be read through
 * the functionality provided by this class.
*/
class Network {

    int tun_fd; ///< FD of the TUN interface
    const ConfigParser& cp; ///< Configuration read from the file by emulator
    struct timespec max_latency; ///< Calculated max pairwise latency

public:

    /** @brief Build TUN interface and set necessary constants
     * 
     * @param cp Emulator's configuration read from a file 
     */
    Network(const ConfigParser& cp);

    /** @brief Get latency between process @p em_id1 and process @p em_id2 
     * 
     * @param em_id1 First process
     * @param em_id2 Second process
     * @return Pairwise latency between those two processes
     */
    struct timespec get_latency(int em_id1, int em_id2) const;

    /** @brief Get maximum pairwise latency in the network
     * 
     * @return Maximum pairwise latency
     */
    struct timespec get_max_latency() const;

    /** @brief Get number of processes in the network
     * 
     * @return Number of processes in the network
     */
    int get_procs() const;

    /** @brief Get emulator's internal id of process with given address
     * 
     * @param address The address (in number/dot form) on which to query
     * @return Internal id of process associated with this address (-1 if none)
     */
    int get_em_id(const std::string& address) const;

    /** @brief Get address of a process with given internal id
     * 
     * @param em_id The id on which to query
     * @return Address (in number/dot form) associated with this internal id
     */
    std::string get_addr(int em_id) const;

    /** @brief Get address of TUN interface
     * 
     * @return Address (in number/dot form) of the TUN interface
     */
    std::string get_inter_addr() const;

    /** @brief Send the buffer of @p packet object through TUN FD
     * 
     * @param packet Packet which will be sent
     */
    void send(const Packet& packet) const;

    /** @brief Receive data through TUN FD
     * 
     * @param buffer Placeholder to which received data will be copied
     * @param buffer_size Available place for new data
     * @return Number of received bytes (0 if none)
     */
    ssize_t receive(char* buffer, size_t buffer_size) const;

private:

    /** @brief Creates and customizes the TUN interface and its subnetwork
     */
    void create_tun();

};

Network::Network(const ConfigParser& cp): cp(cp) {
    create_tun();

    max_latency = timespec{0, 0};

    for (int i = 0; i < cp.procs; ++i) {
        for (int j = 0; j < cp.procs; ++j) {
            if (ts_from_nano(cp.latency[i][j]) > max_latency) 
                max_latency = ts_from_nano(cp.latency[i][j]);
        }
    }

    // for (int i = 0; i < cp.procs; ++i) {
    //     printf("[Network.hpp] em_id: %d, addr: %s\n", i, get_addr(i).c_str());
    // }
}

struct timespec Network::get_latency(int em_id1, int em_id2) const {
    return ts_from_nano(cp.latency[em_id1][em_id2]);
}

struct timespec Network::get_max_latency() const {
    return max_latency;
}

int Network::get_procs() const {
    return cp.procs;
}

int Network::get_em_id(const std::string& address) const {
    for (int em_id = 0; em_id < cp.procs; ++em_id) {
        if (address == cp.addresses[em_id].first)
            return em_id;
    }
    return -1; // address unavailable
}

std::string Network::get_addr(int em_id) const {
    return cp.addresses[em_id].first;
}
std::string Network::get_inter_addr() const {
    return cp.tun_addr;
}

void Network::send(const Packet& packet) const {
    ssize_t ssize;
    size_t to_send = packet.get_size(), offset = 0;

	while (to_send > 0) {
		ssize = write(tun_fd, packet.get_buffer(), to_send);
		if (ssize < 0)
			panic("write");

		offset += (size_t) ssize;
		to_send -= (size_t) ssize;
	}

    logger_ptr->log_event("Packet send from %s.%d to %s.%d", 
        packet.get_source_addr().c_str(), packet.get_source_port_tcp(), 
        packet.get_dest_addr().c_str(), packet.get_dest_port_tcp());
    // printf("Packet send from %s.%d to %s.%d\n", 
    //     packet.get_source_addr().c_str(), packet.get_source_port_tcp(), 
    //     packet.get_dest_addr().c_str(), packet.get_dest_port_tcp());
}

ssize_t Network::receive(char* buffer, size_t buffer_size) const {
    // printf("[network.hpp]Buffer: %s\n", buffer);
    ssize_t ssize = read(tun_fd, buffer, buffer_size);
    if (ssize < 0 && errno != EAGAIN)
        panic("read");
    return ssize;
}

void Network::create_tun() {
	static const char clonedev[] = "/dev/net/tun";
	struct sockaddr_in sin;
	struct ifreq ifr;
	int ifd;
    char inter[IFNAMSIZ];
    strcpy(inter, cp.tun_dev_name.c_str());

	tun_fd = open(clonedev, O_RDWR);
	if (tun_fd < 0)
		panic("open");

	memset(&ifr, 0, sizeof (ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);

	if (ioctl(tun_fd, TUNSETIFF, &ifr) < 0)
		panic("ioctl(TUNSETIFF)");

	strncpy(inter, ifr.ifr_name, IFNAMSIZ);

	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);
	ifr.ifr_flags =
		(short) (IFF_UP | IFF_BROADCAST | IFF_MULTICAST | IFF_DYNAMIC);

	ifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (ifd < 0)
		panic("socket");

	if (ioctl(ifd, SIOCSIFFLAGS, &ifr) < 0)
		panic("ioctl(SIOCSIFFLAGS)");

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, cp.tun_addr.c_str(), &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);
	ifr.ifr_addr = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFADDR, &ifr) < 0)
		panic("ioctl(SIOCSIFADDR)");

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, cp.tun_mask.c_str(), &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);
	ifr.ifr_netmask = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFNETMASK, &ifr) < 0)
		panic("ioctl(SIOCSIFNETMASK)");

	/* Setting the tun_fd to have non-blocking read */
	int flags = fcntl(tun_fd, F_GETFL, 0);
	fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);
}
