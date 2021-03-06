
AC_INIT(voiceman, 1.5.2, michael.pozhidaev@gmail.com)
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall foreign])

if test "x$CFLAGS" == 'x'; then
   CFLAGS='-O2'
fi

if test "x$CXXFLAGS" == 'x'; then
   CXXFLAGS='-O2'
fi

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AR
AC_PROG_RANLIB

# Define some parameters to adjust Voiceman's default settings
AC_ARG_VAR(default_socket, Default path to UNIX domain socket for all clients)
AC_ARG_VAR(default_port, Default port for TCP/IP connections)
AC_ARG_VAR(default_executor, Default port for TCP/IP connections)

if test "x$default_socket" = x; then
default_socket="/tmp/voiceman.socket"
fi

if test "x$default_port" = x; then
default_port="5511"
fi

if test "x$default_executor" = x; then
default_executor="voiceman-executor"
fi

# Find the path to libao and set AO_CFLAGS and AO_LIBS
#XIPH_PATH_AO

AC_SUBST(VOICEMAN_INCLUDES, '-I$(top_srcdir)/utils -I$(top_srcdir)/executors')
AC_SUBST(VOICEMAN_DAEMON_INCLUDES, '$(VOICEMAN_INCLUDES) -I$(top_srcdir)/daemon')

AC_SUBST(VOICEMAN_CFLAGS, '-Wall -pedantic -fpic -DNDEBUG -DVOICEMAN_DATADIR=\"$(pkgdatadir)\" -DVOICEMAN_DEFAULT_EXECUTOR=\"$(bindir)/$(default_executor)\" -DVOICEMAN_DEFAULT_SOCKET=\"$(default_socket)\" -DVOICEMAN_DEFAULT_PORT=$(default_port)')
AC_SUBST(VOICEMAN_CXXFLAGS, '-Wall -pedantic -fpic -fno-rtti  -DNDEBUG -DVOICEMAN_DATADIR=\"$(pkgdatadir)\" -DVOICEMAN_DEFAULT_EXECUTOR=\"$(bindir)/$(default_executor)\" -DVOICEMAN_DEFAULT_SOCKET=\"$(default_socket)\" -DVOICEMAN_DEFAULT_PORT=$(default_port)')

AC_SUBST(VOICEMAN_DAEMON_CFLAGS, '$(VOICEMAN_CFLAGS) -DVOICEMAN_DEFAULT_CONFIG=\"$(sysconfdir)/voiceman.conf\"')
AC_SUBST(VOICEMAN_DAEMON_CXXFLAGS, '$(VOICEMAN_CXXFLAGS) -DVOICEMAN_DEFAULT_CONFIG=\"$(sysconfdir)/voiceman.conf\"')

AC_CONFIG_FILES([
  makefile
  utils/makefile

  daemon/makefile
  daemon/config_file/makefile
  daemon/core/makefile
  daemon/langs/makefile
  daemon/main/makefile
  daemon/system/makefile

  executors/makefile

  libvmclient/makefile
  shell/makefile
  emacspeak/makefile
  tools/makefile
])

AC_OUTPUT(voiceman.conf)

AC_CHECK_LIB(ao, ao_initialize, [], [echo;echo Your system has no libao library needed for proper VoiceMan work! >&2;exit 1])
