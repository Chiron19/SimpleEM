#define _DEFAULT_SOURCE 1

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>
#include <stdio.h>

#include "proc_frame.hpp"
#include "utils.hpp"

sender_desc_t process(const char *buf, size_t len) {
	if (len == 0) {
		printf("[ERROR] Buffer of length 0.\n");
		exit(0);
	}

	switch (buf[0] >> 4) {
	case 4: return process_ip4(buf, len); 
	case 6: return process_ip6(buf, len); 
	default: 
		printf("[ERROR] Unknown IP version.\n");
		exit(0);
	}
}

sender_desc_t process_ip6(const char *buf, size_t len) {
	const struct ip6_hdr *ip = reinterpret_cast<const struct ip6_hdr *>(buf);
	char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
		printf("[ERROR] ipv6 buffer shorter then ipv6 header.\n");
        exit(0);
    }

	inet_ntop(AF_INET6, &ip->ip6_src, src, sizeof (src));
	inet_ntop(AF_INET6, &ip->ip6_dst, dst, sizeof (dst));

	printf("[MESSAGE] ipv6: %s -> %s\n", src, dst);
	
	sender_desc_t desc;
	desc.addr.sin_family = AF_INET6;
	return desc;
}

sender_desc_t process_ip4(const char *buf, size_t len) {
	const struct iphdr *ip = reinterpret_cast<const struct iphdr *>(buf);
	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
		printf("[ERROR] ipv4 buffer shorter then ipv4 header.\n");
		exit(0);
	}

	inet_ntop(AF_INET, &ip->saddr, src, sizeof (src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof (dst));

	printf("ipv4: %s -> %s\n", src, dst);

	sender_desc_t desc;

	switch (ip->protocol) {
	case IPPROTO_TCP:
		// Here we decide to pass the full buffer to tcp processing function
		desc = process_ip4_tcp(ip, buf, len);
		return desc;
	case IPPROTO_UDP:
		// Here we decide to pass the full buffer to udp processing function
		desc = process_ip4_udp(ip, buf, len);
		return desc;
	default:
		printf("[ERROR] unknown ipv4 protocol.\n");
		exit(0);
	}
}

sender_desc_t process_ip4_tcp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct tcphdr *tcp = reinterpret_cast<const struct tcphdr *>(buf + sizeof(*ip));

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

	sender_desc_t desc;
	desc.addr.sin_family = AF_INET;
    desc.addr.sin_port = tcp->th_dport;
	// desc.addr.sin_addr.s_addr = ip->daddr;
	desc.addr.sin_addr.s_addr = INADDR_ANY; // we want to send to lo
	desc.protocol = IPPROTO_TCP;
	return desc;
}

sender_desc_t process_ip4_udp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct udphdr *udp = reinterpret_cast<const struct udphdr *>(buf + sizeof(*ip));

	printf("udp: %hu -> %hu    len: %hu, sum: %hu\n", 
		ntohs(udp->uh_sport), ntohs(udp->uh_dport),
		ntohs(udp->uh_ulen), ntohs(udp->uh_sum));
	size_t data_offset = sizeof(*ip) + 8;
	dump(buf + data_offset, len - data_offset);

	sender_desc_t desc;
	desc.addr.sin_family = AF_INET;
    desc.addr.sin_port = udp->uh_dport;
	desc.addr.sin_addr.s_addr = ip->daddr;
	// desc.addr.sin_addr.s_addr = INADDR_ANY; // we want to send to lo
	desc.protocol = IPPROTO_UDP;
	return desc;
}
