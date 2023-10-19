dnl Process this file with autoconf to create configure.

AC_INIT([libgtpnl],
	m4_esyscmd([./git-version-gen .tarball-version]),
	[osmocom-net-gprs@lists.osmocom.org])
AC_CONFIG_AUX_DIR([build-aux])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign tar-pax no-dist-gzip dist-bzip2 1.6 subdir-objects])

CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=64 -D_REENTRANT"
CFLAGS="$CFLAGS -Wall -Waggregate-return -Wmissing-declarations \
	-Wmissing-prototypes -Wshadow -Wstrict-prototypes \
	-Wformat=2 -pipe"

dnl include release helper
RELMAKE='-include osmo-release.mk'
AC_SUBST([RELMAKE])

CFLAGS="$CFLAGS -std=gnu11"

dnl kernel style compile messages
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl Dependencies
PKG_CHECK_MODULES([LIBMNL], [libmnl >= 1.0.0])

dnl include release helper
RELMAKE='-include osmo-release.mk'
AC_SUBST([RELMAKE])

AC_PROG_CC
AM_PROG_CC_C_O
AC_EXEEXT
AC_DISABLE_STATIC
LT_INIT
CHECK_GCC_FVISIBILITY
case "$host" in
*-*-linux* | *-*-uclinux*) ;;
*) AC_MSG_ERROR([Linux only, dude!]);;
esac

AC_ARG_ENABLE(sanitize,
	[AS_HELP_STRING(
		[--enable-sanitize],
		[Compile with address sanitizer enabled],
	)],
	[sanitize=$enableval], [sanitize="no"])
if test x"$sanitize" = x"yes"
then
	CFLAGS="$CFLAGS -fsanitize=address -fsanitize=undefined"
	CPPFLAGS="$CPPFLAGS -fsanitize=address -fsanitize=undefined"
fi

AC_ARG_ENABLE(werror,
	[AS_HELP_STRING(
		[--enable-werror],
		[Turn all compiler warnings into errors, with exceptions:
		 a) deprecation (allow upstream to mark deprecation without breaking builds);
		 b) "#warning" pragmas (allow to remind ourselves of errors without breaking builds)
		]
	)],
	[werror=$enableval], [werror="no"])
if test x"$werror" = x"yes"
then
	WERROR_FLAGS="-Werror"
	WERROR_FLAGS+=" -Werror=implicit-int -Werror=int-conversion -Werror=old-style-definition"
	WERROR_FLAGS+=" -Wno-error=deprecated -Wno-error=deprecated-declarations"
	WERROR_FLAGS+=" -Wno-error=cpp" # "#warning"
	CFLAGS="$CFLAGS $WERROR_FLAGS"
	CPPFLAGS="$CPPFLAGS $WERROR_FLAGS"
fi

AC_SUBST([CPPFLAGS])
AC_SUBST([CFLAGS])
AC_CONFIG_FILES([Makefile src/Makefile include/Makefile include/libgtpnl/Makefile include/linux/Makefile tools/Makefile libgtpnl.pc])
AC_OUTPUT(
	contrib/libgtpnl.spec)
