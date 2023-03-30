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

struct gtp_tunnel {
	uint8_t		family;
	int             ifns;
	uint32_t	ifidx;
	union {
		struct {
			struct in_addr	ms_addr;
			struct in_addr	sgsn_addr;
		} ip;
		struct {
			struct in6_addr	ms_addr;
			struct in6_addr	sgsn_addr;
		} ip6;
	};
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
