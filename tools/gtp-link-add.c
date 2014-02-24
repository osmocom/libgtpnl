#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <linux/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include <linux/gtp_nl.h>

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	int ret;
	unsigned int seq, portid, change = 0, flags = 0;
	struct nlattr *nest, *nest2;

	if (argc != 2) {
		printf("Usage: %s [ifname]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= RTM_NEWLINK;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL |NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_INET;
	ifm->ifi_change |= IFF_UP;
	ifm->ifi_flags |= IFF_UP;

	fprintf(stderr, "WARNING: attaching dummy socket descriptors. Use "
			"this command for testing purposes only.\n"):
	int fd1 = socket(AF_INET, SOCK_DGRAM, 0);
	int fd2 = socket(AF_INET, SOCK_DGRAM, 0);

	mnl_attr_put_u32(nlh, IFLA_LINK, if_nametoindex(argv[1]));
	mnl_attr_put_str(nlh, IFLA_IFNAME, "gtp0");
	nest = mnl_attr_nest_start(nlh, IFLA_LINKINFO);
	mnl_attr_put_str(nlh, IFLA_INFO_KIND, "gtp");
	nest2 = mnl_attr_nest_start(nlh, IFLA_INFO_DATA);
	mnl_attr_put_u32(nlh, IFLA_GTP_FD0, fd1);
	mnl_attr_put_u32(nlh, IFLA_GTP_FD1, fd2);
	mnl_attr_put_u32(nlh, IFLA_GTP_HASHSIZE, 131072);
	mnl_attr_nest_end(nlh, nest2);
	mnl_attr_nest_end(nlh, nest);

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len,
			  sizeof(struct ifinfomsg));

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	ret = mnl_cb_run(buf, ret, seq, portid, NULL, NULL);
	if (ret == -1){
		perror("callback");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return 0;
}
