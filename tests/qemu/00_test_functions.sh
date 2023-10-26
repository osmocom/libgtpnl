#!/bin/sh -ex

# Use ip from iproute2 instead of busybox ip, because iproute2's version has
# "ip netns" implemented. Calling /bin/ip explicitly is needed here, otherwise
# busybox sh will use busybox ip, regardless of PATH.
alias ip="/bin/ip"
alias ggsn_side="ip netns exec ggsn_side"

# MS - SGSN -gtp- GGSN - WEBSERVER
tunnel_start() {
	test -n "$MS"
	test -n "$MS_PREFLEN"
	test -n "$SGSN_GGSN_PROTO"
	test -n "$SGSN"
	test -n "$SGSN_PREFLEN"
	test -n "$GGSN"
	test -n "$WEBSERVER"

	ip netns add ggsn_side

	# SGSN side: prepare veth_sgsn (SGSN), lo (MS)
	ip link add veth_sgsn type veth peer name veth_ggsn
	ip addr add "$SGSN"/"$SGSN_PREFLEN" dev veth_sgsn
	ip link set veth_sgsn up
	ip addr add "$MS"/"$MS_PREFLEN" dev lo
	ip link set lo up

	# SGSN side: prepare gtp-tunnel
	gtp-link add gtp_sgsn "$SGSN_GGSN_PROTO" --sgsn &
	sleep 1
	gtp-tunnel add gtp_sgsn v1 200 100 "$MS" "$GGSN"
	ip route add "$WEBSERVER"/"$MS_PREFLEN" dev gtp_sgsn

	# GGSN side: prepare veth_ggsn (GGSN), lo (WEBSERVER)
	ip link set veth_ggsn netns ggsn_side
	ggsn_side ip addr add "$GGSN"/"$SGSN_PREFLEN" dev veth_ggsn
	ggsn_side ip link set veth_ggsn up
	ggsn_side ip addr add "$WEBSERVER"/"$MS_PREFLEN" dev lo
	ggsn_side ip link set lo up

	# GGSN side: prepare gtp-tunnel
	ggsn_side gtp-link add gtp_ggsn "$SGSN_GGSN_PROTO" &
	sleep 1
	ggsn_side gtp-tunnel add gtp_ggsn v1 100 200 "$MS" "$SGSN"
	ggsn_side ip route add "$MS"/"$MS_PREFLEN" dev gtp_ggsn

	# List tunnel from both sides
	gtp-tunnel list
	ggsn_side gtp-tunnel list
}

tunnel_ping() {
	ip addr show
	ping -c 1 "$WEBSERVER"
	ggsn_side ping -c 1 "$MS"
}

tunnel_stop() {
	killall gtp-link

	ip addr del "$MS"/"$MS_PREFLEN" dev lo
	ip link set veth_sgsn down

	if [ "$SGSN_GGSN_PROTO" == "ip" ]; then  # FIXME: doesn't work with ip6
		ip addr del "$SGSN"/"$SGSN_PREFLEN" dev veth_sgsn
	fi

	ip link del veth_sgsn
	ip route del "$WEBSERVER"/"$MS_PREFLEN" dev gtp_sgsn
	gtp-tunnel delete gtp_sgsn v1 200 "$SGSN_GGSN_PROTO"
	gtp-link del gtp_sgsn
	ggsn_side gtp-tunnel delete gtp_ggsn v1 100 "$SGSN_GGSN_PROTO"
	ggsn_side gtp-link del gtp_ggsn
	ip netns del ggsn_side
}
