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

enum {
	GTP_TUN_IFNS		= (1 << 0),
	GTP_TUN_IFIDX		= (1 << 1),
	GTP_TUN_FAMILY		= (1 << 2),
	GTP_TUN_MS_ADDR		= (1 << 3),
	GTP_TUN_SGSN_ADDR	= (1 << 4),
	GTP_TUN_VERSION		= (1 << 5),
	GTP_TUN_V0_TID		= (1 << 6),
	GTP_TUN_V0_FLOWID	= (1 << 7),
	GTP_TUN_V1_I_TEI	= (1 << 8),
	GTP_TUN_V1_O_TEI	= (1 << 9),
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
	uint32_t	flags;
};

#endif
