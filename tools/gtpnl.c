/* Helper functions */

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

#include <stdint.h>
#include <libmnl/libmnl.h>
#include <linux/gtp_nl.h>

void gtp_build_payload(struct nlmsghdr *nlh, uint64_t tid, uint32_t ifidx,
		       uint32_t sgsn_addr, uint32_t ms_addr, uint32_t version)
{
	mnl_attr_put_u32(nlh, GTPA_VERSION, version);
	mnl_attr_put_u32(nlh, GTPA_LINK, ifidx);
	mnl_attr_put_u32(nlh, GTPA_SGSN_ADDRESS, sgsn_addr);
	mnl_attr_put_u32(nlh, GTPA_MS_ADDRESS, ms_addr);
	mnl_attr_put_u64(nlh, GTPA_TID, tid);
}
