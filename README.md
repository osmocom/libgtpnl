# libgtpnl - netlink library for Linux kernel GTP

In order to control the kernel-side GTP-U plane (`gtp` driver), a
netlink based control interface between GTP-C in userspace and GTP-U in
kernelspace was invented.

The encoding and decoding of these control messages is implemented in
this *libgtpnl* (library for GTP netlink).

libgtpnl is part of the [Osmocom](https://osmocom.org/) Open Source
Mobile Communications project.

## Homepage

The official homepage of the project is
<https://osmocom.org/projects/linux-kernel-gtp-u/wiki/Libgtpnl>

GIT Repository
--------------

You can clone from the official libgtpnl.git repository using

	git clone https://gitea.osmocom.org/cellular-infrastructure/libgtpnl

Visiting the URL in a browser shows a
[web interface](https://gitea.osmocom.org/cellular-infrastructure/libgtpnl).

Mailing List
------------

Discussions related to libgtpnl are happening on the
osmocom-net-gprs@lists.osmocom.org mailing list, please see
<https://lists.osmocom.org/mailman/listinfo/osmocom-net-gprs> for
subscription options and the list archive.

Please observe the [Osmocom Mailing List
Rules](https://osmocom.org/projects/cellular-infrastructure/wiki/Mailing_List_Rules)
when posting.

Contributing
------------

Our coding standards are described at
<https://osmocom.org/projects/cellular-infrastructure/wiki/Coding_standards>

We use a gerrit based patch submission/review process for managing
contributions.  Please see
<https://osmocom.org/projects/cellular-infrastructure/wiki/Gerrit> for
more details

The current patch queue for libgtpnl can be seen at
<https://gerrit.osmocom.org/#/q/project:libgtpnl+status:open>
