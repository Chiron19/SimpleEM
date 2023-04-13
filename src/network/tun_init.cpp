#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "utils.hpp"
#include "network/tun_init.hpp"

int create_tun(char *dest, const char *addr, const char *mask) {
	static const char clonedev[] = "/dev/net/tun";
	struct sockaddr_in sin;
	struct ifreq ifr;
	int fd, ifd;

	log_event("Create TUN interface: '%s'", dest);

	fd = open(clonedev, O_RDWR);
	if (fd < 0)
		panic("open");

	memset(&ifr, 0, sizeof (ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);

	if (ioctl(fd, TUNSETIFF, &ifr) < 0)
		panic("ioctl(TUNSETIFF)");

	strncpy(dest, ifr.ifr_name, IFNAMSIZ);

	log_event("bring TUN interface up");

	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_flags =
		(short) (IFF_UP | IFF_BROADCAST | IFF_MULTICAST | IFF_DYNAMIC);

	ifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (ifd < 0)
		panic("socket");

	if (ioctl(ifd, SIOCSIFFLAGS, &ifr) < 0)
		panic("ioctl(SIOCSIFFLAGS)");

	log_event("give TUN interface addr: '%s'", addr);

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_addr = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFADDR, &ifr) < 0)
		panic("ioctl(SIOCSIFADDR)");

	log_event("give TUN interface mask: '%s'", mask);

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, mask, &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_netmask = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFNETMASK, &ifr) < 0)
		panic("ioctl(SIOCSIFNETMASK)");

	/* Setting the fd to have non-blocking read */
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);

	return fd;
}