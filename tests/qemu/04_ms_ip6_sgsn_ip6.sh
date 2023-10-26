#!/bin/sh -ex
. /tests/00_test_functions.sh

MS="fc00::1"
MS_PREFLEN="7"
SGSN="fd00::1"
SGSN_PREFLEN="7"
GGSN="fd00::2"
WEBSERVER="fc00::2"

tunnel_start
tunnel_ping
tunnel_stop
