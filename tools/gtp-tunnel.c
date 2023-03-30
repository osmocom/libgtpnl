/* Command line utility to create GTP tunnels (PDP contexts) */

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
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <inttypes.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include <linux/gtp.h>
#include <linux/if_link.h>
#include <libgtpnl/gtp.h>
#include <libgtpnl/gtpnl.h>

static void add_usage(const char *name)
{
	printf("%s add <gtp device> <v0> <tid> <family> <ms-addr> <sgsn-addr>\n",
	       name);
	printf("%s add <gtp device> <v1> <i_tei> <o_tei> <family> <ms-addr> <sgsn-addr>\n",
	       name);
}

static int
add_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	union {
		struct in_addr addr;
		struct in6_addr addr6;
	} ms;
	union {
		struct in_addr addr;
		struct in6_addr addr6;
	} sgsn;
	struct gtp_tunnel *t;
	uint32_t gtp_version;
	uint32_t gtp_ifidx;
	int optidx, family;

	if (argc < 8 || argc > 9) {
		add_usage(argv[0]);
		return EXIT_FAILURE;
	}

	t = gtp_tunnel_alloc();
	optidx = 2;

	gtp_ifidx = if_nametoindex(argv[optidx]);
	if (gtp_ifidx == 0) {
		fprintf(stderr, "wrong GTP interface %s\n", argv[optidx]);
		return EXIT_FAILURE;
	}
	gtp_tunnel_set_ifidx(t, gtp_ifidx);

	optidx++;

	if (strcmp(argv[optidx], "v0") == 0)
		gtp_version = GTP_V0;
	else if (strcmp(argv[optidx], "v1") == 0)
		gtp_version = GTP_V1;
	else {
		fprintf(stderr, "wrong GTP version %s, use v0 or v1\n",
			argv[optidx]);
		return EXIT_FAILURE;
	}
	gtp_tunnel_set_version(t, gtp_version);

	optidx++;

	if (gtp_version == GTP_V0)
		gtp_tunnel_set_tid(t, atoi(argv[optidx++]));
	else if (gtp_version == GTP_V1) {
		gtp_tunnel_set_i_tei(t, atoi(argv[optidx++]));
		gtp_tunnel_set_o_tei(t, atoi(argv[optidx++]));
	}

	if (!strcmp(argv[optidx], "ip")) {
		family = AF_INET;
	} else if (!strcmp(argv[optidx], "ip6")) {
		family = AF_INET6;
	} else {
		fprintf(stderr, "wrong family `%s', expecting `ip' or `ip6'\n", argv[optidx]);
		return EXIT_FAILURE;
	}

	gtp_tunnel_set_family(t, family);

	optidx++;

	if (inet_pton(family, argv[optidx++], &ms) <= 0) {
		fprintf(stderr, "bad address for ms\n");
		exit(EXIT_FAILURE);
	}

	switch (family) {
	case AF_INET:
		gtp_tunnel_set_ms_ip4(t, &ms.addr);
		break;
	case AF_INET6:
		gtp_tunnel_set_ms_ip6(t, &ms.addr6);
		break;
	}

	if (inet_pton(family, argv[optidx++], &sgsn) <= 0) {
		fprintf(stderr, "bad address for sgsn\n");
		exit(EXIT_FAILURE);
	}

	switch (family) {
	case AF_INET:
		gtp_tunnel_set_sgsn_ip4(t, &sgsn.addr);
		break;
	case AF_INET6:
		gtp_tunnel_set_sgsn_ip6(t, &sgsn.addr6);
		break;
	}

	gtp_add_tunnel(genl_id, nl, t);

	gtp_tunnel_free(t);
	return 0;
}

static int
del_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	struct gtp_tunnel *t;
	uint32_t gtp_ifidx;

	if (argc != 5) {
		printf("%s del <gtp device> <version> <tid>\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	t = gtp_tunnel_alloc();

	gtp_ifidx = if_nametoindex(argv[2]);
	if (gtp_ifidx == 0) {
		fprintf(stderr, "wrong GTP interface %s\n", argv[2]);
		gtp_tunnel_free(t);
		return EXIT_FAILURE;
	}
	gtp_tunnel_set_ifidx(t, gtp_ifidx);

	if (strcmp(argv[3], "v0") == 0) {
		gtp_tunnel_set_version(t, GTP_V0);
		gtp_tunnel_set_tid(t, atoi(argv[4]));
	} else if (strcmp(argv[3], "v1") == 0) {
		gtp_tunnel_set_version(t, GTP_V1);
		gtp_tunnel_set_i_tei(t, atoi(argv[4]));
	} else {
		fprintf(stderr, "wrong GTP version %s, use v0 or v1\n",
			argv[3]);
		gtp_tunnel_free(t);
		return EXIT_FAILURE;
	}

	gtp_del_tunnel(genl_id, nl, t);

	gtp_tunnel_free(t);
	return 0;
}

struct gtp_pdp {
	uint32_t	version;
	union {
		struct {
			uint64_t tid;
		} v0;
		struct {
			uint32_t i_tei;
			uint32_t o_tei;
		} v1;
	} u;
	struct in_addr	sgsn_addr;
	struct in_addr	ms_addr;
};

static int
list_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	return gtp_list_tunnel(genl_id, nl);
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	int32_t genl_id;
	int ret;

	if (argc < 2) {
		printf("%s <add|delete|list> [<options,...>]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nl = genl_socket_open();
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	genl_id = genl_lookup_family(nl, "gtp");
	if (genl_id < 0) {
		printf("not found gtp genl family\n");
		exit(EXIT_FAILURE);
	}

	if (strncmp(argv[1], "add", strlen(argv[1])) == 0)
		ret = add_tunnel(argc, argv, genl_id, nl);
	else if (strncmp(argv[1], "delete", strlen(argv[1])) == 0)
		ret = del_tunnel(argc, argv, genl_id, nl);
	else if (strncmp(argv[1], "list", strlen(argv[1])) == 0)
		ret = list_tunnel(argc, argv, genl_id, nl);
	else {
		printf("Unknown command `%s'\n", argv[1]);
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return ret;
}
