include $(top_srcdir)/Make_global.am

@RELMAKE@

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = \
	  include \
	  src \
	  tests \
	  tools \
	  $(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgtpnl.pc

${pkgconfig_DATA}: ${top_builddir}/config.status

@RELMAKE@
