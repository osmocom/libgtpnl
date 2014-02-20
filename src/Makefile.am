include $(top_srcdir)/Make_global.am
lib_LTLIBRARIES = libgtpnl.la

libgtpnl_la_LDFLAGS = -Wl,--version-script=$(srcdir)/libgtpnl.map -version-info $(LIBVERSION)
libgtpnl_la_SOURCES = genl.c gtp-genl.c gtp-rtnl.c libgtpnl.map
