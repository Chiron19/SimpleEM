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

class Network {

    int tun_fd;
    const ConfigParser& cp;
    struct timespec max_latency;

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
    Network(const ConfigParser& cp);

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

Network::Network(const ConfigParser& cp): cp(cp) {
    create_tun();

    max_latency = timespec{0, 0};

    for (int i = 0; i < cp.procs; ++i) {
        for (int j = 0; j < cp.procs; ++j) {
            if (ts_from_nano(cp.latency[i][j]) > max_latency) 
                max_latency = ts_from_nano(cp.latency[i][j]);
        }
    }
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
