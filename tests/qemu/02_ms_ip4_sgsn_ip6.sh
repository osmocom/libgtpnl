#!/bin/sh -ex
. /tests/00_test_functions.sh

MS="172.99.0.1"
MS_PREFLEN="32"
SGSN_GGSN_PROTO="ip6"
SGSN="fd00::1"
SGSN_PREFLEN="7"
GGSN="fd00::2"
WEBSERVER="172.99.0.2"

tunnel_start
tunnel_ping
tunnel_stop
