#!/usr/bin/env bash

set -ex

osmo-clean-workspace.sh

autoreconf --install --force
./configure --enable-sanitize CFLAGS="-Werror" CPPFLAGS="-Werror"
$MAKE $PARALLEL_MAKE
$MAKE distcheck

osmo-clean-workspace.sh
