#!/bin/busybox sh
echo "Running initrd-init.sh"
set -x

run_test() {
	echo
	echo "QEMU test: $1"
	echo
	if ! sh -ex "/tests/$1"; then
		poweroff -f
	fi
}

export HOME=/root
export LD_LIBRARY_PATH=/usr/local/lib
export PATH=/usr/local/bin:/usr/bin:/bin:/sbin:/usr/local/sbin:/usr/sbin
export TERM=screen

/bin/busybox --install -s
hostname qemu
mount -t proc proc /proc
mount -t sysfs sys /sys
mknod /dev/null c 1 3
. /cmd.sh
set +x

# Run all tests
run_test 01_ms_ip4_sgsn_ip4.sh
run_test 02_ms_ip4_sgsn_ip6.sh
run_test 03_ms_ip6_sgsn_ip4.sh
run_test 04_ms_ip6_sgsn_ip6.sh
run_test 05_ms_ip46_sgsn_ip4.sh

# Success (run-qemu.sh checks for this line)
echo "QEMU_TEST_SUCCESSFUL"
poweroff -f
