check-local:
	$(MAKE) qemu-tests

if ENABLE_QEMU_TESTS
qemu-download-kernel:
	rm -f qemu/_linux
	wget -O qemu/_linux \
		https://jenkins.osmocom.org/jenkins/job/ttcn3-ggsn-test-kernel-latest-net-next/ws/_cache/kernel-test/linux
qemu-tests:
	qemu/initrd-build.sh
	qemu/run-qemu.sh
else
qemu-tests:
	@echo "Not running QEMU tests (determined at configure-time)"
endif
