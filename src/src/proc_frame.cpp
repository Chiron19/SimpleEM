#define _DEFAULT_SOURCE 1

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <cstring>
#include <cassert>

#include "proc_control/proc_frame.hpp"
#include "utils.hpp"

static void write_or_die(const char *buf, size_t len)
{
	ssize_t ssize;
	// while (len > 0) {
	// 	ssize = write(tun_fd, buf, len);
	// 	if (ssize < 0)
	// 		panic("write");

	// 	buf += (size_t) ssize;
	// 	len -= (size_t) ssize;
	// }
}

void dump(const char *buf, size_t len)
{
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

void dump_short(const char *buf, size_t len)
{
	size_t i, j;

	for (i = 0; i < len; i++) {
		if (i > 7 && i < len - 8) continue;
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

static uint16_t ip4_checksum(const struct iphdr *ip)
{
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

void process(const char *buf, size_t len) {
	if (len == 0) {
		printf("[ERROR] Buffer of length 0.\n");
		exit(0);
	}

	switch (buf[0] >> 4) {
	case 4: 
		process_ip4(buf, len); 
		break;
	case 6: 
		process_ip6(buf, len); 
		break;
	default: 
		printf("[ERROR] Unknown IP version.\n");
		exit(0);
	}
}

void process_ip6(const char *buf, size_t len) {
	const struct ip6_hdr *ip = reinterpret_cast<const struct ip6_hdr *>(buf);
	char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
		printf("[ERROR] ipv6 buffer shorter then ipv6 header.\n");
        exit(0);
    }

	inet_ntop(AF_INET6, &ip->ip6_src, src, sizeof (src));
	inet_ntop(AF_INET6, &ip->ip6_dst, dst, sizeof (dst));

	printf("[MESSAGE] ipv6: %s -> %s\n", src, dst);
}

static void process_ip4_tcp_payload(const struct iphdr *ip
				    __attribute__ ((unused)),
				    const struct tcphdr *tcp
				    __attribute__ ((unused)), const char *buf,
				    size_t len)
{
	printf("payload (%lu bytes):\n", len);
	dump_short(buf, len);
}

// static void process_ip4_tcp_syn(const struct iphdr *ip,
// 				const struct tcphdr *tcp)
// {
// 	struct {
// 		struct iphdr ip;
// 		struct tcphdr tcp;
// 		struct {
// 			uint8_t kind;
// 			uint8_t len;
// 			uint16_t val;
// 		} tcpopt_mss;
// 		struct {
// 			uint8_t kind;
// 			uint8_t len;
// 			uint8_t val;
// 		} tcpopt_wscale;
// 		uint8_t pad[1];
// 	} __attribute__ ((packed)) msg;

// 	msg.ip = *ip;
// 	msg.ip.saddr = ip->daddr;
// 	msg.ip.daddr = ip->saddr;
// 	msg.ip.tot_len = htons(sizeof (msg));
// 	msg.ip.check = htons(ip4_checksum((struct iphdr *) ip));

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

// 	msg.tcp.th_sum = htons(tcp_checksum((struct iphdr *)ip, (char *) &msg.tcp,sizeof (msg) - sizeof (msg.ip)));

// 	printf("\n");
// 	printf("reply SYN/ACK:\n");
// 	dump((char *) &msg, sizeof (msg));

// 	write_or_die((char *) &msg, sizeof (msg));
// }

void process_ip4_tcp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct tcphdr *tcp = reinterpret_cast<const struct tcphdr *>(buf);
	size_t hdrlen;

	printf("tcp: %hu -> %hu\n", 
		ntohs(tcp->th_sport), ntohs(tcp->th_dport));
	printf("     flags:");
	if (tcp->urg)
		printf(" URG");
	if (tcp->ack)
		printf(" ACK");
	if (tcp->psh)
		printf(" PSH");
	if (tcp->rst)
		printf(" RST");
	if (tcp->syn)
		printf(" SYN");
	if (tcp->fin)
		printf(" FIN");
	printf("\n");

	if (tcp->syn) {
		// process_ip4_tcp_syn(ip, tcp);
		return;
	}

	hdrlen = tcp->doff * sizeof (uint32_t);
	printf("hdrlen: %ld\n", hdrlen);
	// printf("buffer: %s\n", buf);

	if (hdrlen > len)
		return;

	buf += hdrlen;
	len -= hdrlen;
	process_ip4_tcp_payload(ip, tcp, buf, len);
}

void process_ip4_udp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct udphdr *udp = reinterpret_cast<const struct udphdr *>(buf + sizeof(*ip));

	printf("udp: %hu -> %hu    len: %hu, sum: %hu\n", 
		ntohs(udp->uh_sport), ntohs(udp->uh_dport),
		ntohs(udp->uh_ulen), ntohs(udp->uh_sum));
	size_t data_offset = sizeof(*ip) + 8;
	dump(buf + data_offset, len - data_offset);

}

void process_ip4(const char *buf, size_t len) {
	const struct iphdr *ip = reinterpret_cast<const struct iphdr *>(buf);
	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
		printf("[ERROR] ipv4 buffer shorter then ipv4 header.\n");
		exit(0);
	}

	inet_ntop(AF_INET, &ip->saddr, src, sizeof (src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof (dst));

	printf(" ---- ---- ---- ----\n");
	printf("ipv4: %s -> %s, ihl: %u, tot_len: %d, id: %d, frag_off: %d, protocol: %d\n", src, dst, ip->ihl, ntohs(ip->tot_len), ntohs(ip->id), ntohs(ip->frag_off) % 8192, ip->protocol);

	// printf("buffer: %s\n", buf);

	switch (ip->protocol) {
	case IPPROTO_TCP:
		// Here we decide to pass the full buffer to tcp processing function
		buf += sizeof (*ip);
		len -= sizeof (*ip);
		process_ip4_tcp(ip, buf, len);
		break;
	case IPPROTO_UDP:
		// Here we decide to pass the full buffer to udp processing function
		process_ip4_udp(ip, buf, len);
		break;
	default:
		printf("[ERROR] unknown ipv4 protocol.\n");
		exit(0);
	}
}
