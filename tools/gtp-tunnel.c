#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <libmnl/libmnl.h>
#include <linux/genetlink.h>

#include <linux/gtp_nl.h>
#include <libgtpnl/gtpnl.h>

static int
add_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	uint32_t gtp_ifidx;
	struct in_addr ms, sgsn;
	struct nlmsghdr *nlh;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t seq = time(NULL), gtp_version;

	if (argc != 7) {
		printf("%s add <gtp device> <v0|v1> <tid> <ms-addr> <sgsn-addr>\n",
			argv[0]);
		return EXIT_FAILURE;
	}
	gtp_ifidx = if_nametoindex(argv[2]);
	if (gtp_ifidx == 0) {
		fprintf(stderr, "wrong GTP interface %s\n", argv[2]);
		return EXIT_FAILURE;
	}

	if (inet_aton(argv[5], &ms) < 0) {
		perror("bad address for ms");
		exit(EXIT_FAILURE);
	}

	if (inet_aton(argv[6], &sgsn) < 0) {
		perror("bad address for sgsn");
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[3], "v0") == 0)
		gtp_version = GTP_V0;
	else if (strcmp(argv[3], "v1") == 0)
		gtp_version = GTP_V1;
	else {
		fprintf(stderr, "wrong GTP version %s, use v0 or v1\n",
			argv[3]);
		return EXIT_FAILURE;
	}

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_EXCL | NLM_F_ACK, ++seq,
				   GTP_CMD_TUNNEL_NEW);
	gtp_build_payload(nlh, atoi(argv[4]), gtp_ifidx, sgsn.s_addr,
			  ms.s_addr, gtp_version);

	if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0)
		perror("genl_socket_talk");

	return 0;
}

static int
del_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
{
	uint32_t gtp_ifidx;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t seq = time(NULL);

	if (argc != 5) {
		printf("%s add <gtp device> <version> <tid>\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	gtp_ifidx = if_nametoindex(argv[2]);

	nlh = genl_nlmsg_build_hdr(buf, genl_id, NLM_F_ACK, ++seq,
				   GTP_CMD_TUNNEL_DELETE);
	gtp_build_payload(nlh, atoi(argv[4]), gtp_ifidx, 0, 0, atoi(argv[3]));

	if (genl_socket_talk(nl, nlh, seq, NULL, NULL) < 0)
		perror("genl_socket_talk");

	return 0;
}

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
	struct gtp_pdp pdp;
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
	printf("tid %llu ms_addr %s ", pdp.tid, inet_ntoa(pdp.sgsn_addr));
	printf("sgsn_addr %s\n", inet_ntoa(pdp.ms_addr));

	return MNL_CB_OK;
}

static int
list_tunnel(int argc, char *argv[], int genl_id, struct mnl_socket *nl)
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

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	unsigned int portid;
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
