# QEMU tests for libgtpnl

The tests simulate how a GGSN would use libgtpnl, to set up a GTP tunnel
between SGSN and GGSN, so a MS on the SGSN side can talk to a webserver on the
GGSN side.

## Running the tests

```
$ autoreconf -fi
$ ./configure --enable-qemu-tests
$ make
$ make -C tests qemu-download-kernel  # or build your own, see below
$ make check
```

## Building your own kernel

Clone a kernel tree, then:
```
$ make defconfig
$ make menuconfig
```

Set the following options:
```
CONFIG_GTP=y
CONFIG_NET_NS=y
CONFIG_VETH=y
```

Build the kernel and copy it to the tests dir:
```
$ make -j$(nproc)
$ cp arch/x86/boot/bzImage /path/to/libgtpnl/tests/qemu/_linux
```
