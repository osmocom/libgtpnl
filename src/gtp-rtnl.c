#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <libmnl/libmnl.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

#include <libgtpnl/gtpnl.h>

#include <linux/gtp_nl.h>

#include "internal.h"

static struct nlmsghdr *
gtp_put_nlmsg(char *buf, uint16_t type, uint16_t nl_flags, uint32_t seq)
{
	struct nlmsghdr *nlh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= type;
	nlh->nlmsg_flags = NLM_F_REQUEST | nl_flags;
	nlh->nlmsg_seq = seq;

	return nlh;
}

static int gtp_dev_talk(struct nlmsghdr *nlh, uint32_t seq)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL) {
		perror("mnl_socket_open");
		return -1;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		goto err;
	}

	mnl_nlmsg_fprintf(stdout, nlh, nlh->nlmsg_len,
			  sizeof(struct ifinfomsg));

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		goto err;
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1) {
		perror("read");
		goto err;
	}

	ret = mnl_cb_run(buf, ret, seq, mnl_socket_get_portid(nl), NULL, NULL);
	if (ret == -1){
		perror("callback");
		goto err;
	}

	mnl_socket_close(nl);
	return 0;
err:
	mnl_socket_close(nl);
	return -1;
}

int gtp_dev_create(const char *ifname)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	unsigned int seq = time(NULL);
	struct nlattr *nest, *nest2;

	nlh = gtp_put_nlmsg(buf, RTM_NEWLINK, NLM_F_CREATE | NLM_F_ACK, seq);
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_INET;
	ifm->ifi_change |= IFF_UP;
	ifm->ifi_flags |= IFF_UP;

	mnl_attr_put_u32(nlh, IFLA_LINK, if_nametoindex(ifname));
	nest = mnl_attr_nest_start(nlh, IFLA_LINKINFO);
	mnl_attr_put_str(nlh, IFLA_INFO_KIND, "gtp");
	nest2 = mnl_attr_nest_start(nlh, IFLA_INFO_DATA);
	mnl_attr_put_u32(nlh, IFLA_GTP_LOCAL_ADDR_IPV4, 0);
	mnl_attr_put_u32(nlh, IFLA_GTP_HASHSIZE, 131072);
	mnl_attr_nest_end(nlh, nest2);
	mnl_attr_nest_end(nlh, nest);

	return gtp_dev_talk(nlh, seq);
}
EXPORT_SYMBOL(gtp_dev_create);

int gtp_dev_destroy(const char *ifname)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	unsigned int seq = time(NULL);

	nlh = gtp_put_nlmsg(buf, RTM_DELLINK, NLM_F_ACK, seq);
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_INET;
	ifm->ifi_change |= IFF_UP;
	ifm->ifi_flags &= ~IFF_UP;
	ifm->ifi_index = if_nametoindex(ifname);

	return gtp_dev_talk(nlh, seq);
}
EXPORT_SYMBOL(gtp_dev_destroy);
