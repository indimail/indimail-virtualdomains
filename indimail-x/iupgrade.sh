#!/bin/sh
#
# $Id: iupgrade.sh,v 2.14 2023-06-08 22:27:18+05:30 Cprogrammer Exp mbhangui $
#

do_upgrade()
{
	if [ -f /usr/libexec/indimail/ilocal_upgrade.sh ] ; then
		echo "Running upgrade script for $1 ----------"
		sh /usr/libexec/indimail/ilocal_upgrade.sh $1
	fi
}

do_pre()
{
	case $1 in
		install)
		echo "-- do_pre install -- $PKG_VER -------------"
		do_upgrade preinstall
		echo
		;;
		upgrade)
		echo "-- do_pre upgrade -- $PKG_VER -------------"
		do_upgrade pre
		echo
		;;
	esac
}

do_post()
{
	case $1 in
		install)
		echo "-- do_post install -- $PKG_VER ------------"
		do_upgrade install
		echo
		;;
		upgrade)
		echo "-- do_post upgrade -- $PKG_VER ------------"
		do_upgrade post
		echo
		;;
	esac
}

do_preun()
{
	if [ -f /etc/indimail/indimail-release ] ; then
		echo $PKG_VER > /tmp/indimail-pkg.old
		. /etc/indimail/indimail-release
	else
		/bin/rm -f /tmp/indimail-pkg.old
	fi
	case $1 in
		upgrade)
		echo "-- do_preun upgrade -- $PKG_VER -----------"
		echo
		;;
		uninstall)
		echo "-- do_preun uninstall -- $PKG_VER ---------"
		echo
		;;
		## debian ###
		remove)
		echo "-- do_preun remove -- $PKG_VER ------------"
		echo
		;;
		deconfigure)
		echo "-- do_preun deconfigure -- $PKG_VER -------"
		echo
		;;
	esac
}

do_postun()
{
	if [ -f /etc/indimail/indimail-release ] ; then
		if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
			PKG_VER=`rpm -qf /etc/indimail/indimail-release`
		else
			PKG_VER=`dpkg -S /etc/indimail/indimail-release`
		fi
		. /etc/indimail/indimail-release
	fi
	case $1 in
		upgrade)
		echo "-- do_preun upgrade -- $PKG_VER -----------"
		echo
		;;
		uninstall|remove)
		echo "-- do_postun uninstall -- $PKG_VER --------"
		echo
		;;
		## debian ##
    	purge|failed-upgrade|abort-install|abort-upgrade|disappear)
		echo "-- do_postun purge -- $PKG_VER ------------"
		echo
		;;
	esac
}

do_prettrans()
{
	if [ -f /etc/indimail/indimail-release ] ; then
		if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
			PKG_VER=`rpm -qf /etc/indimail/indimail-release`
		else
			PKG_VER=`dpkg -S /etc/indimail/indimail-release`
		fi
		. /etc/indimail/indimail-release
	fi
	case $1 in
		noargs)
		echo "-- do_prettrans noargs -- $PKG_VER --------"
		echo
		do_upgrade prettrans
		;;
	esac
}

do_posttrans()
{
	if [ -f /tmp/indimail-pkg.old ] ; then
		OLD_PKG_VER=`cat /tmp/indimail-pkg.old`
	fi
	case $1 in
		noargs)
		echo "-- do_posttrans noargs -- $PKG_VER --------"
		echo
		do_upgrade posttrans
		;;
	esac
}

# iupgrade.sh pretrans  noargs    %{version} $*
#   do_upgrade pretrans
# iupgrade.sh pre       upgrade   %{version} $*
#   do_upgrade pre
# iupgrade.sh post      upgrade   %{version} $*
#   do_upgrade post
# iupgrade.sh post      install   %{version} $*
# iupgrade.sh preun     upgrade   %{version} "$argv1"
# iupgrade.sh preun     uninstall %{version} "$argv1"
# iupgrade.sh postun    upgrade   %{version} $*
# iupgrade.sh postun    uninstall %{version} $*
# iupgrade.sh posttrans noargs    %{version} $*
#   do_upgrade posttrans
echo "Start indimail installation Script"
version=$3
if [ -f /etc/indimail/indimail-release ] ; then
	if [ -f /usr/bin/rpm -o -f /bin/rpm ] ; then
		PKG_VER=`rpm -qf /etc/indimail/indimail-release`
	else
		PKG_VER=`dpkg -S /etc/indimail/indimail-release`
	fi
	. /etc/indimail/indimail-release
fi
case $1 in
	pre)
	do_pre $2
	;;
	post)
	do_post $2
	;;
	preun|prerm)
	do_preun $2
	;;
	postun)
	do_postun $2
	;;
	prettrans)
	do_prettrans $2
	;;
	posttrans)
	do_posttrans $2
	;;
esac
echo "End   indimail installation Script"
#
# $Log: iupgrade.sh,v $
# Revision 2.14  2023-06-08 22:27:18+05:30  Cprogrammer
# refactored iupgrade script
#
# Revision 2.13  2020-06-17 11:15:54+05:30  Cprogrammer
# fixed ilocal_upgrade.sh not getting called
#
# Revision 2.12  2020-06-09 11:33:12+05:30  Cprogrammer
# added timestamp and seperators in messages
#
# Revision 2.11  2020-04-27 21:59:40+05:30  Cprogrammer
# added install routine
#
# Revision 2.10  2018-03-13 14:14:04+05:30  Cprogrammer
# fixed syntax error
#
# Revision 2.9  2018-03-13 14:10:29+05:30  Cprogrammer
# fixed rpm path
#
# Revision 2.8  2018-02-18 19:06:01+05:30  Cprogrammer
# fix for opensuse rpm path
#
# Revision 2.7  2018-01-09 12:12:22+05:30  Cprogrammer
# renamed upgrade.sh to iupgrade.sh
#
# Revision 2.6  2017-11-06 21:45:55+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 2.5  2017-10-22 18:53:22+05:30  Cprogrammer
# refactored upgrade.sh
#
# Revision 2.4  2017-03-29 22:37:06+05:30  Cprogrammer
# added case for pretrans
#
# Revision 2.3  2017-03-29 14:49:56+05:30  Cprogrammer
# fixes for v2.1
#
# Revision 2.2  2017-03-28 19:15:10+05:30  Cprogrammer
# added do_upgrade scriptlet
#
# Revision 2.1  2017-03-28 19:12:18+05:30  Cprogrammer
# generic upgrade script for indimail
#
