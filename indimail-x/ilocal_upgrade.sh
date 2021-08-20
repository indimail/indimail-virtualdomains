#!/bin/sh
# $Log: ilocal_upgrade.sh,v $
# Revision 2.34  2021-08-20 23:17:48+05:30  Cprogrammer
# fixes for non-existend dir/files
#
# Revision 2.33  2020-06-24 22:23:19+05:30  Cprogrammer
# fixed setting supplementary groups
#
# Revision 2.32  2020-06-17 11:14:54+05:30  Cprogrammer
# removed posttrans to avoid duplicate run of ilocal_upgrade.sh
#
# Revision 2.31  2020-06-09 11:32:46+05:30  Cprogrammer
# fixed typo
#
# Revision 2.30  2020-05-26 11:22:18+05:30  Cprogrammer
# fixed permission of spamignore
#
# Revision 2.29  2020-05-25 23:05:25+05:30  Cprogrammer
# upgrade pwdlookup, qmail-logifo services and nssd config file
#
# Revision 2.28  2020-04-28 10:57:21+05:30  Cprogrammer
# disable mysqld service if indimail database gets created successfully
#
# Revision 2.27  2020-04-27 21:59:16+05:30  Cprogrammer
# added install routine
#
# Revision 2.26  2019-10-01 14:08:25+05:30  Cprogrammer
# use svctool to update libindimail, mysql_lib control files
#
# Revision 2.25  2019-06-17 18:15:33+05:30  Cprogrammer
# update with mysql_lib control file with either libmsyqlclient or libmariadbclient
#
# Revision 2.24  2019-06-07 19:21:49+05:30  Cprogrammer
# set mysql_lib control file
#
# Revision 2.23  2018-03-25 22:19:07+05:30  Cprogrammer
# removed chmod of variables directory as it is redundant now with read perm for indimail group
#
# Revision 2.22  2018-02-18 22:17:58+05:30  Cprogrammer
# pass argument to do_post_upgrade()
#
# Revision 2.21  2018-02-18 21:42:17+05:30  Cprogrammer
# update cron entries
#
# Revision 2.20  2018-01-09 12:11:40+05:30  Cprogrammer
# removed indimail-mta specific code
#
# Revision 2.19  2018-01-08 10:52:23+05:30  Cprogrammer
# fixed typo
#
# Revision 2.18  2017-12-30 21:50:01+05:30  Cprogrammer
# create environment variable DISABLE_PLUGIN
#
# Revision 2.17  2017-12-26 23:34:03+05:30  Cprogrammer
# update control files only if changed
#
# Revision 2.16  2017-11-22 22:37:32+05:30  Cprogrammer
# logdir changed to /var/log/svc
#
# Revision 2.15  2017-11-06 21:45:42+05:30  Cprogrammer
# fixed upgrade script for posttrans
#
# Revision 2.14  2017-10-22 19:03:41+05:30  Cprogrammer
# overwrite LOGFILTER only if it is already set
#
# Revision 2.13  2017-10-22 18:57:27+05:30  Cprogrammer
# fixed rcs id
#
# Revision 2.12  2017-10-22 15:27:23+05:30  Cprogrammer
# remove redundant indimail.service during upgrade
#
# Revision 2.11  2017-04-21 10:24:04+05:30  Cprogrammer
# run upgrade script only on post
#
# Revision 2.10  2017-04-16 19:55:04+05:30  Cprogrammer
# changed qmail-greyd path to /usr/sbin
#
# Revision 2.9  2017-04-14 00:16:35+05:30  Cprogrammer
# added permissions for roundcube to accces certs, spamignore
#
# Revision 2.8  2017-04-11 03:44:57+05:30  Cprogrammer
# documented steps involved in upgrade
#
# Revision 2.7  2017-04-05 14:11:14+05:30  Cprogrammer
# upgraded soft mem to 536870912
#
# Revision 2.6  2017-04-03 15:56:50+05:30  Cprogrammer
# create FIFODIR
#
# Revision 2.5  2017-03-31 21:17:37+05:30  Cprogrammer
# fix DEFAULT_HOST, QMAILDEFAULTHOST, envnoathost, defaulthost settings
#
# Revision 2.4  2017-03-30 23:29:04+05:30  Cprogrammer
# added chgrp
#
# Revision 2.3  2017-03-29 19:31:59+05:30  Cprogrammer
# added rsa2048.pem, dh2048.pem
#
# Revision 2.2  2017-03-29 14:45:42+05:30  Cprogrammer
# fixed upgrade for v2.1
#
# Revision 2.1  2017-03-28 19:21:18+05:30  Cprogrammer
# upgrade script for indimail 2.1
#
#
# $Id: ilocal_upgrade.sh,v 2.34 2021-08-20 23:17:48+05:30 Cprogrammer Exp mbhangui $
#
PATH=/bin:/usr/bin:/usr/sbin:/sbin
chgrp=$(which chgrp)
chmod=$(which chmod)
chown=$(which chown)
rm=$(which rm)
cp=$(which cp)

check_update_if_diff()
{
	val=`cat $1 2>/dev/null`
	if [ ! " $val" = " $2" ] ; then
		echo $2 > $1
	fi
}

do_install()
{
date
echo "Running $1 $Id: ilocal_upgrade.sh,v 2.34 2021-08-20 23:17:48+05:30 Cprogrammer Exp mbhangui $"
if [ -d /var/indimail/mysqldb/data/indimail ] ; then
	if [ ! -f /service/mysql.3306/down ] ; then
		for i in mysqld mariadb mysql
		do
			echo "systemctl disable $i.service"
			systemctl disable $i.service > /dev/null 2>&1
			if [ $? -eq 0 ] ; then
				break
			fi
		done
	fi
fi
/usr/sbin/svctool --fixsharedlibs
}

