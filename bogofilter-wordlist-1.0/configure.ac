#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([bogofilter-wordlist],m4_normalize(m4_include([conf-version])), [Manvendra Bhangui indimail-spamfilter@indimail.org],bogofilter-wordlist,[https://github.com/mbhangui/indimail-virtualdomains])
AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
if test -f conf-release
then
	release=$(cat conf-release)
else
	release="1.1"
fi
AC_SUBST(release)

AC_OUTPUT(Makefile)
