#ifndef INTERNAL_H
#define INTERNAL_H 1

#include "config.h"
#ifdef HAVE_VISIBILITY_HIDDEN
#	define __visible	__attribute__((visibility("default")))
#	define EXPORT_SYMBOL(x)	typeof(x) (x) __visible
#else
#	define EXPORT_SYMBOL
#endif

#include <stdint.h>
#include <netinet/in.h>

struct gtp_addr {
	sa_family_t family;
	union {
		struct in_addr ip4;
		struct in6_addr ip6;
	};
};

struct gtp_tunnel {
	int             ifns;
	uint32_t	ifidx;
	struct gtp_addr ms_addr;
	struct gtp_addr sgsn_addr;
	int		gtp_version;
	union {
		struct {
			uint64_t tid;
			uint16_t flowid;
		} v0;
		struct {
			uint32_t i_tei;
			uint32_t o_tei;
		} v1;
	} u;
};

#endif
