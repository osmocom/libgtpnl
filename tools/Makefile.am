include $(top_srcdir)/Make_global.am

check_PROGRAMS = gtp-link-add		\
		 gtp-tunnel

gtp_link_add_SOURCES = gtp-link-add.c gtpnl.c
gtp_link_add_LDADD = ../src/libgtpnl.la ${LIBMNL_LIBS}

gtp_tunnel_SOURCES = gtp-tunnel.c gtpnl.c
gtp_tunnel_LDADD = ../src/libgtpnl.la ${LIBMNL_LIBS}
