#!/bin/sh -ex
. /tests/00_test_functions.sh

MS_PROTO="ip6"
MS="fc00::"
MS_PREFLEN="64"
SGSN_GGSN_PROTO="ip6"
SGSN="fd00::1"
SGSN_PREFLEN="7"
GGSN="fd00::2"
WEBSERVER="fe00::2"

tunnel_start
tunnel_ping
tunnel_stop
