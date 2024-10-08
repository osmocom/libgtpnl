/* External interface functions of the library */

/* (C) 2014 by sysmocom - s.f.m.c. GmbH
 * Author: Pablo Neira Ayuso <pablo@gnumonks.org>
 *
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <netinet/in.h>

#include <libgtpnl/gtp.h>

#include "internal.h"

EXPORT_SYMBOL(gtp_tunnel_alloc);
struct gtp_tunnel *gtp_tunnel_alloc(void)
{
	struct gtp_tunnel *t;

	t = calloc(1, sizeof(struct gtp_tunnel));
	if (!t)
		return NULL;

	t->ifns = -1;
	return t;
}

EXPORT_SYMBOL(gtp_tunnel_free);
void gtp_tunnel_free(struct gtp_tunnel *t)
{
	free(t);
}

EXPORT_SYMBOL(gtp_tunnel_set_ifns);
void gtp_tunnel_set_ifns(struct gtp_tunnel *t, int ifns)
{
	t->ifns = ifns;
	t->flags |= GTP_TUN_IFNS;
}

EXPORT_SYMBOL(gtp_tunnel_set_ifidx);
void gtp_tunnel_set_ifidx(struct gtp_tunnel *t, uint32_t ifidx)
{
	t->ifidx = ifidx;
	t->flags |= GTP_TUN_IFIDX;
}

EXPORT_SYMBOL(gtp_tunnel_set_family);
void gtp_tunnel_set_family(struct gtp_tunnel *t, uint16_t family)
{
	t->ms_addr.family = family;
	t->flags |= GTP_TUN_FAMILY;
}

EXPORT_SYMBOL(gtp_tunnel_set_ms_ip4);
void gtp_tunnel_set_ms_ip4(struct gtp_tunnel *t, struct in_addr *ms_addr)
{
	t->ms_addr.family = AF_INET;
	t->ms_addr.ip4 = *ms_addr;
	t->flags |= GTP_TUN_FAMILY | GTP_TUN_MS_ADDR;
}

EXPORT_SYMBOL(gtp_tunnel_set_sgsn_ip4);
void gtp_tunnel_set_sgsn_ip4(struct gtp_tunnel *t, struct in_addr *sgsn_addr)
{
	t->sgsn_addr.family = AF_INET;
	t->sgsn_addr.ip4 = *sgsn_addr;
	t->flags |= GTP_TUN_SGSN_ADDR;
}

EXPORT_SYMBOL(gtp_tunnel_set_ms_ip6);
void gtp_tunnel_set_ms_ip6(struct gtp_tunnel *t, const struct in6_addr *ms_addr)
{
	t->ms_addr.family = AF_INET6;
	t->ms_addr.ip6 = *ms_addr;
	t->flags |= GTP_TUN_FAMILY | GTP_TUN_MS_ADDR;
}

EXPORT_SYMBOL(gtp_tunnel_set_sgsn_ip6);
void gtp_tunnel_set_sgsn_ip6(struct gtp_tunnel *t, const struct in6_addr *sgsn_addr)
{
	t->sgsn_addr.family = AF_INET6;
	t->sgsn_addr.ip6 = *sgsn_addr;
	t->flags |= GTP_TUN_SGSN_ADDR;
}

EXPORT_SYMBOL(gtp_tunnel_set_version);
void gtp_tunnel_set_version(struct gtp_tunnel *t, uint32_t version)
{
	t->gtp_version = version;
	t->flags |= GTP_TUN_VERSION;
}

EXPORT_SYMBOL(gtp_tunnel_set_tid);
void gtp_tunnel_set_tid(struct gtp_tunnel *t, uint64_t tid)
{
	t->u.v0.tid = tid;
	t->flags |= GTP_TUN_V0_TID;
}

EXPORT_SYMBOL(gtp_tunnel_set_flowid);
void gtp_tunnel_set_flowid(struct gtp_tunnel *t, uint16_t flowid)
{
	t->u.v0.flowid = flowid;
	t->flags |= GTP_TUN_V0_FLOWID;
}

EXPORT_SYMBOL(gtp_tunnel_set_i_tei);
void gtp_tunnel_set_i_tei(struct gtp_tunnel *t, uint32_t i_tei)
{
	t->u.v1.i_tei = i_tei;
	t->flags |= GTP_TUN_V1_I_TEI;
}

EXPORT_SYMBOL(gtp_tunnel_set_o_tei);
void gtp_tunnel_set_o_tei(struct gtp_tunnel *t, uint32_t o_tei)
{
	t->u.v1.o_tei = o_tei;
	t->flags |= GTP_TUN_V1_O_TEI;
}

EXPORT_SYMBOL(gtp_tunnel_get_ifns);
const int gtp_tunnel_get_ifns(struct gtp_tunnel *t)
{
	return t->ifns;
}

EXPORT_SYMBOL(gtp_tunnel_get_ifidx);
const uint32_t gtp_tunnel_get_ifidx(struct gtp_tunnel *t)
{
	return t->ifidx;
}

EXPORT_SYMBOL(gtp_tunnel_get_ms_ip4);
const struct in_addr *gtp_tunnel_get_ms_ip4(struct gtp_tunnel *t)
{
	return &t->ms_addr.ip4;
}

EXPORT_SYMBOL(gtp_tunnel_get_sgsn_ip4);
const struct in_addr *gtp_tunnel_get_sgsn_ip4(struct gtp_tunnel *t)
{
	return &t->sgsn_addr.ip4;
}

EXPORT_SYMBOL(gtp_tunnel_get_version);
int gtp_tunnel_get_version(struct gtp_tunnel *t)
{
	return t->gtp_version;
}

EXPORT_SYMBOL(gtp_tunnel_get_tid);
uint64_t gtp_tunnel_get_tid(struct gtp_tunnel *t)
{
	return t->u.v0.tid;
}

EXPORT_SYMBOL(gtp_tunnel_get_flowid);
uint16_t gtp_tunnel_get_flowid(struct gtp_tunnel *t)
{
	return t->u.v0.flowid;
}

EXPORT_SYMBOL(gtp_tunnel_get_i_tei);
uint32_t gtp_tunnel_get_i_tei(struct gtp_tunnel *t)
{
	return t->u.v1.i_tei;
}

EXPORT_SYMBOL(gtp_tunnel_get_o_tei);
uint32_t gtp_tunnel_get_o_tei(struct gtp_tunnel *t)
{
	return t->u.v1.o_tei;
}
