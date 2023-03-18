#include <assert.h>
#include <string.h>

#include "trans_frame.h"
#include "utils.h"

uint16_t tcp_checksum(const struct iphdr *ip, const char *buf, size_t len) {
	const struct tcphdr *tcp = (const void *) buf;
	const uint16_t *ptr;
	uint32_t sum = 0;
	size_t i;

	sum += ip->protocol;
	sum += ntohs((uint16_t) (ip->saddr & 0xffff));
	sum += ntohs((uint16_t) (ip->saddr >> 16));
	sum += ntohs((uint16_t) (ip->daddr & 0xffff));
	sum += ntohs((uint16_t) (ip->daddr >> 16));
	sum += ntohs(ip->tot_len) - (ip->ihl * sizeof (uint32_t));

	ptr = (const uint16_t *) buf;

	for (i = 0; (i + sizeof (*ptr)) <= len; i += sizeof (*ptr))
		sum += ntohs(*ptr++);

	if (i < len)
		sum += ((uint16_t) buf[i]) << 8;

	sum -= ntohs(tcp->th_sum);

	while (sum > 0xffff)
		sum = (sum & 0xffff) + (sum >> 16);

	return ~((uint16_t) sum);
}

uint16_t ip4_checksum(const struct iphdr *ip) {
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

void write_or_die(int fd, const char *buf, size_t len) {
	ssize_t ssize;

	while (len > 0) {
		ssize = write(fd, buf, len);
		if (ssize < 0)
			panic("write");

		buf += (size_t) ssize;
		len -= (size_t) ssize;
	}
}

void transfer_ip4_udp(int fd, const char* buf, size_t len) {	
	int8_t msg_buf[len];
	struct iphdr *ip = (struct iphdr*)buf;
	const struct udphdr *udp = (const void *)(buf + sizeof(*ip));

	int helpy = ip->daddr;
	ip->daddr = ip->saddr;
	ip->saddr = helpy;
	ip->check = htons(ip4_checksum(ip));

	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ip->saddr, src, sizeof (src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof (dst));
	printf("Transfering packet from: %s:%u -> %s:%u\n",
		src, ntohs(udp->uh_sport), 
		dst, ntohs(udp->uh_dport)); 

	size_t off = 0;
	for (; off < sizeof(struct iphdr); ++off) {
		msg_buf[off] = *(((uint8_t*) ip) + off);
	}
	for (; off < len; ++off) {
		msg_buf[off] = buf[off];
	}
	write_or_die(fd, (void *) msg_buf, len);
}

void transfer_ip4_tcp(int fd, const char* buf, size_t len) {
	int8_t msg_buf[len];
	struct iphdr *ip = (struct iphdr*)buf;
	const struct tcphdr *tcp = (const void *)(buf + sizeof(*ip));

	int helpy = ip->daddr;
	ip->daddr = ip->saddr;
	ip->saddr = helpy;
	ip->check = htons(ip4_checksum(ip));

	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &ip->saddr, src, sizeof(src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof(dst));
	printf("Transfering packet from: %s:%u -> %s:%u\n",
		src, ntohs(tcp->th_sport), 
		dst, ntohs(tcp->th_dport)); 

	size_t off = 0;
	for (; off < sizeof(struct iphdr); ++off) {
		msg_buf[off] = *(((uint8_t*) ip) + off);
	}
	for (; off < len; ++off) {
		msg_buf[off] = buf[off];
	}
	write_or_die(fd, (void *) msg_buf, len);
}

void transfer_ip4_tcp_syn(int fd, const struct iphdr *ip, const struct tcphdr *tcp) {
    msg_t msg;

	msg.ip = *ip;
	msg.ip.saddr = ip->daddr;
	msg.ip.daddr = ip->saddr;
	msg.ip.tot_len = htons(sizeof (msg));
	msg.ip.check = htons(ip4_checksum(&msg.ip));

	msg.tcp = *tcp;
	msg.tcp.th_sport = tcp->th_dport;
	msg.tcp.th_dport = tcp->th_sport;
	msg.tcp.th_ack = htonl(ntohl(tcp->th_seq) + 1);
	msg.tcp.th_seq = htonl(42);
	msg.tcp.ack = 1;
	msg.tcp.doff = (sizeof (msg) - sizeof (msg.ip)) / sizeof (uint32_t);

	msg.tcpopt_mss.kind = TCPOPT_MAXSEG;
	msg.tcpopt_mss.len = TCPOLEN_MAXSEG;
	msg.tcpopt_mss.val = htons(1460);

	msg.tcpopt_wscale.kind = TCPOPT_WINDOW;
	msg.tcpopt_wscale.len = TCPOLEN_WINDOW;
	msg.tcpopt_wscale.val = 7;

	memset(msg.pad, 0, sizeof (msg.pad));

	msg.tcp.th_sum = htons(tcp_checksum(&msg.ip, (void *) &msg.tcp,
					    sizeof (msg) - sizeof (msg.ip)));

	printf("\n");
	printf("reply SYN/ACK:\n");
	dump((char *) &msg, sizeof (msg));

	write_or_die(fd, (void *) &msg, sizeof (msg));
}