#ifndef _LIBGTPNL_H_
#define _LIBGTPNL_H_

#include <stdint.h>

struct mnl_socket;
struct nlmsghdr;

struct mnl_socket *genl_socket_open(void);
struct nlmsghdr *genl_nlmsg_build_hdr(char *buf, uint16_t type, uint16_t flags,
				      uint32_t seq, uint8_t cmd);
int genl_socket_talk(struct mnl_socket *nl, struct nlmsghdr *nlh, uint32_t seq,
		     int (*cb)(const struct nlmsghdr *nlh, void *data),
		     void *data);
int genl_lookup_family(struct mnl_socket *nl, const char *family);

int gtp_dev_create(const char *ifname);
int gtp_dev_destroy(const char *ifname);

struct gtp_tunnel;

int gtp_add_tunnel(int genl_id, struct mnl_socket *nl, struct gtp_tunnel *t);
int gtp_del_tunnel(int genl_id, struct mnl_socket *nl, struct gtp_tunnel *t);
int gtp_list_tunnel(int genl_id, struct mnl_socket *nl);

#endif
