#!/bin/sh -ex
. /tests/00_test_functions.sh

MS_PROTO="ip6"
MS="fd00::"
MS_PREFLEN="64"
SGSN_GGSN_PROTO="ip"
SGSN="172.0.0.1"
SGSN_PREFLEN="24"
GGSN="172.0.0.2"
WEBSERVER="fe00::2"

tunnel_start
tunnel_ping
tunnel_stop
