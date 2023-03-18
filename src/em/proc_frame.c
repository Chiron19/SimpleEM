#define _DEFAULT_SOURCE 1

#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#include <arpa/inet.h>

#include <stdio.h>

#include "proc_frame.h"
#include "utils.h"
#include "verbosity.h"

sender_desc_t process(const char *buf, size_t len) {
	if (len == 0) {
        if (REPORT_STRANGE) 
            printf("[process] Buffer of length 0.\n");
		exit(0);
	}

	switch (buf[0] >> 4) {
	case 4: return process_ip4(buf, len); 
	case 6: return process_ip6(buf, len); 
	default: 
		if (REPORT_STRANGE)
			printf("[process] Unknown IP version.\n");
		exit(0);
	}
}

sender_desc_t process_ip6(const char *buf, size_t len) {
	const struct ip6_hdr *ip = (const void *) buf;
	char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
        if (REPORT_STRANGE)
            printf("[process_ip6] Buffer shorter then ipv6 header.\n");
        exit(0);
    }

	inet_ntop(AF_INET6, &ip->ip6_src, src, sizeof (src));
	inet_ntop(AF_INET6, &ip->ip6_dst, dst, sizeof (dst));

	if (REPORT_IPV6)
        printf("ipv6: %s -> %s\n", src, dst);
	
	sender_desc_t desc;
	desc.addr.sin_family = AF_INET6;
	return desc;
}

sender_desc_t process_ip4(const char *buf, size_t len) {
	const struct iphdr *ip = (const void *) buf;
	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];

	if (len < sizeof (*ip)) {
		if (REPORT_STRANGE)
            printf("[process_ip4] Buffer shorter then ipv4 header.\n");
		exit(0);
	}

	inet_ntop(AF_INET, &ip->saddr, src, sizeof (src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof (dst));

    if (REPORT_IPV4) {
        printf("%s\n", FRAME_START);
        printf("ipv4: %s -> %s\n", src, dst);
    }

	sender_desc_t desc;

	switch (ip->protocol) {
	case IPPROTO_TCP:
		// Here we decide to pass the full buffer to tcp processing function
		desc = process_ip4_tcp(ip, buf, len);
		if (REPORT_IPV4) {
        	printf("%s\n", FRAME_END);
    	}
		return desc;
	case IPPROTO_UDP:
		// Here we decide to pass the full buffer to udp processing function
		desc = process_ip4_udp(ip, buf, len);
		if (REPORT_IPV4) {
        	printf("%s\n", FRAME_END);
    	}
		return desc;
	default:
		if (REPORT_IPV4) {
        	printf("%s\n", FRAME_END);
    	}
		if (REPORT_STRANGE) {
			printf("[process_ip4] unknown ipv4 protocol.\n");
		}
		exit(0);
	}
}

sender_desc_t process_ip4_tcp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct tcphdr *tcp = (const void *) (buf + sizeof(*ip));

	printf("tcp: %hu -> %hu\n", 
		   ntohs(tcp->th_sport), ntohs(tcp->th_dport));
	// size_t data_offset = sizeof(*ip) + 8;
	// dump(buf + data_offset, len - data_offset);

	// size_t hdrlen;

	// if (len < sizeof (*tcp))
	// 	return;

	// printf("tcp: %hu -> %hu\n", ntohs(tcp->th_sport),ntohs(tcp->th_dport));
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

	// if (tcp->syn) {
	// 	// process_ip4_tcp_syn(fd, ip, tcp);
	// 	transfer_ip4_tcp_syn(ip, tcp, buf + sizeof(*tcp), len - sizeof(*tcp));
	// 	return;
	// }

	// hdrlen = tcp->doff * sizeof (uint32_t);

	// if (hdrlen > sizeof(*tcp)) {
		// process_ip4_tcp_options(buf + sizeof(*tcp), len - sizeof(*tcp));
	// }

	// if (hdrlen > len)
	// 	return;

	// buf += hdrlen;
	// len -= hdrlen;
	// process_ip4_tcp_payload(ip, tcp, buf, len);

	sender_desc_t desc;
	desc.addr.sin_family = AF_INET;
    desc.addr.sin_port = tcp->th_dport;
	// desc.addr.sin_addr.s_addr = ip->daddr;
	desc.addr.sin_addr.s_addr = INADDR_ANY; // we want to send to lo
	desc.protocol = IPPROTO_TCP;
	return desc;
}

sender_desc_t process_ip4_udp(const struct iphdr *ip, const char *buf, size_t len) {
	const struct udphdr *udp = (const void *) (buf + sizeof(*ip));

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
