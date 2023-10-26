#!/bin/sh -e
DIR="$(cd "$(dirname "$0")" && pwd)"

if [ -e /dev/kvm ]; then
	MACHINE_ARG="-machine pc,accel=kvm"
else
	echo "WARNING: /dev/kvm not found, emulation will be slower"
	MACHINE_ARG="-machine pc"
fi

if ! [ -e "$DIR"/_linux ]; then
	echo "ERROR: linux kernel not found: $DIR/_linux"
	echo "Put a kernel there, either download it from the Osmocom jenkins:"
	echo "$ make -C tests qemu-download-kernel"
	echo "(FIXME: isn't built with required config options yet)"
	echo
	echo "Or build your own kernel. Make sure to set:"
	echo "  CONFIG_GTP=y"
	echo "  CONFIG_NETNS=y"
	echo "  CONFIG_VETH=y"
	exit 1
fi

KERNEL_CMDLINE="root=/dev/ram0 console=ttyS0 panic=-1 init=/init"

set -x
qemu-system-x86_64 \
	$MACHINE_ARG \
	-smp 1 \
	-m 512M \
	-no-user-config -nodefaults -display none \
	-gdb unix:"$DIR"/_gdb.pipe,server=on,wait=off \
	-no-reboot \
	-kernel "$DIR"/_linux \
	-initrd "$DIR"/_initrd.gz \
	-append "${KERNEL_CMDLINE}" \
	-serial stdio \
	-chardev socket,id=charserial1,path="$DIR"/_gdb-serial.pipe,server=on,wait=off \
	-device isa-serial,chardev=charserial1,id=serial1 \
	2>&1 | tee "$DIR/_output"

set +x
if grep -q "QEMU_TEST_SUCCESSFUL" "$DIR/_output"; then
	echo
	echo "QEMU tests: successful"
	echo
else
	echo
	echo "QEMU tests: failed"
	echo
	exit 1
fi
