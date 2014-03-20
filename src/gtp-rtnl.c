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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

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

static struct mnl_socket *rtnl_open(void)
{
	struct mnl_socket *nl;

	nl = mnl_socket_open(NETLINK_ROUTE);
	if (nl == NULL) {
		perror("mnl_socket_open");
		return NULL;
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		goto err;
	}

	return nl;
err:
	mnl_socket_close(nl);
	return NULL;
}

static int rtnl_talk(struct mnl_socket *nl, struct nlmsghdr *nlh)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	int ret;

	ret = mnl_socket_sendto(nl, nlh, nlh->nlmsg_len);
	if (ret < 0)
		return ret;

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	return mnl_cb_run(buf, ret, nlh->nlmsg_seq, mnl_socket_get_portid(nl),
			  NULL, NULL);
}

static int gtp_dev_talk(struct nlmsghdr *nlh, uint32_t seq)
{
	struct mnl_socket *nl;
	int ret;

	nl = rtnl_open();
	if (nl == NULL)
		return -1;

	ret = rtnl_talk(nl, nlh);

	mnl_socket_close(nl);
	return ret;
}

int gtp_dev_create(const char *gtp_ifname, const char *real_ifname,
		   int fd0, int fd1)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct ifinfomsg *ifm;
	unsigned int seq = time(NULL);
	struct nlattr *nest, *nest2;

	nlh = gtp_put_nlmsg(buf, RTM_NEWLINK,
			    NLM_F_CREATE | NLM_F_EXCL | NLM_F_ACK, seq);
	ifm = mnl_nlmsg_put_extra_header(nlh, sizeof(*ifm));
	ifm->ifi_family = AF_INET;
	ifm->ifi_change |= IFF_UP;
	ifm->ifi_flags |= IFF_UP;

	mnl_attr_put_u32(nlh, IFLA_LINK, if_nametoindex(real_ifname));
	mnl_attr_put_str(nlh, IFLA_IFNAME, gtp_ifname);
	nest = mnl_attr_nest_start(nlh, IFLA_LINKINFO);
	mnl_attr_put_str(nlh, IFLA_INFO_KIND, "gtp");
	nest2 = mnl_attr_nest_start(nlh, IFLA_INFO_DATA);
	mnl_attr_put_u32(nlh, IFLA_GTP_FD0, fd0);
	mnl_attr_put_u32(nlh, IFLA_GTP_FD1, fd1);
	mnl_attr_put_u32(nlh, IFLA_GTP_HASHSIZE, 131072);
	mnl_attr_nest_end(nlh, nest2);
	mnl_attr_nest_end(nlh, nest);

	return gtp_dev_talk(nlh, seq);
}
EXPORT_SYMBOL(gtp_dev_create);

int gtp_dev_destroy(const char *gtp_ifname)
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
	ifm->ifi_index = if_nametoindex(gtp_ifname);

	return gtp_dev_talk(nlh, seq);
}
EXPORT_SYMBOL(gtp_dev_destroy);

int gtp_dev_config(const char *ifname, struct in_addr *dst, uint32_t prefix)
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct rtmsg *rtm;
	int iface, ret;

	iface = if_nametoindex(ifname);
	if (iface == 0) {
		perror("if_nametoindex");
		return -1;
	}

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type	= RTM_NEWROUTE;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_ACK;
	nlh->nlmsg_seq = time(NULL);

	rtm = mnl_nlmsg_put_extra_header(nlh, sizeof(struct rtmsg));
	rtm->rtm_family = AF_INET;
	rtm->rtm_dst_len = prefix;
	rtm->rtm_src_len = 0;
	rtm->rtm_tos = 0;
	rtm->rtm_protocol = RTPROT_STATIC;
	rtm->rtm_table = RT_TABLE_MAIN;
	rtm->rtm_type = RTN_UNICAST;
	rtm->rtm_scope = RT_SCOPE_UNIVERSE;
	rtm->rtm_flags = 0;

	mnl_attr_put_u32(nlh, RTA_DST, dst->s_addr);
	mnl_attr_put_u32(nlh, RTA_OIF, iface);

	nl = rtnl_open();
	if (nl == NULL)
		return -1;

	ret = rtnl_talk(nl, nlh);

	mnl_socket_close(nl);

	return ret;
}
EXPORT_SYMBOL(gtp_dev_config);
