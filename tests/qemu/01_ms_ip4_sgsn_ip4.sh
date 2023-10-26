#!/bin/sh -ex
. /tests/00_test_functions.sh

MS="172.99.0.1"
MS_PREFLEN="32"
SGSN="172.0.0.1"
SGSN_PREFLEN="24"
GGSN="172.0.0.2"
WEBSERVER="172.99.0.2"

tunnel_start
tunnel_ping
tunnel_stop
