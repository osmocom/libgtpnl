#!/bin/sh
RET=0

require_program() {
	if [ -z "$(command -v "$1")" ]; then
		RET=1
		echo "ERROR: missing program: $1"
	fi
}

require_program busybox
require_program cpio
require_program find
require_program gzip
require_program ip
require_program qemu-system-x86_64

exit "$RET"
