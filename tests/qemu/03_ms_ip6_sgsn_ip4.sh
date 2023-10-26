#!/bin/sh -ex
. /tests/00_test_functions.sh

MS="fd00::1"
MS_PREFLEN="7"
SGSN="172.0.0.1"
SGSN_PREFLEN="24"
GGSN="172.0.0.2"
WEBSERVER="fd00::2"

tunnel_start
tunnel_ping
tunnel_stop
