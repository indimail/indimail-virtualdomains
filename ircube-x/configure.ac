#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ircube],m4_normalize(m4_include([conf-version])),m4_normalize(m4_include([conf-email])),ircube, [https://github.com/mbhangui/indimail-virtualdomains])
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_ARG_ENABLE(plugindir, [  --enable-plugindir=dir    directory where roundcube plugins directory is present.],
	plugindir="$enableval",
	[
		if test "$plugindir" = ""
		then
			AC_MSG_ERROR([Unable to find your plugindir directory, specify --enable-plugindir.])
		fi
	])
AC_DEFINE_UNQUOTED(QMAILDIR,"$plugindir",roundcube's plugin directory)
AC_SUBST(plugindir)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([ircube-release:ircube-release.in],[], [])
AC_OUTPUT