do_post_upgrade()
{
date
echo "Running $1 $Id: ilocal_upgrade.sh,v 2.34 2021-08-20 23:17:48+05:30 Cprogrammer Exp mbhangui $"
# Fix CERT locations
for i in /service/qmail-imapd* /service/qmail-pop3d* /service/proxy-imapd* /service/proxy-pop3d*
do
	if [ ! -d $i ] ; then
		continue
	fi
	check_update_if_diff $i/variables/TLS_CACHEFILE /etc/indimail/certs/couriersslcache
	check_update_if_diff $i/variables/TLS_CERTFILE /etc/indimail/certs/servercert.pem
done
for i in /service/qmail-poppass* /service/indisrvr.*
do
	if [ ! -d $i ] ; then
		continue
	fi
	check_update_if_diff $i/variables/CERTFILE /etc/indimail/certs/servercert.pem
done

if [ -f /service/fetchmail/variables/QMAILDEFAULTHOST ] ; then
	$rm -f /service/fetchmamil/variables/QMAILDEFAULTHOST
fi
# changed fifo location from /etc/indimail/inquery to /var/indimail/inquery
for i in /service/inlookup.infifo /service/qmail-imapd* /service/qmail-pop3d* \
	/service/qmail-smtpd.25 /service/qmail-smtpd.465 /service/qmail-smtpd.587
do
	if [ -d $i ] ; then
		check_update_if_diff $i/variables/FIFODIR /var/indimail/inquery
	fi
done

# path for /tmp/nssd.sock, /tmp/logfifo have changed to /run/indimail
if [ -d /run ] ; then
	logfifo=/run/indimail/logfifo
	nssd_sock=/run/indimail/nssd.sock
	mkdir -p /run/indimail
	chown indimail:indimail /run/indimail
elif [ -d /var/run ] ; then
	logfifo=/var/run/indimail/logfifo
	nssd_sock=/var/run/indimail/nssd.sock
	mkdir -p /var/run/indimail
	chown indimail:indimail /var/run/indimail
else
	logfifo=/tmp/logfifo
	nssd_sock=/tmp/nssd.sock
fi
if [ " $logfifo" != " /tmp/logfifo" ] ; then
	/usr/sbin/svctool --fifologger=$logfifo --servicedir=/service
	for i in fetchmail qmail-smtpd.25 qmail-logfifo qmail-smtpd.366 \
		qmail-qmqpd.628 qmail-smtpd.465 qmail-qmtpd.209 qmail-smtpd.587
	do
		if [ -d /service/$i ] ; then
			check_update_if_diff /service/$i/variables/LOGFILTER $logfifo
			for j in /service/$i/run /service/$i/variables/.options
			do
				grep "/tmp/logfifo" $j > /dev/null
				if [ $? -eq 0 ] ; then
					sed -i -e "s}/tmp/logfifo}$logfifo}" $j
				fi
			done
		fi
	done
fi
update_nssd=0
for j in /service/pwdlookup/run /service/pwdlookup/variables/.options
do
	if [ ! -f $j ] ; then
		continue
	fi
	grep "/tmp/nssd.sock" $j > /dev/null
	if [ $? -eq 0 ] ; then
		update_nssd=1
		sed -i -e "s}/tmp/nssd.sock}$nssd_sock}" $j
	fi
done

if [ $update_nssd -eq 1 ] ; then
	/usr/sbin/svctool --servicedir=/service --norefreshsvc="1 /service/pwdlookup"
	/usr/sbin/svctool --servicedir=/service --refreshsvc="/service/pwdlookup"
	/usr/sbin/svctool --servicedir=/service --norefreshsvc="0 /service/pwdlookup"
fi

# update pid file to /run or /var/run
if [ -f /etc/indimail/nssd.conf ] ; then
	grep "/tmp/logfifo" /etc/indimail/nssd.conf > /dev/null
	if [ $? -eq 0 ] ; then
		if [ -d /run ] ; then
			pidfile=/run/indimail/nssd.pid
		elif [ -d /var/run ] ; then
			pidfile=/var/run/indimail/nssd.pid
		else
			pidfile=/tmp/nssd.pid
		fi
		sed -i -e "s{^pidfile.*{pidfile     $pidfile{g" /etc/indimail/nssd.conf
	fi
fi

# add for roundcube/php to access certs
/usr/bin/getent passwd apache > /dev/null && /usr/sbin/usermod -aG qmail apache || true
if [ -f /etc/indimail/control/spamignore ] ; then
	$chgrp qmail /etc/indimail/control/spamignore
	$chmod 664 /etc/indimail/control/spamignore
fi

# copy updated cron entries
if [ -f /etc/indimail/cronlist.i -a -d /etc/cron.d ] ; then
	diff /etc/indimail/cronlist.i /etc/cron.d/cronlist.i >/dev/null 2>&1
	if [ $? -ne 0 ] ; then
		$cp /etc/indimail/cronlist.i /etc/cron.d/cronlist.i
	fi
fi

# upgrade libindimail (VIRTUAL_PKG_LIB) for dynamic loading of libindimail
# upgrade libmysqlclient path in /etc/indimail/control/mysql_lib
/usr/sbin/svctool --fixsharedlibs
} 

case $1 in
	post)
	do_post_upgrade $1
	;;
	install)
	do_install $1
	;;
esac
