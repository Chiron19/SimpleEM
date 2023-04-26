#include <assert.h>
#include <string.h>

#include "trans_frame.hpp"
#include "utils.hpp"

uint16_t tcp_checksum(const struct iphdr *ip, const char *buf, size_t len) {
	const struct tcphdr *tcp = reinterpret_cast<const struct tcphdr *>(buf);
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
