/* Command line utility to create GTP link */

/* (C) 2014 by sysmocom - s.f.m.c. GmbH
 * (C) 2016 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * Author: Pablo Neira Ayuso <pablo@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include <linux/gtp.h>
#include <linux/if_link.h>

#include <libgtpnl/gtpnl.h>

struct gtp_server_sock {
	int			family;
	int			fd1;
	int			fd2;
	socklen_t		len;
	struct {
		union {
			struct sockaddr_in	in;
			struct sockaddr_in6	in6;
		} fd1;
		union {
			struct sockaddr_in	in;
			struct sockaddr_in6	in6;
		} fd2;
	} sockaddr;
};

static void setup_sockaddr_in(struct sockaddr_in *sockaddr, uint16_t port)
{
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(port);
	sockaddr->sin_addr.s_addr = INADDR_ANY;
}

static void setup_sockaddr_in6(struct sockaddr_in6 *sockaddr, uint16_t port)
{
	sockaddr->sin6_family = AF_INET6;
	sockaddr->sin6_port = htons(port);
	sockaddr->sin6_addr = in6addr_any;
}

static int setup_socket(struct gtp_server_sock *gtp_sock, int family)
{
	int fd1 = socket(family, SOCK_DGRAM, 0);
	int fd2 = socket(family, SOCK_DGRAM, 0);

	if (fd1 < 0 || fd2 < 0)
		return -1;

	memset(gtp_sock, 0, sizeof(*gtp_sock));

	gtp_sock->family = family;
	gtp_sock->fd1 = fd1;
	gtp_sock->fd2 = fd2;

	switch (family) {
	case AF_INET:
		gtp_sock->len = sizeof(struct sockaddr_in);
		setup_sockaddr_in(&gtp_sock->sockaddr.fd1.in, 3386);
		setup_sockaddr_in(&gtp_sock->sockaddr.fd2.in, 2152);
		break;
	case AF_INET6:
		gtp_sock->len = sizeof(struct sockaddr_in6);
		setup_sockaddr_in6(&gtp_sock->sockaddr.fd1.in6, 3386);
		setup_sockaddr_in6(&gtp_sock->sockaddr.fd2.in6, 2152);
		if (setsockopt(fd1, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one)) < 0)
			perror("setsockopt IPV6_V6ONLY: ");
		if (setsockopt(fd2, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one)) < 0)
			perror("setsockopt IPV6_V6ONLY: ");
		break;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct gtp_server_sock gtp_sock;
	int ret, sgsn_mode = 0, family;

	if (argc < 3) {
		printf("Usage: %s add <device> <family> [--sgsn]\n", argv[0]);
		printf("       %s del <device>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	if (!strcmp(argv[1], "del")) {
		printf("destroying gtp interface...\n");
		if (gtp_dev_destroy(argv[2]) < 0)
			perror("gtp_dev_destroy");

		return 0;
	} else if (!strcmp(argv[1], "add")) {
		if (argc < 4) {
			printf("Usage: %s add <device> <family> [--sgsn]\n", argv[0]);
			exit(EXIT_FAILURE);
		}

		if (argc == 5 && !strcmp(argv[4], "--sgsn"))
			sgsn_mode = 1;
	}

	if (!strcmp(argv[3], "ip"))
		family = AF_INET;
	else if (!strcmp(argv[3], "ip6"))
		family = AF_INET6;
	else {
		fprintf(stderr, "unsupport family `%s', expecting `ip' or `ip6'\n",
			argv[3]);
		exit(EXIT_FAILURE);
	}

	if (setup_socket(&gtp_sock, family) < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (bind(gtp_sock.fd1, (struct sockaddr *) &gtp_sock.sockaddr.fd1, gtp_sock.len) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	if (bind(gtp_sock.fd2, (struct sockaddr *) &gtp_sock.sockaddr.fd2, gtp_sock.len) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (sgsn_mode)
		ret = gtp_dev_create_sgsn(-1, argv[2], gtp_sock.fd1, gtp_sock.fd2);
	else
		ret = gtp_dev_create(-1, argv[2], gtp_sock.fd1, gtp_sock.fd2);
	if (ret < 0) {
		perror("cannot create GTP device\n");
		exit(EXIT_FAILURE);
	}

	fprintf(stderr, "WARNING: attaching dummy socket descriptors. Keep "
			"this process running for testing purposes.\n");

	while (1) {
		union {
			struct sockaddr_in addr;
			struct sockaddr_in6 addr6;
		} sock;

		ret = recvfrom(gtp_sock.fd1, buf, sizeof(buf), 0,
			       (struct sockaddr *)&sock, &gtp_sock.len);
		printf("received %d bytes via UDP socket\n", ret);
	}

	return 0;
}
