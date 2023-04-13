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


#define MTU 1500



int main() {
	char 
		dev[IFNAMSIZ] = "tun0", 
		inter_addr[16] = "172.16.0.1",
		mask[16] = "255.240.0.0",
		buf[MTU];
	int fd = create_tun(dev, inter_addr, mask);
	sender_desc_t desc;
	ssize_t ssize;

	clients_desc_t clients;
	memset(&clients, 0, sizeof(clients));

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