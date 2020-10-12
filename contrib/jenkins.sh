#!/usr/bin/env bash

set -ex

osmo-clean-workspace.sh

autoreconf --install --force
./configure --enable-sanitize CFLAGS="-Werror" CPPFLAGS="-Werror"
$MAKE $PARALLEL_MAKE
$MAKE $PARALLEL_MAKE distcheck
$MAKE $PARALLEL_MAKE maintainer-clean

osmo-clean-workspace.sh
