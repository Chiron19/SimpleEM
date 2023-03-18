#define _DEFAULT_SOURCE 1

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

#include "tinyem.h"
#include "utils.h"
#include "proc_frame.h"
#include "trans_frame.h"

#define MTU 1500



int create_tun(char *dest, const char *addr, const char *mask) {
	static const char clonedev[] = "/dev/net/tun";
	struct sockaddr_in sin;
	struct ifreq ifr;
	int fd, ifd;

	printf("create TUN interface: '%s'\n", dest);

	fd = open(clonedev, O_RDWR);
	if (fd < 0)
		panic("open");

	memset(&ifr, 0, sizeof (ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);

	if (ioctl(fd, TUNSETIFF, &ifr) < 0)
		panic("ioctl(TUNSETIFF)");

	strncpy(dest, ifr.ifr_name, IFNAMSIZ);

	printf("bring TUN interface up\n");

	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_flags =
		(short) (IFF_UP | IFF_BROADCAST | IFF_MULTICAST | IFF_DYNAMIC);

	ifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (ifd < 0)
		panic("socket");
	printf("fd value: %d ifd value: %d\n", fd, ifd);

	if (ioctl(ifd, SIOCSIFFLAGS, &ifr) < 0)
		panic("ioctl(SIOCSIFFLAGS)");

	printf("give TUN interface addr: '%s'\n", addr);

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_addr = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFADDR, &ifr) < 0)
		panic("ioctl(SIOCSIFADDR)");

	printf("give TUN interface mask: '%s'\n", mask);

	sin.sin_family = AF_INET;
	inet_pton(AF_INET, mask, &sin.sin_addr);
	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_netmask = *(struct sockaddr *) &sin;

	if (ioctl(ifd, SIOCSIFNETMASK, &ifr) < 0)
		panic("ioctl(SIOCSIFNETMASK)");

	printf("\n");

	return fd;
}



int create_sending_socket() {
	int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Socket creation failed...\n");
        exit(0);
    }   
	return sockfd;
}

void send_socket(int sock, struct sockaddr_in recvaddr, const char* data, size_t len) {
	sendto(sock, data, len, MSG_CONFIRM, (const struct sockaddr *) &recvaddr, sizeof(recvaddr));
	printf("Message sent.\n");

	// int n;
    // socklen_t len;
    // n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
    //             MSG_WAITALL, (struct sockaddr *) &servaddr,
    //             &len);
    // buffer[n] = '\0';

    // printf("Message received: %s\n", buffer);
}


int main() {
	char 
		dev[IFNAMSIZ] = "tun0", 
		inter_addr[16] = "172.16.0.1",
		mask[16] = "255.240.0.0",
		buf[MTU];
	int fd = create_tun(dev, inter_addr, mask);
	int sock = create_sending_socket();
	sender_desc_t desc;
	ssize_t ssize;

	while (1) {
		if ((ssize = read(fd, buf, sizeof(buf))) < 0)
			panic("read");
		printf("\n");
		
		memset(&desc, 0, sizeof(desc));
		desc = process(buf, (size_t) ssize);

		if (desc.addr.sin_family == AF_INET) {
			const struct iphdr *ip = (const void *) buf;

			if (desc.protocol == IPPROTO_UDP) {
				transfer_ip4_udp(fd, buf, ssize);
			}
			if (desc.protocol == IPPROTO_TCP) {
				transfer_ip4_tcp(fd, buf, ssize);
			}


		}
	}

	close(sock);
	return EXIT_SUCCESS;
}



















// static void process_ip6(int fd __attribute__ ((unused)),
// 			const char *buf, size_t len)
// {
// 	const struct ip6_hdr *ip = (const void *) buf;
// 	char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

// 	if (len < sizeof (*ip))
// 		return;

	// inet_ntop(AF_INET6, &ip->ip6_src, src, sizeof (src));
// 	inet_ntop(AF_INET6, &ip->ip6_dst, dst, sizeof (dst));

// 	printf("ipv6: %s -> %s\n", src, dst);
// }


// void process_ip4_tcp_syn(int fd, const struct iphdr *ip,
// 				const struct tcphdr *tcp)
// {
// 	msg_t msg;

// 	msg.ip = *ip;
// 	msg.ip.saddr = ip->daddr;
// 	msg.ip.daddr = ip->saddr;
// 	msg.ip.tot_len = htons(sizeof (msg));
// 	msg.ip.check = htons(ip4_checksum(&msg.ip));

// 	msg.tcp = *tcp;
// 	msg.tcp.th_sport = tcp->th_dport;
// 	msg.tcp.th_dport = tcp->th_sport;
// 	msg.tcp.th_ack = htonl(ntohl(tcp->th_seq) + 1);
// 	msg.tcp.th_seq = htonl(42);
// 	msg.tcp.ack = 1;
// 	msg.tcp.doff = (sizeof (msg) - sizeof (msg.ip)) / sizeof (uint32_t);

// 	msg.tcpopt_mss.kind = TCPOPT_MAXSEG;
// 	msg.tcpopt_mss.len = TCPOLEN_MAXSEG;
// 	msg.tcpopt_mss.val = htons(1460);

// 	msg.tcpopt_wscale.kind = TCPOPT_WINDOW;
// 	msg.tcpopt_wscale.len = TCPOLEN_WINDOW;
// 	msg.tcpopt_wscale.val = 7;

// 	memset(msg.pad, 0, sizeof (msg.pad));

// 	msg.tcp.th_sum = htons(tcp_checksum(&msg.ip, (void *) &msg.tcp,
// 					    sizeof (msg) - sizeof (msg.ip)));

// 	printf("\n");
// 	printf("reply SYN/ACK:\n");

// 	write_or_die(fd, (void *) &msg, sizeof (msg));
// }