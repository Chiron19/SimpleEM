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

class Network {

    int procs, tun_fd;
    std::vector<std::vector<int>> latency;
    std::vector<std::pair<std::string, int>> addresses;
    struct timespec max_latency;
    const std::string interface, inter_addr, inter_mask;

public:

    /** \brief Load network configuration from config file
     * 
     * Load network configuration from file \p config_path
     * The expected file format is as follows:
     * - first line consists of single number 'procs' - number of processes
     * to emulate
     * - next 'procs' lines consist of two entries, i-th line consists of 
     * the address under which the i-th process receives packets (in number 
     * dot form) and the port.
     * - next 'procs' lines consist of 'procs' numbers each. The j-th number
     * in i-th line is the latency, when sending packet from i-th node to j-th
     * node in miliseconds. Values on diagonal of the matrix have 
     * to be equal to 0.
     */
    Network(std::string config_path, std::string interface,
        std::string inter_addr, std::string inter_mask);

    struct timespec get_latency(int em_id1, int em_id2) const;

    struct timespec get_max_latency() const;

    int get_procs() const;

    int get_em_id(const std::string& address) const;

    std::string get_addr(int em_id) const;
    std::string get_inter_addr() const;

    void send(const Packet& packet) const;

    ssize_t receive(char* buffer, size_t buffer_size) const;

private:

    void create_tun();

};

Network::Network(std::string config_path, std::string interface,
        std::string inter_addr, std::string inter_mask): 
        interface(interface), inter_addr(inter_addr), inter_mask(inter_mask) {
    create_tun();

    std::ifstream config(config_path);
    std::string line, address;
    int port, lat;
    max_latency = timespec{0, 0};

    config >> procs;

    for (int i = 0; i < procs; ++i) {
        config >> address >> port;
        addresses.push_back(std::make_pair(address, port));
    }

    for (int i = 0; i < procs; ++i) {
        latency.push_back(std::vector<int>());

        for (int j = 0; j < procs; ++j) {
            config >> lat;
            if (i == j && lat != 0) {
                std::cout << "NETWORK CONFIG NON-ZERO AT DIAGONAL" << std::endl;
                exit(1);
            }
            latency.back().push_back(lat * MILLISECOND); // latency is given in miliseconds
            if (ts_from_nano(latency.back().back()) > max_latency)
                max_latency = ts_from_nano(latency.back().back());
        }
    }
}

struct timespec Network::get_latency(int em_id1, int em_id2) const {
    return ts_from_nano(latency[em_id1][em_id2]);
}

struct timespec Network::get_max_latency() const {
    return max_latency;
}

int Network::get_procs() const {
    return procs;
}

int Network::get_em_id(const std::string& address) const {
    for (int em_id = 0; em_id < procs; ++em_id) {
        if (address == addresses[em_id].first)
            return em_id;
    }
    return -1; // address unavailable
}

std::string Network::get_addr(int em_id) const {
    return addresses[em_id].first;
}
std::string Network::get_inter_addr() const {
    return inter_addr;
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

    log_event("Packet send from %s.%d to %s.%d", 
        packet.get_source_addr().c_str(), packet.get_source_port(), 
        packet.get_dest_addr().c_str(), packet.get_dest_port());
}

ssize_t Network::receive(char* buffer, size_t buffer_size) const {
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
    strcpy(inter, interface.c_str());

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
	inet_pton(AF_INET, inter_addr.c_str(), &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);
	ifr.ifr_addr = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFADDR, &ifr) < 0)
		panic("ioctl(SIOCSIFADDR)");

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, inter_mask.c_str(), &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, inter, IFNAMSIZ);
	ifr.ifr_netmask = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFNETMASK, &ifr) < 0)
		panic("ioctl(SIOCSIFNETMASK)");

	/* Setting the tun_fd to have non-blocking read */
	int flags = fcntl(tun_fd, F_GETFL, 0);
	fcntl(tun_fd, F_SETFL, flags | O_NONBLOCK);
}
