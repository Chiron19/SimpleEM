#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>


#define MTU 1500


#define panic(_str) do { perror(_str); abort(); } while (0)


static void write_or_die(int fd, const char *buf, size_t len)
{
	ssize_t ssize;

	while (len > 0) {
		ssize = write(fd, buf, len);
		if (ssize < 0)
			panic("write");

		buf += (size_t) ssize;
		len -= (size_t) ssize;
	}
}

static int create_tun(char *dest, const char *addr, const char *mask)
{
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

	ifd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (ifd < 0)
		panic("socket");

	printf("bring TUN interface up\n");

	memset(&ifr, 0, sizeof (ifr));
	strncpy(ifr.ifr_name, dest, IFNAMSIZ);
	ifr.ifr_flags =
		(short) (IFF_UP | IFF_BROADCAST | IFF_MULTICAST | IFF_DYNAMIC);

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

	return fd;
}

static void dump(const char *buf, size_t len)
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

static void process_ip6(int fd __attribute__ ((unused)),
			const char *buf, size_t len)
{
	const struct ip6_hdr *ip = (const void *) buf;
	char src[INET6_ADDRSTRLEN], dst[INET6_ADDRSTRLEN];

	if (len < sizeof (*ip))
		return;

	inet_ntop(AF_INET6, &ip->ip6_src, src, sizeof (src));
	inet_ntop(AF_INET6, &ip->ip6_dst, dst, sizeof (dst));

	printf("ipv6: %s -> %s\n", src, dst);
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

static uint16_t tcp_checksum(const struct iphdr *ip, const char *buf,
			     size_t len)
{
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

static void process_ip4_tcp_payload(int fd __attribute__ ((unused)),
				    const struct iphdr *ip
				    __attribute__ ((unused)),
				    const struct tcphdr *tcp
				    __attribute__ ((unused)), const char *buf,
				    size_t len)
{
	printf("payload (%lu bytes):\n", len);
	dump(buf, len);
}

static void process_ip4_tcp_syn(int fd, const struct iphdr *ip,
				const struct tcphdr *tcp)
{
	struct {
		struct iphdr ip;
		struct tcphdr tcp;
		struct {
			uint8_t kind;
			uint8_t len;
			uint16_t val;
		} tcpopt_mss;
		struct {
			uint8_t kind;
			uint8_t len;
			uint8_t val;
		} tcpopt_wscale;
		uint8_t pad[1];
	} __attribute__ ((packed)) msg;

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

static void process_ip4_tcp(int fd, const struct iphdr *ip, const char *buf,
			     size_t len)
{
	const struct tcphdr *tcp = (const void *) buf;
	size_t hdrlen;

	if (len < sizeof (*tcp))
		return;

	printf("tcp: %hu -> %hu\n", ntohs(tcp->th_sport),ntohs(tcp->th_dport));
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
		process_ip4_tcp_syn(fd, ip, tcp);
		return;
	}

	hdrlen = tcp->doff * sizeof (uint32_t);

	if (hdrlen > len)
		return;

	buf += hdrlen;
	len -= hdrlen;
	process_ip4_tcp_payload(fd, ip, tcp, buf, len);
}

static void process_ip4(int fd, const char *buf, size_t len)
{
	const struct iphdr *ip = (const void *) buf;
	char src[INET_ADDRSTRLEN], dst[INET_ADDRSTRLEN];

	if (len < sizeof (*ip))
		return;

	inet_ntop(AF_INET, &ip->saddr, src, sizeof (src));
	inet_ntop(AF_INET, &ip->daddr, dst, sizeof (dst));

	printf("ipv4: %s -> %s\n", src, dst);

	switch (ip->protocol) {
	case IPPROTO_TCP:
		buf += sizeof (*ip);
		len -= sizeof (*ip);
		process_ip4_tcp(fd, ip, buf, len);
	default:
		return;
	}
}

static void process(int fd, const char *buf, size_t len)
{
	if (len < 1)
		return;

	switch (buf[0] >> 4) {
	case 4: process_ip4(fd, buf, len); break;
	case 6: process_ip6(fd, buf, len); break;
	default: return;
	}
}

int main(int argc, const char **argv)
{
	char dev[IFNAMSIZ], addr[16], mask[16], buf[MTU];
	ssize_t ssize;
	int fd;

	if (argc != 4) {
		fprintf(stderr, "Syntax: ./tinyem <tun-name> <addr> <mask>\n");
		abort();
	}

	strncpy(dev, argv[1], sizeof (dev));
	dev[sizeof(dev) - 1] = '\0';

	strncpy(addr, argv[2], sizeof (addr));
	addr[sizeof(addr) - 1] = '\0';

	strncpy(mask, argv[3], sizeof (mask));
	mask[sizeof(mask) - 1] = '\0';

	fd = create_tun(dev, addr, mask);

	while (1) {
		ssize = read(fd, buf, sizeof (buf));
		if (ssize < 0)
			panic("read");

		printf("----------------------------------------");
		printf("----------------------------------------\n");
		dump(buf, (size_t) ssize);

		printf("\n");
		process(fd, buf, (size_t) ssize);

		printf("\n");
	}

	return EXIT_SUCCESS;
}
