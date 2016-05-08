/* Command line utility to create GTP tunnels (PDP contexts) */

/* (C) 2014 by sysmocom - s.f.m.c. GmbH
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
	printf("%s add <gtp device> <v0> <tid> <ms-addr> <sgsn-addr>\n",
	       name);
	printf("%s add <gtp device> <v1> <i_tei> <o_tei> <ms-addr> <sgsn-addr>\n",
	       name);
}

static int
add_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	struct gtp_tunnel *t;
	uint32_t gtp_ifidx;
	struct in_addr ms, sgsn;
	uint32_t gtp_version;
	int optidx;

	if (argc < 7 || argc > 8) {
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

	if (gtp_version == GTP_V0)
		gtp_tunnel_set_tid(t, atoi(argv[optidx++]));
	else if (gtp_version == GTP_V1) {
		gtp_tunnel_set_i_tei(t, atoi(argv[optidx++]));
		gtp_tunnel_set_o_tei(t, atoi(argv[optidx++]));
	}

	if (inet_aton(argv[optidx++], &ms) < 0) {
		perror("bad address for ms");
		exit(EXIT_FAILURE);
	}
	gtp_tunnel_set_ms_ip4(t, &ms);

	if (inet_aton(argv[optidx++], &sgsn) < 0) {
		perror("bad address for sgsn");
		exit(EXIT_FAILURE);
	}
	gtp_tunnel_set_sgsn_ip4(t, &sgsn);

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
		printf("%s add <gtp device> <version> <tid>\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	t = gtp_tunnel_alloc();

	gtp_ifidx = if_nametoindex(argv[2]);
	if (gtp_ifidx == 0) {
		fprintf(stderr, "wrong GTP interface %s\n", argv[2]);
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

static int genl_gtp_validate_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, CTRL_ATTR_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case GTPA_TID:
		if (mnl_attr_validate(attr, MNL_TYPE_U64) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case GTPA_I_TEI:
	case GTPA_O_TEI:
	case GTPA_SGSN_ADDRESS:
	case GTPA_MS_ADDRESS:
	case GTPA_VERSION:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	default:
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int genl_gtp_attr_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[GTPA_MAX + 1] = {};
	struct gtp_pdp pdp = {};
	struct genlmsghdr *genl;

	mnl_attr_parse(nlh, sizeof(*genl), genl_gtp_validate_cb, tb);
	if (tb[GTPA_TID])
		pdp.u.v0.tid = mnl_attr_get_u64(tb[GTPA_TID]);
	if (tb[GTPA_I_TEI])
		pdp.u.v1.i_tei = mnl_attr_get_u32(tb[GTPA_I_TEI]);
	if (tb[GTPA_O_TEI])
		pdp.u.v1.o_tei = mnl_attr_get_u32(tb[GTPA_O_TEI]);
	if (tb[GTPA_SGSN_ADDRESS]) {
		pdp.sgsn_addr.s_addr =
			mnl_attr_get_u32(tb[GTPA_SGSN_ADDRESS]);
	}
	if (tb[GTPA_MS_ADDRESS]) {
		pdp.ms_addr.s_addr = mnl_attr_get_u32(tb[GTPA_MS_ADDRESS]);
	}
	if (tb[GTPA_VERSION]) {
		pdp.version = mnl_attr_get_u32(tb[GTPA_VERSION]);
	}

	printf("version %u ", pdp.version);
	if (pdp.version == GTP_V0)
		printf("tid %"PRIu64" ms_addr %s ",
		       pdp.u.v0.tid, inet_ntoa(pdp.ms_addr));
	else if (pdp.version == GTP_V1)
		printf("tei %u/%u ms_addr %s ", pdp.u.v1.i_tei,
		       pdp.u.v1.o_tei, inet_ntoa(pdp.ms_addr));
	printf("sgsn_addr %s\n", inet_ntoa(pdp.sgsn_addr));

	return MNL_CB_OK;
}

static int
list_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t seq = time(NULL);

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_DUMP, 0,
				   GTP_CMD_GETPDP);

	if (genl_socket_talk(nl, nlh, seq, genl_gtp_attr_cb, NULL) < 0) {
		perror("genl_socket_talk");
		return 0;
	}

	return 0;
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
