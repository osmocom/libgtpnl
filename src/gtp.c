#include <stdlib.h>
#include <netinet/in.h>

#include <libgtpnl/gtp.h>

#include "internal.h"

struct gtp_tunnel *gtp_tunnel_alloc(void)
{
	return calloc(1, sizeof(struct gtp_tunnel));
}
EXPORT_SYMBOL(gtp_tunnel_alloc);

void gtp_tunnel_free(struct gtp_tunnel *t)
{
	free(t);
}
EXPORT_SYMBOL(gtp_tunnel_free);

void gtp_tunnel_set_ifidx(struct gtp_tunnel *t, uint32_t ifidx)
{
	t->ifidx = ifidx;
}
EXPORT_SYMBOL(gtp_tunnel_set_ifidx);

void gtp_tunnel_set_ms_ip4(struct gtp_tunnel *t, struct in_addr *ms_addr)
{
	t->ms_addr = *ms_addr;
}
EXPORT_SYMBOL(gtp_tunnel_set_ms_ip4);

void gtp_tunnel_set_sgsn_ip4(struct gtp_tunnel *t, struct in_addr *sgsn_addr)
{
	t->sgsn_addr = *sgsn_addr;
}
EXPORT_SYMBOL(gtp_tunnel_set_sgsn_ip4);

void gtp_tunnel_set_version(struct gtp_tunnel *t, uint32_t version)
{
	t->gtp_version = version;
}
EXPORT_SYMBOL(gtp_tunnel_set_version);

void gtp_tunnel_set_tid(struct gtp_tunnel *t, uint64_t tid)
{
	t->tid = tid;
}
EXPORT_SYMBOL(gtp_tunnel_set_tid);

const uint32_t gtp_tunnel_get_ifidx(struct gtp_tunnel *t)
{
	return t->ifidx;
}
EXPORT_SYMBOL(gtp_tunnel_get_ifidx);

const struct in_addr *gtp_tunnel_get_ms_ip4(struct gtp_tunnel *t)
{
	return &t->ms_addr;
}
EXPORT_SYMBOL(gtp_tunnel_get_ms_ip4);

const struct in_addr *gtp_tunnel_get_sgsn_ip4(struct gtp_tunnel *t)
{
	return &t->sgsn_addr;
}
EXPORT_SYMBOL(gtp_tunnel_get_sgsn_ip4);

int gtp_tunnel_get_version(struct gtp_tunnel *t)
{
	return t->gtp_version;
}
EXPORT_SYMBOL(gtp_tunnel_get_version);

uint64_t gtp_tunnel_get_tid(struct gtp_tunnel *t)
{
	return t->tid;
}
EXPORT_SYMBOL(gtp_tunnel_get_tid);
