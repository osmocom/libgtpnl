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
