AC_INIT([flood.c], [1.0])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO

CFLAGS="-Wall -O3 -fPIC -march=native -mtune=native -g -flto"
AC_SUBST(CFLAGS)

AC_CHECK_LIB([websockets], [lws_create_context],
  [AC_DEFINE([HAVE_LIBWEBSOCKETS], [1], [Define if you have the libwebsockets library])],
  [AC_MSG_ERROR([libwebsockets not found])])

AC_CHECK_HEADERS([ \
  libwebsockets.h \
  stdio.h \
  stdlib.h \
  assert.h \
  netinet/ether.h \
  stdnoreturn.h \
  signal.h \
  net/if.h \
  arpa/inet.h \
  sys/mman.h \
  sys/socket.h \
  sys/types.h \
  sys/ioctl.h \
  sys/wait.h \
  netinet/in.h \
  net/ethernet.h \
  netdb.h \
  ifaddrs.h \
  ctype.h \
  stdbool.h \
  fcntl.h \
  unistd.h \
  time.h \
  linux/if_packet.h \
  netinet/ip.h \
  netinet/ip_icmp.h \
  netinet/tcp.h \
  netinet/udp.h \
  sys/time.h \
])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

