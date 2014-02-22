#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include <libgtpnl/gtp.h>
#include <libgtpnl/gtpnl.h>

#include <net/if.h>
#include <linux/gtp_nl.h>

#include "internal.h"

static void gtp_build_payload(struct nlmsghdr *nlh, uint64_t tid,
			      uint32_t ifidx, uint32_t sgsn_addr,
			      uint32_t ms_addr, uint32_t version)
{
	mnl_attr_put_u32(nlh, GTPA_VERSION, version);
	mnl_attr_put_u32(nlh, GTPA_LINK, ifidx);
	mnl_attr_put_u32(nlh, GTPA_SGSN_ADDRESS, sgsn_addr);
	mnl_attr_put_u32(nlh, GTPA_MS_ADDRESS, ms_addr);
	mnl_attr_put_u64(nlh, GTPA_TID, tid);
}

int gtp_add_tunnel(int genl_id, struct mnl_socket *nl, struct gtp_tunnel *t)
{
	struct nlmsghdr *nlh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t seq = time(NULL);

	if (t->gtp_version > GTP_V1) {
		fprintf(stderr, "wrong GTP version %u, use v0 or v1\n",
			t->gtp_version);
		return -1;
	}

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_EXCL | NLM_F_ACK, ++seq,
				   GTP_CMD_TUNNEL_NEW);
	gtp_build_payload(nlh, t->tid, t->ifidx, t->sgsn_addr.s_addr,
			  t->ms_addr.s_addr, t->gtp_version);

	if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0)
		perror("genl_socket_talk");

	return 0;
}
EXPORT_SYMBOL(gtp_add_tunnel);

int gtp_del_tunnel(int genl_id, struct mnl_socket *nl, struct gtp_tunnel *t)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t seq = time(NULL);

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_ACK, ++seq,
				   GTP_CMD_TUNNEL_DELETE);
	gtp_build_payload(nlh, t->tid, t->ifidx, 0, 0, t->gtp_version);

	if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0)
		perror("genl_socket_talk");

	return 0;
}
EXPORT_SYMBOL(gtp_del_tunnel);

struct gtp_pdp {
	uint32_t	version;
	uint64_t	tid;
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
		pdp.tid = mnl_attr_get_u64(tb[GTPA_TID]);
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
	printf("tid %"PRIu64" ms_addr %s ", pdp.tid, inet_ntoa(pdp.sgsn_addr));
	printf("sgsn_addr %s\n", inet_ntoa(pdp.ms_addr));

	return MNL_CB_OK;
}

int gtp_list_tunnel(int genl_id, struct mnl_socket *nl)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t seq = time(NULL);

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_DUMP, 0,
				   GTP_CMD_TUNNEL_GET);

	if (genl_socket_talk(nl, nlh, seq, genl_gtp_attr_cb, NULL) < 0) {
		perror("genl_socket_talk");
		return 0;
	}

	return 0;
}
EXPORT_SYMBOL(gtp_list_tunnel);
