# Installing from RPM or DEB

This section covers installing IndiMail from rpm/deb. This is the easy way to install IndiMail.
If you are looking for something more simpler, then you need the Install IndiMail for
dummies - instructions for which are in the file [Quick-INSTALL](Quick-INSTALL.md).

There is also an Installation for Experts, involving source compilation. For that read [INSTALL](INSTALL-indimail.md). I would recommend you to go through the file INSTALL so that you undertand IndiMail better. [INSTALL](INSTALL-indimail.md) gives instructions for installing from source and could be challenging for those who are not accustomed with compilation, make, GNU autotools process.

Please refer to Support Information in [README](README-indimail.md) for getting support for IndiMail.

Refer to 'Binary Builds on openSUSE Build Service' section in [README](README-indimail.md) for indimail repository information for binary builds.

Refer to 'Docker / Podman Repository' section in [README](README-indimail.md) for information on running a docker/podman image of IndiMail.

## What happens during installation?

It depends on whether you are Installing, upgrading or removing IndiMail using the rpm version. Following are the steps carried out during RPM/Deb installation.

### Package Installation/Upgrade

1.  Stops IndiMail services (in case the system already has IndiMail installed)
2.  Add users (indimail, alias, qmaild, qmaill, qmailp, qmailq, qmailr, qmails, qscand)
    and groups (indimail, nofiles, qmail, qscand)
3.  Installs important directories in /var/indimail, binaries in /usr/bin,
    /usr/sbin, /usr/libexec/indimail, shared libraries in /usr/lib64
4.  Install plugins, modules in /usr/lib/indimail
5.  Installs shared files in /usr/share/indimail
6.  Sets up configuration files in /etc/indimail/control, etc
7.  Installs services under supervise in /service
    smtp, smtp - odmr, qmtp, qmqp, qmail-send, imap/pop3, indisrvr, inlookup, pwdlookup,
    clamav antivirus service, freshclam - virus signature update service,
    bogofilter spam filter service
8.  Installs (for installation only) MySQL database for use by IndiMail
    (database in /var/indimail/mysqldb/data and logs in /var/indimail/mysqldb/logs)
9.  Installs 5 queues in /var/indimail/queue (queue1, queue2, ... queue5)
10. Install MTA as the alterntive MTA. Creates link sendmail in /usr/sbin or
    /usr/lib to /usr/bin/sendmail
11. Creates directory for supervise logs in /var/log/svc
12. Configures selinux/apparmor
13. Sets up svscan to be run by init (/etc/inittab), upstart (/etc/event.d) or
    by systemctl (/lib/systemd/system)

### Package Removal

1. Stops IndiMail services (in case the system already has IndiMail installed)
2. Delete users (indimail, alias, qmaild, qmaill, qmailp, qmailq, qmailr, qmails, qscand)
   and groups (indimail, nofiles, qmail, qscand)
3. Removes files/binaries in /var/indimail
4. Removes services under supervise in /service
   smtp, smtp - odmr, qmtp, qmqp, qmail-send, imap/pop3, indisrvr, inlookup, pwdlookup,
   clamav antivirus service, freshclam - virus signature update service,
   bogofilter spam filter service
5. Remove selinux/apparmor rules
6. removes svscan to be run by init (/etc/inittab) or by upstart (/etc/event.d). removes
   all startup scripts
7. Sets the original sendmail as the MTA.

NOTE: MySQL database is not removed, config files in /etc/indimail
/etc/indimail/control, /etc/indimail/users, .qmail files in /var/indimail/alias,
domain information in /var/indimail/domains are not removed.

### What the Installation doesn't do

1. Install and configure MySQL (See How to install MySQL below)
2. Configure IndiMail

Note: The RPM installation does attempt installing a minimal MySQL database in
/var/indimail/mysqldb/data. But if this does not succeed there are workarounds


## Start IndiMail Services

Review the email in /var/indimail/alias/.qmail-postmaster, before starting services

```
% sudo service indimail start         # (works on all systems)
    or
% sudo systemctl start indimail       # (if your system uses systemd - almost all current distros)
    or
% sudo /usr/sbin/initsvc -on # (works on all systems)
    or
% sudo init q                         # (if your system uses traditional init)
    or
% sudo /sbin/initctl emit qmailstart  # (if your system uses upstart - ubuntu/fedora10 &above)
    or
% sudo /bin/launchctl load /System/Library/LaunchDaemons/indimail.plist
% sudo service indimail start         # (If you are on Mac OSX)
```

The RPM/Deb installation deliberately installs MySQL and fetchmail services in down state.  This is for you to review your MySQL installation and configuration. For fetchmail service
to run, you need to create /var/indimail/etc/fetchmailrc file. You can use fetchmailconf for configuring fetchmail. To have these services started automatically during reboot, delete the files /service/mysql.3306/down and /service/fetchmail/down . After deleting the file you can use the following to start MySQL

```
% sudo /usr/bin/svc -u /service/mysql.3306
```

NOTE: Please disable MySQL from getting started up by the system by executing the chkconfig command

```
% sudo chkconfig mysqld off
  or
% sudo systemctl disable mysqld.service
```

Please check your OS documentation on how to disable mysqld from starting automatically. Also I have found that the service name is mysql instead of mysqld on some distros. Another complication could be that the service name could be mariadb instead of mysqld.

If you want to use the MySQL started by the system, then do not delete the file /service/mysql.3306/down and do not issue the above svc command. In such a case you will need to tell IndiMail on how to connect to MySQL.

The information for IndiMail to connect to IndiMail is maintained in the file

```
/etc/indimail/control/host.mysql    # for non-clustered setup
```

and

```
/etc/indimail/control/host.cntrl    # for clustered setup
/etc/indimail/control/host.master   # for clustered setup
```

The format for this file is

```
mysql_host:mysql_user:mysql_pass:mysql_socket[:use_ssl]
```

or

```
mysql_host:mysql_user:mysql_pass:mysql_port[:use_ssl]
```

where use_ssl is the keyword "ssl" or "nossl", or it can be omitted entirely.

Proceed to SECTION 8, STEP 2 in the file [INSTALL](INSTALL-indimail.md) for further instruction on configuring IndiMail

Log files for IndiMail reside under the /var/log/svc directory
