# INSTALLATION DOCUMENT #

PLEASE DON'T HESITATE to join [IndiMail Group](https://groups.google.com/forum/#!forum/indimail)

## Installation Instructions ##

Copyright (C) 1994, 1995, 1996, 1999, 2000, 2001, 2002, 2004, 2005,
2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.

This file is free documentation; the Free Software Foundation gives
unlimited permission to copy, distribute and modify it.

This section covers installing IndiMail. If you're an experienced system administrator, you can install IndiMail following the directions below. If you are in a hurry, you can directly jump to the Section 'Installation Steps' below. However, I would recommend you to go through the Checklist below to understand what is involved in setting up a full featured mail server.

There is also a 'Installation for Dummies'. For that read [INSTALL-RPM.md](INSTALL-RPM.md)

This document is complicated as it involves installing from multiple sources. However, there is a crash course on IndiMail installation if you want to skip the above two documents and directly install from the RPM/Deb. Read the file [Quick-INSTALL.md](Quick-INSTALL.md)

Using Quick-INSTALL it is possible to setup a fully functional mail server in flat 30 minutes and in less than 10 commands;

Installation of IndiMail using source involves going through ten sections. Don't say that I didn't warn you. You may jump to Section 1 below if you are in a hurry and have some Unix experience under your belt. If you have ideas on how to simplify this documentation further, do write to me. In case you get stuck, do write to me at "Manvendra Bhangui" <manvendra@indimail.org>

You can look also take a look at the document [README.md](README.md).

## Checklist ##

You need to have answers to the following ready before starting the installation.

   1.  Whether you want a full featured **mailstore** or just a relay server installation
   2.  Default Domain name for your Mail Server (**mailstore**)
*      postmaster email account
*      abuse account
   3.  What Filesystem to use (XFS, EXT4 for Maildirs, ext2 for queue)? (mailstore)
   4.  Filesystem where you will have your queue (mailstore, relayserver)
   5.  Filesystem where you will have your user's home directory (mailstore)
   6.  Filesystem where you will have the MySQL data files and logs (mailstore, clusterinfo)
   7.  Filesystem where you will have the supervise log files
   8.  A MySQL database to hold the user cluster information (clusterinfo)
   9.  A MySQL database to hold local user information for each host which is part of the user cluster (mailstore)
   10. Userids, passwords for the MySQL Database (mailstore, clusterinfo)
   11. Estimation of Load (mailstore, relayserver, clusterinfo)
   12. IP addresses (Mailserver, relayservers, clusterinfo)
   13. MX records to be setup (relayserver)
   14. Access to root (mailstore, relayserver, clusterinfo)
   15. Broad level features required (SPAM/Virus Filter, IMAP, POP3, WebMail) (mailstore)
   16. Whom should the daily mail statistics reports be sent to.

## TERMS ##

Term|Description
---|-------
mailstore|A host which keeps the user's maildir. I may also refer sometimes a mailstore as a mailserver.
relayserver|A host which accepts mail from the internet (port 25) and/or the users (port 587)
clusterinfo|A host which hosts the MySQL database having the user cluster information (only if you want to install single domain multi-host mailstores). A clusterinfo helps to extend a domain across multiple mailstores. I may refer the cluserinfo sometimes as the 'central database' or hostcntrl.

Before starting the installation, there are a few things you need to think about.

* If possible, install IndiMail on a "practice" system. This will give you a chance to make mistakes without losing important mail or interrupting mail service to your users. You can also use, while issuing the 'make install' command

  DESTDIR=staging_directory

  to install indimail in a staging directory, before copying to the actual destination. Pre-pending the variable DESTDIR to each target in this way provides for staged installs, where the installed files are not placed directly into their expected location but are instead copied into a temporary location (DESTDIR). However, installed files maintain their relative directory structure. DESTDIR support is commonly used in package creation. It is also helpful to users who want to understand what a given package will install where, and to allow users, who don't normally have permissions to install into protected areas, to build and install before gaining those permissions.  Finally, it can be useful with tools such as stow, where code is installed in one place but made to appear to be installed somewhere else using symbolic links or special mount operations.
* If you don't have a spare system, and your system is already handling mail using sendmail, smail, or some other MTA, you can install and test most pieces of IndiMail without interfering with the existing service.
* When migrating a system from some other MTA to IndiMail, even if you've got some IndiMail experience under your belt, it's a good idea to formulate a plan.


## Requirements ##

IndiMail will install and run on most UNIX and UNIX-like systems, but there are few requirements:

  * Around 300 megabytes of diskspace for all the packages (you will download this in the build
    area).
  * About 800 megabytes of free space in the build area during the build. After the build, you can
    free about 500 megabytes by doing make clean
  * Around 80 megabytes for the binaries, documentation, and configuration files.
  * A safe filesystem for the queue. qmail's reliability guarantee requires that the queue reside
    on a filesystem with traditional BSD FFS semantics. Most modern local filesystems meet these
    requirements with one important exception: the link() system call is often
    asynchronous--meaning that the results of the link() operation might not have been written to
    disk when the link() call returns. Bruce Guenter's syncdir library is used by IndiMail
    to work around this problem.
  * Sufficient disk space for the queue. Small single-user systems only need a couple megabytes.
    Large servers may need a couple gigabytes.
  * A filesystem for the user's home directories where mail will be delivered.
  * A compatible operating system. Most flavors of UNIX are acceptable which have GNU Compilation
    tools (autoconf/automake/libtool/texinfo/emacs).
    NOTE: autoconf 2.69 and above is required.
  * A complete, functioning C development system including a compiler, system header files, and
    libraries. The installation steps will guide you through the installation process.
  * Commands like gzip, bzip2, tar, vi, etc. If you can't find a compiler installed, you'll have to
    locate one and install it. Contact your administrator or OS vendor.
  * MySQL 5.1.x or greater. You can make IndiMail work with 5.0 or less but you will have to
    do some manual tweaking with MySQL database and configuration to make MySQL work with
    IndiMail (read INSTALL-MYSQL for more information).
  * Access to a domain name server (DNS) is highly recommended. Without one, qmail can only send
    to remote systems configured in its smtproutes config file.
  * Adequate network connectivity. IndiMail was designed for well-connected systems, so you
    probably don't want to try to use it for a mailing list server on a 28.8k dial-up. The
    fetchmail package was designed to make Mail more compatible with poorly-connected systems
    works well with IndiMail. IndiMail also provides On Demand Mail Relay Protocol (ODMR) for
    such systems. The installation and configuration for fetchmail is also discussed.
  * IndiMail supports the concept of staged installation. You need to specify
    make DESTDIR=path_to_staging_directory when doing the build. Also svctool (see below) can
    be passed an extra argument --destdir=path_to_staging_directory to create/modify all
    configuration files in the staging area alone. Using staged installation, the administrator
    can install IndiMail on a live system without disturbing an existing running installation.
    To upgrade, all that is required is to move all files from the staged directories to the
    actual production directories. The Directory tree structure in the
    staged area is exactly the same as would be present in the production directory.


## Installation Steps ##

For more information, read Basic Installation instructions at the bottom. In all the steps below (in the make command) you may omit DESTDIR=staging\_directory in the make command in case

a) you are doing this on a fresh system or
b) in case you are installing this on a live IndiMail System and do not wish
   to do make install or make install-strip immediately.
c) IndiMail 2.x follows FHS. The base directory is installed in /var/indimail and does not contain binaries.

Which user should I use to run the commands listed below ?

If a command is prefixed with the '%' sign, it means that it was run using a non-privilege id. e.g.

```
% df -k
```

If a command is prefixed with the '#' sign, it means that it was run using root. e.g.

```
# df -k
```

I usually prefer to run privileged commands like this (so that I don't get fond of login in as root)

```
% sudo df -k
```

## SECTION 1  DISK SPACE Check ##

**check for diskspace**

```
% df

or

% df -k 
```

Determine which disk partition you install IndiMail software

Prepare Source Directory Structure in the disk partition selected
NOTE: nssd does not compile on Mac OS X

```
+-/home/local/src/libqmail/.+
                           . multiple source files

+-/home/local/src/indimail-mta/.+
                           .
                           . libdkim-x           (4.0  Mb)
                           . libsrs2-x           (4.0  Mb)
                           . indimail-mta-x      (18   Mb)

+-/home/local/src/indimail-virtualdomains/.+
                           . indimail-3.2  (20   Mb)
                           . ucspi-tcp-x         (4.0  Mb)
                           . nssd-x              (2.0  Mb)
                           . pam-multi-x         (3.0  Mb)
                           . logalert-x          (2.0  Mb)
                           . flash-x             (4.3  Mb)
                           . altermime-x         (1.5  Mb)
                           . ripmime-x           (2.1  Mb)
                           . mpack-x             (1.5  Mb)
                           . fortune-x           (8.4  Mb)
                           . courier-imap-x      (23   Mb)
                           . bogofilter-x        (11.6 Mb)
                           . fetchmail-x         (10.5 Mb)
                           . iwebadmin-x         (3.0  Mb)
                           ------------------------------
                             Total               (200  Mb)
```

## SECTION 2  USER/GROUP Creation ##

**add groups and users and home directory**

For Linux (I presume this should hold good for OS like Solaris too). You will have to run this commands under root. The commands may be different for the OS you are running. Please consult your OS man pages/documentation for the exact syntax. Also note that the exact value of uids, gids aren't important Note: You can skip this entire section and create the users in SECTION 3, STEP 5, by using the svctool --config=users command

```
% su
# groupadd -g 555 indimail
# groupadd -g 556 nofiles
# groupadd -g 557 qmail
# groupadd -g 558 qscand

# useradd -M -g indimail -u 555 -d /var/indimail indimail
# useradd -M -g nofiles  -u 556 -d /var/indimail/alias  -s /bin/false alias
# useradd -M -g nofiles  -u 557 -d /var/indimail        -s /bin/false qmaild
# useradd -M -g nofiles  -u 558 -d /var/indimail        -s /bin/false qmaill
# useradd -M -g nofiles  -u 559 -d /var/indimail        -s /bin/false qmailp
# useradd -M -g qmail    -u 560 -d /var/indimail        -s /bin/false qmailq
# useradd -M -g qmail    -u 561 -d /var/indimail        -s /bin/false qmailr
# useradd -M -g qmail    -u 562 -d /var/indimail        -s /bin/false qmails
# useradd -M -g qscand   -u 563 -d /var/indimail/qscanq -G qmail,qscand -s /bin/false qscand

on a debian system omit the -M option above
```

For Mac OS X 10.5.4 or greater

```
# dscl . -create /Groups/indimail PrimaryGroupID 555
# dscl . -create /Users/indimail UniqueID 555
# dscl . -create /Users/indimail home /var/indimail
# dscl . -create /Users/indimail PrimaryGroupID 555
# dscl . -create /Users/indimail UserShell /bin/bash
# dscl . -create /Users/indimail RealName indimail

# dscl . -create /Groups/nofiles PrimaryGroupID 556
# dscl . -create /Groups/qmail   PrimaryGroupID 557
# dscl . -create /Groups/qscand  PrimaryGroupID 558

# dscl . -create /Users/alias    UniqueID 556
# dscl . -create /Users/alias    home /var/indimail/alias
# dscl . -create /Users/alias    PrimaryGroupID 556
# dscl . -create /Users/alias    UserShell /bin/bash
# dscl . -create /Users/alias    RealName alias

# dscl . -create /Users/qmaild   UniqueID 557
# dscl . -create /Users/qmaild   home /var/indimail
# dscl . -create /Users/qmaild   PrimaryGroupID 556
# dscl . -create /Users/qmaild   UserShell /bin/bash
# dscl . -create /Users/qmaild   RealName qmaild

# dscl . -create /Users/qmaill   UniqueID 558
# dscl . -create /Users/qmaill   home /var/indimail
# dscl . -create /Users/qmaill   PrimaryGroupID 556
# dscl . -create /Users/qmaill   UserShell /bin/bash
# dscl . -create /Users/qmaill   RealName qmaill

# dscl . -create /Users/qmailp   UniqueID 559
# dscl . -create /Users/qmailp   home /var/indimail
# dscl . -create /Users/qmailp   PrimaryGroupID 556
# dscl . -create /Users/qmailp   UserShell /bin/bash
# dscl . -create /Users/qmailp   RealName qmailp

# dscl . -create /Users/qmailq   UniqueID 560
# dscl . -create /Users/qmailq   home /var/indimail
# dscl . -create /Users/qmailq   PrimaryGroupID 557
# dscl . -create /Users/qmailq   UserShell /bin/bash
# dscl . -create /Users/qmailq   RealName qmailq

# dscl . -create /Users/qmailr   UniqueID 561
# dscl . -create /Users/qmailr   home /var/indimail
# dscl . -create /Users/qmailr   PrimaryGroupID 557
# dscl . -create /Users/qmailr   UserShell /bin/bash
# dscl . -create /Users/qmailr   RealName qmailr

# dscl . -create /Users/qmails   UniqueID 562
# dscl . -create /Users/qmails   home /var/indimail
# dscl . -create /Users/qmails   PrimaryGroupID 557
# dscl . -create /Users/qmails   UserShell /bin/bash
# dscl . -create /Users/qmails   RealName qmails

# dscl . -create /Users/qscand UniqueID 563
# dscl . -create /Users/qscand home /var/indimail/qscanq
# dscl . -create /Users/qscand PrimaryGroupID 558
# dscl . -create /Users/qscand UserShell /bin/false
# dscl . -create /Users/qscand RealName qscand
# dscl . -append /Groups/qmail GroupMembership qscand
```

If you don't want to see the above added  users in the login-window, you might want to use the following command

```
# defaults write /Library/Preferences/com.apple.loginwindow HiddenUsersList \
    -array-add indimail alias qmaild qmaill qmailp qmailq qmailr qmails qscand \
     mysql
# exit
```

RedHat and other linux systems place useradd and groupadd in the /usr/sbin directory.

for freebsd the following would work
```
% su
# pw groupadd -g 555 -n indimail
# pw groupadd -n nofiles
# pw groupadd -n qmail
# pw groupadd -n qscand

# useradd -m -u 555 -g indimail -d /var/indimail -n indimail
# useradd -m -g nofiles  -d /var/indimail/alias  -s /bin/false -n alias
# useradd -m -g nofiles  -d /var/indimail        -s /bin/false -n qmaild
# useradd -m -g nofiles  -d /var/indimail        -s /bin/false -n qmaill
# useradd -m -g nofiles  -d /var/indimail        -s /bin/false -n qmailp
# useradd -m -g qmail    -d /var/indimail        -s /bin/false -n qmailq
# useradd -m -g qmail    -d /var/indimail        -s /bin/false -n qmailr
# useradd -m -g qmail    -d /var/indimail        -s /bin/false -n qmails
# useradd -m -g qscand   -d /var/indimail/qscanq -g qmail,qscand -s /bin/false -n qscand
```

NOTE: the home directory of indimail must exist before you continue with the installation. You can also create all the users using `svctool --config=users` command

## SECTION 3  SOFTWARE INSTALLATION ##

**Download and Extract Software**

Create a Work Directory

```
# mkdir -p /home/local/src
```

Clone the following github repositories

```
% git clone https://github.com/mbhangui/libqmail.git
% git clone https://github.com/mbhangui/indimail-virtualdomains.git
% git clone https://github.com/mbhangui/indimail-mta.git

% cd /home/local/src
```

You may omit STEP 1 if you already have a working installation of MySQL
and would want to use the existing MySQL installation with IndiMail. You
may also omit STEP 2 in cause you do not need DKIM Signatures for signing
for verification

```
##### STEP 1  ##### Install MySQL ######################

The version number is given just as an example. You may use the latest
version available and also chose to install the binary using rpm/deb packages
Install MySQL 5.5.2-m2 with MD5 cd3254f29561953ffb7c023cb1b825d2
(You can install MySQL 5.1.x and greater)
current working directory /home/local/src
% You can download mysql-5.5.2-m2.tar.gz from dev.mysql.com
% gunzip -c mysql-5.5.2-m2.tar.gz |tar xf -
% cd mysql-5.5.2-m2
% ./configure --prefix=/usr --enable-local-infile
% make -s DESTDIR=staging_directory
% sudo make install-strip DESTDIR=staging_directory
cd ..

NOTE: You may install any version of MySQL you like. The steps would be similar.
You can also choose to install MariaDB. MariaDB is a drop-in replacement for
MySQL.

New versions of MySQL use cmake.

% cmake . -DCMAKE_INSTALL_PREFIX=/usr -DENABLED_LOCAL_INFILE=1
% make -s DESTDIR=staging_directory
% sudo make install-strip DESTDIR=staging_directory
cd ..

You can read the transition guide at

http://forge.mysql.com/wiki/Autotools_to_CMake_Transition_Guide
```

```
##### STEP 2  ##### Install libqmail ###################
current working directory /home/local/src

% cd libqmail
% ./default.configure
% make
% sudo make install-strip
```

```
##### STEP 3  ##### Install libdkim-x ###################
current working directory /home/local/src/indimail-mta

% cd libdkim-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..

```

```
##### STEP 4  ##### Install libsrs2-x ################
current working directory /home/local/src/indimail-mta

% cd libsrs2-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..
```

```
##### STEP 5  ##### indimail-mta ##########################
current working directory /home/local/src/indimail-mta

% cd indimail-mta-x

On Linux
change first line in indimail-mta-x/conf-qmail to /var/indimail
change first line in indimail-mta-x/conf-shared to /usr/share/indimail
change first line in indimail-mta-x/conf-prefix to /usr
change first line in indimail-mta-x/conf-libexec to /usr/libexec/indimail
change first line in indimail-mta-x/conf-sysconfdir to /etc/indimail

On FreeBSD
change first line in indimail-mta-x/conf-qmail to /var/indimail
change first line in indimail-mta-x/conf-shared to /usr/local/share/indimail
change first line in indimail-mta-x/conf-prefix to /usr/local
change first line in indimail-mta-x/conf-libexec to /usr/local/libexec/indimail
change first line in indimail-mta-x/conf-sysconfdir to /usr/local/etc/indimail

% make -s

Run the following command to create all default users

% sudo ./svctool --config=users --nolog
Once you have created all users required by IndiMail you can
continue the make below

% sudo make -s install [DESTDIR=staging_directory]
% cd ..
```

```
##### STEP 6  ##### Install ucsp-tcp ######################
current working directory /home/local/src/indimail-mta

ensure that mysql_config is in the path. This is needed by ucspi-tcp to detect
MySQL libraries.

On Linux
change first line in conf-home to /usr
change first line in conf-shared to /usr/share/indimail

On FreeBSD
change first line in conf-home to /usr/local
change first line in conf-shared to /usr/local/share/indimail

% cd ucspi-tcp-x
% make -s [DESTDIR=staging_directory]
% sudo make -s install [DESTDIR=staging_directory]
% cd ..
```

```
##### STEP 5  ##### Install indimail ######################
current working directory /home/local/src/indimail-virtualdomains

% cd indimail-x
% ./default.configure

NOTES: To get a complete list of configure options type: ./configure --help
If you change --enable-indiuser or --enable-indigroup to something different
from indimail:indimail, change conf-users or conf-groups in indimail-mta-2.7
subdirectory and recompile/install indimail-mta

Use --enable-dlload-mysql=no if you want to link libmsqlclient with indimail
binaries and libindimail at runtime.

% make -s [DESTDIR=staging_directory]

% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..

You can omit DESTDIR=staging_directory if you do not have a running indimail system.
Staged installation is required if you have a running indimail system. Staged
installation installs indimail in a staging area. You will have to manually copy
the staging area to the production directories, after shutting down indimail

The above 'sudo make -s install-strip' installs indimail.
'sudo make -s install-strip' also shuts down indimail (if running). You may want to start
indimail at the end of the installation by running the command

% sudo /usr/bin/initsvc -on (see SECTION 5. Start your services below)

BUT DO NOT START THE SERVICES NOW (unless you are upgrading IndiMail)
```

```
##### STEP 7  ##### Install courier IMAP/POP3 #############
current working directory /home/local/src/indimail-virtualdomains

% cd courier-imap-x

% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..

NOTE: remember that IndiMail can work with any IMAP/POP3 server that
provides authentication using PAM or getpwnam(3), getspnam(3). This
happens transparently by using pam-multi(8) in conjunction with nssd(8)
```

```
##### STEP 8  ##### Install indimail-spamfilter ###########
current working directory /home/local/src/indimail-virtualdomains

% cd bogofilter-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..
NOTE: If you are installing bogofilter on Mac OS X, you may need to install
Berkeley db (libdb)
In some cases, if bogofilter gives errors for docs, you may need to install
xmlto package on some systems

# Training Bogofilter ##### SPAM Filter #######################################
OK, now onto the corpus description.  It's split into three parts, as follows:
 - spam: 500 spam messages, all received from non-spam-trap sources.
 - easy_ham: 2500 non-spam messages.  These are typically quite easy to
   differentiate from spam, since they frequently do not contain any spammish
   signatures (like HTML etc).
 - hard_ham: 250 non-spam messages which are closer in many respects to
   typical spam: use of HTML, unusual HTML markup, coloured text,
   "spammish-sounding" phrases etc.
 - easy_ham_2: 1400 non-spam messages.  A more recent addition to the set.
 - spam_2: 1397 spam messages.  Again, more recent.
###############################################################################

% mkdir training
% cd training
Download spam ham corpus from http://spamassassin.apache.org/publiccorpus/
% wget http://spamassassin.apache.org/old/publiccorpus/20030228_easy_ham_2.tar.bz2
% wget http://spamassassin.apache.org/old/publiccorpus/20030228_easy_ham.tar.bz2
% wget http://spamassassin.apache.org/old/publiccorpus/20030228_hard_ham.tar.bz2
% wget http://spamassassin.apache.org/old/publiccorpus/20030228_spam.tar.bz2
% wget http://spamassassin.apache.org/old/publiccorpus/20050311_spam_2.tar.bz2
     
% for i in \*.bz2; do bzip2 -d -c $i | tar xf -; done
% bogofilter -d /etc/indimail -B -s spam
% bogofilter -d /etc/indimail -B -s spam_2
% bogofilter -d /etc/indimail -B -n easy_ham
% bogofilter -d /etc/indimail -B -n easy_ham_2
% bogofilter -d /etc/indimail -B -n hard_ham
% sudo chown indimail:indimail /etc/indimail/wordlist.db (/usr/local/etc/indimail/wordlist.db on FreeBSD)
% cd ..

NOTE: UPGRADE Information. You can always upgrade to the latest version of bogofilter
by applying the latest patch available in the indimail source patch subdirectory
to the orignal latest available bogofilter package.
Then do ./configure <same_parameters_as_earlier_install> ; make ; make install-strip
```

```
##### STEP 9  ## Optional STEP ##### Install fetchmail ####
current working directory /home/local/src/indimail-virtualdomains

% cd fetchmail-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..
```

```
##### STEP 10 ## Optional STEP ## Install nssd ############
current working directory /home/local/src/indimail-virtualdomains

% cd nssd-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..
```

```
##### STEP 11 ## Optional STEP ## Install iwebadmin #######
current working directory /home/local/src/indimail-virtualdomains

% cd iwebadmin-x
% ./default.configure
% make -s [DESTDIR=staging_directory]
% sudo make -s install-strip [DESTDIR=staging_directory]
% cd ..
```

## SECTION 4  Configuration and SETUP ##

**run svctool**

The steps in SECTION 3 above would have installed indimail, indimail-mta, courier-imap, bogofilter (optionally fetchmail, nssd, iwebadmin)

The steps below will help you to install a fresh MySQL database, configure MySQL and have a service installed under supervise which will start MySQL along with all other IndiMail services.

The commands below will create configuration and service for running mysql. The command also creates a default MySQL database for IndiMail in /var/indimail/mysqldb/data and MySQL logs to go in /var/indimail/mysqldb/logs.

```
##### STEP 1  ## MySQL Configuration and Database Creation ##################
a) Create MySQL configuration (/etc/indimail/indimail.cnf)
% sudo svctool --config=mysql --mysqlPrefix=/usr \
 --mysqlport=3306 --mysqlsocket=/var/run/mysqld.sock
You can have a section [indimail] in /etc/indimail/indimail.cnf for parameters specific
to indimail

b) Create a new MySQL database
% sudo svctool --config=mysqldb --mysqlPrefix=/usr \
    --databasedir=/var/indimail/mysqldb --default-domain=indimail.org \
    --base_path=/home/mail
```

```
##### STEP 2  ## MySQL Startup ############################
If you plan to use an existing MySQL server, read INSTALL-MYSQL in indimail-3.2/doc
directory and proceed to STEP 3 after that. The instructions below are for MySQL to be
managed by supervise and hence will require you to disable MySQL startup in your OS
startup/boot scripts.

Remove mysqld startup in boot scripts
% sudo chkconfig mysqld off
% sudo /etc/init.d/mysqld stop

On FreeBSD
% sudo service mysql-server stop
% sudo service mysql-server disable

Some operating systems also use `/etc/rc.local' or `/etc/init.d/boot.local' to start additional
services on startup. You must remove any invocation of mysqld_safe from these scripts.

Create service in supervise to start mysqld
% sudo svctool --mysql=3306 --servicedir=/service \
    --mysqlPrefix=/usr --databasedir=/var/indimail/mysqldb \
    --config=/etc/indimail/indimail.cnf --default-domain=indimail.org

If you had given --enable-dlload-mysql=yes while configuring IndiMail, you should
Create /etc/indimail/control/mysql_lib. This should point to the the shared mysql
client library.  e.g.

% ls -ld /usr/lib*/libmysqlclient*.so.*.*.*
ls: cannot access '/usr/lib*/libmysqlclient.so.*.*.*': No such file or directory

% ls -ld /usr/lib*/mysql/libmysqlclient.so.*.*.*
-rwxr-xr-x. 1 root root 15100376 May  3 03:36 /usr/lib64/mysql/libmysqlclient.so.21.0.16

% sudo sh -c "echo /usr/lib64/mysql/libmysqlclient.so.21.0.16 > /etc/indimail/control/mysql_lib"

Please note that on FreeBSD the libraries will be under /usr/local/lib and
the mysql_lib control file will be in /usr/local/etc/indimail
```

```
##### STEP 3  ## Default Control Files ##################
The command below creates all necessary configuration for running a domain 'indimail.org'
on your host. You may want to replace indimail.org with your domain

% sudo svctool --config=qmail --postmaster=postmaster@indimail.org \
    --default-domain=indimail.org
```

```
##### STEP 4  ## SMTP Service ##################
Stop sendmail and remove from the startup.
% sudo chkconfig sendmail off
% sudo /etc/init.d/sendmail stop
if you don't have clamav installed, don't give the --qhpsi command below

The commands below will create service for the MTA (both smtp and delivery process). The
number of queues can be adjusted by you. The commands below create 5 queues.
% sudo svctool --smtp=25 --servicedir=/service \
    --query-cache --password-cache \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
    --cntrldir=control --localip=0 --maxdaemons=150 --maxperip=25 --persistdb \
    --starttls --fsync --syncdir --memory=104857600 --chkrecipient --chkrelay --masquerade \
    --min-free=52428800 --content-filter --virus-filter \
    --qhpsi="/usr/bin/clamdscan %s --fdpass --quiet --no-summary" \
    --spamfilter="/usr/bin/bogofilter -p -d /etc/indimail" \
    --logfilter=/tmp/spamfifo --rejectspam=0 --spamexitcode=0 --localfilter --remotefilter \
    --remote-authsmtp=plain --dmasquerade \
    --dkverify=both --dksign=both --private_key=/etc/indimail/control/domainkeys/indimail \
    --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net

The command below will create the public and private key for domainkey, dkim signatures

% sudo /usr/bin/dknewkey /etc/indimail/control/domainkeys/indimail 1024

If you want to have a message submission port (587) for users to submit outgoing mails
you can create the Message Submission Port. Note the additional --skipsend option which
creates a service without delivery process. Mails will be processed by delivery process
created above for port 25. The message submission port will enforce authenticated SMTP and
hence you do not require rblsmtpd to frontend the SMTP service.

% sudo svctool --smtp=587 --servicedir=/service \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 --authsmtp --antispoof \
    --query-cache --password-cache \
    --cntrldir=control --localip=0 --maxdaemons=150 --maxperip=25 --persistdb \
    --starttls --fsync --syncdir --memory=104857600 --chkrecipient --chkrelay --masquerade \
    --min-free=52428800 --content-filter --virus-filter \
    --qhpsi="/usr/bin/clamdscan %s --fdpass --quiet --no-summary" \
    --dmasquerade --skipsend \
    --dkverify=both --dksign=both --private_key=/etc/indimail/control/domainkeys/indimail

NOTE: You can create a deliver process for port 587 too either by having a
different --qbase or have a qstart > 5. The point to remember is you can
have multiple delivery process as long as you do not share a queue between
delivery processes.

NOTE: On FreeBSD the path will be /usr/local/etc/indimail/control/domainkeys/indimail
instead of /etc/indimail/control/domainkeys/indimail.

If you are going to use the local mail, sendmail, qmail-inject command, they
need to figure out the queue location. This is done by having a directory
called 'defaultqueue' in /etc/indimail/control. Run the following command to
create this

% sudo svctool --queueParam=defaultqueue \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
    --min-free=52428800 --fsync --syncdir --virus-filter \
    --qhpsi="/usr/bin/clamdscan %s --fdpass --quiet --no-summary" \
    --dkverify="both" --dksign=both \
    --private_key=/etc/indimail/control/domainkeys/indimail

You can implement QMTP service if you want a faster distribution of mails from your
relay servers. Remember you will also need to create the file
/etc/indimail/control/qmtproutes

% sudo svctool --qmtp=209 --servicedir=/service \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 --cntrldir=control --localip=0 \
    --maxdaemons=75 --maxperip=25 --fsync --syncdir --memory=104857600 --min-free=52428800

If you want a centralized mail queue, you can implement QMQP service. Like QMTP, QMQP
is much faster than SMTP.
% sudo svctool --qmqp=628 --servicedir=/service \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 --cntrldir=control --localip=0 \
    --maxdaemons=75 --maxperip=25 --fsync --syncdir --memory=104857600 --min-free=52428800

The above will create a QMQP service for a centralized queue. On servers which do not have
its own queue, you need to create /etc/indimail/control/qmqpservers containing the IP address
of the server which has the QMQP service running. On FreeBSD it will be
/usr/local/etc/indimail/control/qmqpservers.

You can implement Greylisting as described at
http://www.gossamer-threads.com/lists/qmail/users/136740?page=last by executing the command

% sudo svctool --greylist=1999 --servicedir=/service --min-resend-min=2 \
    --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
    --hash-size=65536 --save-interval=5 --whitelist=greylist.white

If you decide to use greylisting, have the following in tcp.smtp

  :allow,GREYIP=":" in the file /etc/indimail/tcp.smtp

You can enforce DANE by configuring qmail-dane TLSA verification daemon by executing the command

% sudo svctool --tlsa=1998 --servicedir=/service \
    --timeout-days=30 --context-file=tlsa.context \
    --hash-size=65536 --save-interval=5 --whitelist=tlsa.white
```

```
##### STEP 5  ## IMAP/POP3 Service ##################
The commands below will create listeners for IMAP and POP3 service.

% sudo svctool --imap=143 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 --starttls
% sudo svctool --imap=993 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 \
    --postmaster=postmaster@indimail.org --ssl
% sudo svctool --pop3=110 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 --starttls
% sudo svctool --pop3=995 --servicedir=/service --localip=0 \
    --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 \
    --postmaster=postmaster@indimail.org --ssl
```

The below four command will create proxies (ssl and non-ssl) for IMAP and
POP3. These will be required if you decide to split your users belonging
to a single domain across multiple hosts.

```
% sudo svctool --imap=4143 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 --starttls \
    --tlsprog=/usr/bin/sslerator \
    --default-domain=indimail.org --memory=52428800 --proxy=143
% sudo svctool --imap=4990 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 --proxy=143 \
    --postmaster=postmaster@indimail.org --ssl
% sudo svctool --pop3=4110 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 --starttls \
    --tlsprog=/usr/bin/sslerator \
    --default-domain=indimail.org --memory=52428800 --proxy=110
% sudo svctool --pop3=4995 --servicedir=/service --localip=0 \
    --query-cache --maxdaemons=150 --maxperip=25 \
    --default-domain=indimail.org --memory=52428800 --proxy=110 \
    --postmaster=postmaster@indimail.org --ssl
```

If you have used the --ssl option above, you can quickly generate a
self-signed X.509 key for IMAP/POP3 over SSL by running the following
command. This command should be run after editing the files
/etc/indimail/imapd.cnf, /etc/indimail/pop3d.cnf
(/usr/local/etc/indimail/imapd.conf, /usr/local/etc/indimail/pop3d.cnf on
 FreeBSD)

the command additionally creates a self-signed X.509 certificate for SMTP over SSL. i.e

```
% sudo svctool --config=cert --postmaster=youremail@yourdomain
```

```
##### STEP 6  ## Query Pooling/Caching/Lookup Service ##################
The command below creates InLookup service with 5 threads. This provides qmail-smtpd,
IMAP and POP3 services to do high speed user lookups and password authentication.
(on FreeBSD replace /etc/indimail/control with /usr/local/etc/indimail/control)

% sudo svctool --inlookup=infifo --servicedir=/service \
    --cntrldir=/etc/indimail/control --threads=5 \
    --activeDays=60 --password-cache --query-cache \
    --use-btree --max-btree-count=10000
```

```
##### STEP 7  ## SPAM/VIRUS Filtering Service ##################
Install clamav (see instructions for your OS)
# check your clamav installation for your OS distribution. e.g

# fedora
% sudo yum install clamav-data clamav-server clamav-filesystem clamav-lib clamav-update clamav

Edit /usr/lib/tmpfiles.d/clamd.scan.conf with the following line
d /var/run/clamd.scan 0750 qscand qmail

# for Ubuntu
% sudo apt-get install clamav clamav-daemon
% sudo update-rc.d -f clamav-daemon disable
% sudo update-rc.d -f clamav-daemon remove

# CentOS 6
% sudo rpm -Uvh http://download.fedoraproject.org/pub/epel/6/x86_64/epel-release-6-8.noarch.rpm
% sudo yum install clamav clamd
% sudo chkconfig clamd off

# CentOS 7
% sudo yum -y install epel-release
% sudo yum -y update
% sudo yum -y install clamav-server clamav-data clamav-update clamav-filesystem clamav clamav-scanner-systemd \
   clamav-lib clamav-server-systemd

The commands below will create services for virus scanning and spam filter.

% sudo svctool --qscanq --servicedir=/service --clamdPrefix=/usr \
    --scanint=200
% sudo svctool --config=clamd
% sudo svctool --config=bogofilter
```

```
##### STEP 8  ## Password Lookup Service ##################
The command below creates Password Lookup Service using Name Service Switch (NSS).
It extends functions like getpwnam(), getspnam(), etc to look at indimail MySQL tables.

% sudo svctool --pwdlookup=/tmp/nssd.sock --threads=5 --timeout=-1 \
    --mysqlhost=localhost --mysqluser=indimail --mysqlpass=ssh-1.5- \
    --mysqlsocket=/var/run/mysqld.sock --servicedir=/service
```

```
##### STEP 9  ## svscan log Service ##################
The command below creates a multilog process for logging output of svscan

% sudo svctool --svscanlog --servicedir=/service
```

```
###### STEP 10 ### Network Access for SMTP/IMAP/POP3 ####################
Check your /etc/indimail/tcp.smtp file, /etc/indimail/tcp.imap file, /etc/indimail/tcp.pop3 file 
(Replace /etc/indimail with /usr/local/etc/indimail on FreeBSD)

This file should list all the static IP's of your machines you want to allow to relay out to the internet.
    
For example: If you have a whole C class named 192.9.200.X either edit
/etc/indimail/tcp.smtp file, or use the following to appened:

% su
# echo "192.9.200.:allow,RELAYCLIENT=\"\"" >> /etc/indimail/tcp.smtp
# echo ":allow,GREYIP=\":\""               >> /etc/indimail/tcp.smtp
# echo "192.9.200.:allow"                  >> /etc/indimail/tcp.qmtp
# echo "192.9.200.:allow"                  >> /etc/indimail/tcp.qmqp
# echo "192.9.200.:allow,IMAPCLIENT=\"\""  >> /etc/indimail/tcp.imap
# echo "192.9.200.:allow,POP3CLIENT=\"\""  >> /etc/indimail/tcp.pop3
# echo "192.9.200.:allow"                  >> /etc/indimail/tcp.poppass
# exit

% sudo /usr/bin/tcprules /etc/indimail/tcp.smtp.cdb \
    /etc/indimail/tcp.smtp.tmp < /etc/indimail/tcp.smtp
% sudo /usr/bin/tcprules /etc/indimail/tcp.qmtp.cdb \
    /etc/indimail/tcp.qmtp.tmp < /etc/indimail/tcp.qmtp
% sudo /usr/bin/tcprules /etc/indimail/tcp.qmqp.cdb \
    /etc/indimail/tcp.qmqp.tmp < /etc/indimail/tcp.qmqp
% sudo /usr/bin/tcprules /etc/indimail/tcp.imap.cdb \
    /etc/indimail/tcp.imap.tmp < /etc/indimail/tcp.imap
% sudo /usr/bin/tcprules /etc/indimail/tcp.pop3.cdb \
    /etc/indimail/tcp.pop3.tmp < /etc/indimail/tcp.pop3
% sudo /usr/bin/tcprules /etc/indimail/tcp.poppass.cdb \
    /etc/indimail/tcp.pop3.tmp < /etc/indimail/tcp.poppass

you can add other ip's later, when ever you want.
```

```
##### STEP 11  ## Optional Services ##################
Optional Commands
-----------------
The command below is required if you need to use ODMR (On Demand Mail Relay Protocol)

% sudo svctool --smtp=366 --odmr --servicedir=/service

The command below is required in case you need to fetch mails from another
host which has intermittent connectivty. You require fetchmail if you need
to pull emails from a host using IMAP, POP3 or ODMR protocol. After creating
fetchmail service, create a file 'down' in the service directory to prevent
fetchmail from getting started automatically. Remove the down file when you
have configured fetchmailrc file. You can use fetchmailconf to generate
fetchmailrc file.

% sudo svctool --fetchmail --servicedir=/service \
    --query-cache --password-cache \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
    --cntrldir=control --default-domain=indimail.org \
    --qhpsi="/usr/bin/clamdscan %s --fdpass --quiet --no-summary" \
    --spamfilter="/usr/bin/bogofilter -p -d /etc/indimail" --spamexitcode=0 \
    --memory=72428800 --fsync --syncdir \
    --dkverify=both
% sudo /bin/touch /service/fetchmail/down

The command below creates a service for indisrvr which allows you to execute
from remote IndiMail admin commands

% sudo svctool --indisrvr=4000 --servicedir=/service \
    --localip=0 --maxdaemons=150 --maxperip=25 --avguserquota=2097152 \
    --certfile=/etc/indimail/control/servercert.pem --ssl \
    --hardquota=52428800 --base_path=/home/mail

The command below creates a service for indimail to support poppassd protocol which
allows you to change passwords over the network.

% sudo svctool --poppass=106 --localip=0 --maxdaemons=40 --maxperip-25 \
    --memory=52428800 \
    --certfile=/etc/indimail/control/servercert.pem --ssl \
    --setpassword=/usr/sbin/vsetpass --servicedir=/service

on FreeBSD change /usr/sbin/vsetpass to /usr/local/sbin/vsetpass and
change /etc/indimail to /usr/local/etc/indimail
```


## SECTION 5  Start Services ##

5. Start your services
IndiMail puts shared libs in /usr/lib64. In case if this is not /usr/lib,
/usr/lib64 or /usr/lib/x86_64-linux-gnu (debian),
/usr/lib/i386-linux-gnu (debian), You need to put the libdir in
/etc/ld.so.conf.d/indimail.conf and run ldconfig

```
# su
# echo lib_path > /etc/ld.so.conf.d/indimail-`arch`.conf
# /sbin/ldconfig
# exit
```

Run the following command to install indimail in startup scripts.

```
% sudo svctool --config=add-boot
```

The command below will install indimail as an alternative MTA in case your system has
/usr/sbin/alternatives or mailwrapper. Additionally sendmail will be disabled. On FreeBSD,
/etc/mail/mailer.conf will be configured.

```
% sudo svctool --config=add-alt
```

The above command will set IndiMail as an alternative MTA and replace /usr/sbin/sendmail
or /usr/lib/sendmail as link to /usr/bin/sendmail (/usr/local/bin/sendmail on FreeBSD)

To make sure that sendmail is disabled on your system

```
% sudo chkconfig --list sendmail
```

In case sendmail is not disabled

```
% sudo chkconfig sendmail off
```

On FreeBSD, you would do

```
% sudo service sendmail disable
```

Shutdown down sendmail

```
% sudo /etc/init.d/sendmail stop
or
% sudo service sendmail stop (on FreeBSD)
```

Make sure that sendmail is linked to /usr/bin/sendmail.  If not, then do the following. On FreeBSD, just configure /etc/mail/mailer.conf.

```
% ln -s /usr/bin/sendmail /usr/sbin/sendmail
```

On some systems sendmail could be in /usr/lib instead of /usr/sbin

```
% ln -s /usr/bin/sendmail /usr/lib/sendmail
```

FINALLY! To start all services

```
% sudo service indimail start
```

## SECTION 6  Verfiy INSTALLATION ##

6. Check your installation

```
% sudo svctool --check-install --servicedir=/service \
    --qbase=/var/indimail/queue --qcount=5 --qstart=1

NOTE: All the above svctool commands, in all previous steps, are in the
script create_services. You can run this script from IndiMail's source
directory to create all services with reasonable defaults.

$ cd /usr/local/src/indimail-mta-x
$ sudo ./create_services --servicedir=/services --qbase=/var/indimail/queue
$ sudo service indimail start
```

## SECTION 7  Crontab Entries ##

**Setup crontab for indimail**

```
% su -
# cp /etc/indimail/cronlist.\* /etc/cron.d
# exit

The following procedure configures MRTG for IndiMail

% sudo mkdir /var/www/html/mailmrtg
% sudo cp /etc/indimail/indimail.mrtg.cfg /var/www/html/mailmrtg
% sudo indexmaker --title="IndiMail Statistics" --section=title \
    --output=/var/www/html/mailmrtg/index.html \
    /etc/indimail/indimail.mrtg.cfg
```

You can go on to add a virtual domain and users if you wish

## SECTION 8  VIRTUAL DOMAIN ADMIN ##

**Operations on Virtual Domains**
IndiMail supports two types of virtual domains

1. Non-Clustered domain - A domain existing on a single server
2. Clustered domain - A domain extended across multiple servers, with each
   server having its own set of users.
   
You must decide if you want Non-clustered setup or a clustered setup. If you have
millions of users, then you must chose clustered setup. Even if you chose to install
a non-clustered setup, you can always migrate to a clustered setup later.

```
##### STEP 1  #############################################
Creating a non-clustered Domain
-------------------------------
Create configuration for IndiMail to connect to MySQL

First you need to have all local information stored in MySQL

% cd /etc/indimail/control (or /usr/local/etc/indiail/control on FreeBSD)
% su
# echo "localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock:nossl" > host.mysql
  or
# echo localhost > host.mysql

Ensure MySQL is running

% sudo /usr/bin/svc -u /service/mysql.3306
% sudo /usr/bin/svstat /service/mysql.3306
/service/mysql.3306: up (pid 11936) 5 seconds

NOTE: replace localhost, indimail, ssh-1.5-, /var/run/mysqld.sock as relevant to your MySQL
installation. You can use 3306 instead of /var/run/mysqld.sock in case your MySQL
database is on another host.
you must use host:user:password:socket or host:user:password:port format
for host.mysql (for IndiMail 1.6.9 and above). Also set your PATH to 
have /usr/bin in the path.

Creating a Clustered Domain
---------------------------
If you need to setup a cluster of mailstores having the same domain, you need to
have a common MySQL server to store cluster information for all mailstores.
After you have setup a MySQL server, you need do the following

% cd /etc/indimail/control (or /usr/local/etc/indiail/control on FreeBSD)
% su
# echo "mysql_server_ip:indimail:ssh-1.5-:/var/run/mysqld.sock" > host.cntrl
   or
# echo mysql_server_ip > host.cntrl
# ln -s host.cntrl host.master
# echo 192.168.1.100 > localiphost (replace 192.168.1.100 with your mailserver/mailstore IP)
# exit

Ensure MySQL is running

% sudo /usr/bin/svc -u /service/mysql.3306
% sudo /usr/bin/svstat /service/mysql.3306
/service/mysql.3306: up (pid 11936) 5 seconds

NOTE: replace localhost, indimail, ssh-1.5-, /var/run/mysqld.sock as relevant
to your MySQL installation. You can use 3306 instead of /var/run/mysqld.sock
in case your MySQL database is on another host.

you must use host:user:password:socket or host:user:password:port format for
host.cntrl (for IndiMail 1.6.9 and above). Also set your PATH to have
/usr/bin in the path.

mysql_server_ip ideally should point to a dedicated MySQL server.
host.cntrl needs to be same for all mailstores. Also you can have
host.master pointing to another MySQL server. host.master, host.cntrl
implements MySQL Master / Slave architecture. However, you need to
configure master slave replication in MySQL.
```

```
##### STEP 2  #############################################
Add a virtual domain

For this example, we will add a domain "indimail.org"

For a non-clustered domain you would execute the following command

% su indimail
% /usr/bin/vadddomain indimail.org
    or
% /usr/bin/vadddomain indimail.org password-for-postmaster

For a clustered domain you would execute the following command

% su indimail
% /usr/bin/vadddomain -D indimail -S localhost \
    -U indimail -P ssh-1.5- -p 3306 -c indimail.org password-for-postmaster
    or
% /usr/bin/vadddomain -D indimail -S localhost \
    -U indimail -P ssh-1.5- -p 3306 -c indimail.org password-for-postmaster

vadddomain will modify the following qmail files 
(default locations used)
/etc/indimail/control/locals
/etc/indimail/control/rcpthosts
/etc/indimail/control/morercpthosts (if rcpthosts > than 50 lines)
/etc/indimail/control/virtualdomains
/etc/indimail/users/assign
/etc/indimail/users/cdb

NOTE: FreeBSD has /usr/local/etc/indimail instead of /etc/indimail

It will also create a domains directory 
    ~indimail/domains/indimail.org
    ~indimail/domains/indimail.org/postmaster/Maildir ...

If you do not specify a password on the command line, it will prompt for a password for the postmaster.

Then it will send a kill -HUP signal to qmail-send telling it to re-read the control files.

Note: setting up DNS MX records for the virtual domain is not covered in this INSTALL file.
```

```
##### STEP 3  #############################################
Add a new user.

You can use vadduser to add users.

% su indimail
% /usr/bin/vadduser newuser@indimail.org
    or
% /usr/bin/vadduser newuser@indimail.org <password-for-newuser>
```

```
##### STEP 4  #############################################
Delete a user

% su indimail
% /usr/bin/vdeluser newuser@indimail.org (for the indimail.org virtualdomain example)
```

```
##### STEP 5  #############################################
Delete a virtual domain

% su indimail
% /usr/bin/vdeldomain indimail.org
```

```
##### STEP 6  #############################################
Changing a users password

% su indimail
% /usr/bin/vpasswd user@indimail.org
        or
% /usr/bin/vpasswd user@indimail.org <password-for-user@indimail.org>
```

## SECTION 9 RTFM ##

**Man pages**
A lot of the underlying indimail details are not covered in this file. This
is on purpose. If you want to find out the internal workings of indimail and
qmail look into all files in /usr/share/indimail/doc and /usr/share/man/man?

As a first step, do

% man indimail


10. Good luck
**Send / Receive Mails**

```
At this stage, your setup is ready to send mails to the outside world. To receive
mails, you need to create your actual domain (instead of example.com) using
vadddomain and setup a mail exchanger record for your domain (MX record).
To send mails, you can either use SMTP or use sendmail (which is actually /usr/bin/sendmail).

you can do the following to send a test mail.

% ( echo 'First M. Last'; uname -a) | mail -s "IndiMail Installation" manvendra@indimail.org

Replace First M. Last with your name.
```

If you have questions about indimail, join the indimail mailing list; You can also use the
tracker to track all your requests.

-------
**CentOS/Fedora/openSUSE/RedHat NOTES**

Install the following for making a build (source build or rpm build) openssl-devel
gcc gcc-c++ bison readline-devel ncurses-devel gettext-devel python-devel flex autoconf
libidn-devel sharutils openldap-devel chrpath pam-devel mysql-devel automake libtool groff
libdb4-devel expect
         
**Ubuntu NOTES**

You may need to install the following additional packages before you can start the
compilation of most of the packages

```
% sudo apt-get install libssl-dev
% sudo apt-get install gettext
% sudo apt-get install libncurses5-dev
% sudo apt-get install libpam0g-dev
% sudo apt-get install libreadline-dev
% sudo apt-get install libdb4.8-dev (or libdb-dev)
% sudo apt-get install libldap2-dev
% sudo apt-get install libmysqlclient-dev
% sudo apt-get install libgsl0-dev
% sudo apt-get install byacc
% sudo apt-get install automake autopoint libtool debhelper
```

for qmailmrtg you will additionally require

```
% sudo apt-get install mrtg
% sudo apt-get install apache2
```

for fetchmail you will additionally require

```
% sudo apt-get install flex bison
```

for courier-imap you could require

```
% sudo apt-get install libidn11 libidn11-dev
```

To remove MySQL from startup

```
# update-rc.d -f mysql remove
```

To enable indimail at startup

```
# update-rc.d indimail start 2345 stop 016
```

**openSUSE Notes**

install the following packages

* mysql-community-server
* libmysqlclient-devel
* pam-devel
* ncurses-devel readline-devel 
* gdbm-devel libdb-4\_8-devel 
* gcc
* gcc-c++
* make
* bison  
* byacc
* flex
* update-alternatives
* libmysqlclient18
* libncurses6
* libreadline6
* lsb-release
* openssl
* readline-devel
* gettext-runtime-mini
* autoconf
* automake
* libtool
* gettext-tools-mini
* libdb-4\_8-devel
* libidn-devel
* libldap
* openldap2-devel
* libopenssl-devel
* pam
* pam-modules
* python-devel
* sharutils
* chrpath
 
**OS X NOTES**

Post Install

In order to get IndiMail to start automatically when OS X boots, you will
need to create a new startup item containing the IndiMail startup script
and paramters list. Create a new startup directory for IndiMail using the
following command:

Mac OS X 10.5.x or later

```
% sudo cp /usr/share/indimail/boot/svscan.plist /Library/LaunchDaemons/org.indimail.svscan
```

Mac OS X 19.6.x or later

```
% sudo cp /usr/share/indimail/boot/svscan.plist /System/Library/LaunchDaemons
```

NOTE: You can also add /usr/sbin/svscanboot to /etc/rc.local

prior to Mac OS X 10.5.x (NOTE: I have not compiled/tested IndiMail on Mac OS X after 10.5.x)

```
% cd ..
% sudo mkdir /Library/StartupItems/IndiMail
% sudo cp /usr/bin/qmailctl /Library/StartupItems/IndiMail/IndiMail
% sudo cp /usr/share/indimail/boot/StartupParameters.plist /Library/StartupItems/IndiMail
```

When you reboot your machine IndiMail will automatically start.

In order to have a Maildir created automatically when a new user is added to the system
you will need to modify the new user template. The new user template can be found in the
/System/Library/User Template/English.lproj directory.

The following four commands will walk you through the Maildir creation process:

```
% cd /System/Library/User\ Template/English.lproj
% su
# /usr/bin/maildirmake Maildir
# echo ./Maildir/ > .qmail
# /Developer/Tools/SetFile -a V Maildir
```

The last command, SetFile -a V Maildir, will set a special bit in the Maildir meta-file such
that the directory is hidden in the Finder (only works on HFS filesystems). This handy command
can be used for any file/directory that you might want to hide. Originally found on the O'Reilly
site macdevcenter.com -- tip \#5 from the article Top Ten Mac OS X Tips for Unix Geeks.

Remember to create a Maildir for each existing user account. You will have to use the SetFile
command on each Maildir individually (if you want to hide them in the Finder). After you create
a Maildir for an existing user, remember to make the user the owner of the of the directory using
chown.

----
If you see the new users in the login-window, you might want to use one of the following commands
(depending on which one works for you) to hide them:

```
% sudo defaults write /Library/Preferences/com.apple.loginwindow HiddenUsersList \
   -array-add indimail alias qmaild qmaill qmailp qmailq qmailr qmails qscand \
   mysql
```

If the above doesn't work, try this one:

```
% sudo defaults write /Library/Preferences/com.apple.loginwindow Hide500User -bool YES
```
[source] (http://qmail.jms1.net/djbdns/osx.shtml)

**MySQL NOTES**

```
CREATE USER user [IDENTIFIED BY [PASSWORD] 'password']
    [, user [IDENTIFIED BY [PASSWORD] 'password']] ...
GRANT
    priv_type [(column_list)]
      [, priv_type [(column_list)]] ...
    ON [object_type]
        {
            *
          | *.*
          | db_name.*
          | db_name.tbl_name
          | tbl_name
          | db_name.routine_name
        }
    TO user [IDENTIFIED BY [PASSWORD] 'password']
        [, user [IDENTIFIED BY [PASSWORD] 'password']] ...
    [REQUIRE
        NONE |
        [{SSL| X509}]
        [CIPHER 'cipher' [AND]]
        [ISSUER 'issuer' [AND]]
        [SUBJECT 'subject']]
    [WITH with_option [with_option] ...]

object_type =
    TABLE
  | FUNCTION
  | PROCEDURE

with_option =
    GRANT OPTION
  | MAX_QUERIES_PER_HOUR count
  | MAX_UPDATES_PER_HOUR count
  | MAX_CONNECTIONS_PER_HOUR count
  | MAX_USER_CONNECTIONS count
```

**PS Output**

```
% ps waux
user       pid %cpu %mem    vsz   rss tty      stat start   time command
smmsp     2506  0.0  0.0   9180  1512 ?        Ss   14:21   0:00 sendmail: Queue runner@01:00:00 for /var/spool/clientmqueue
root     31328  0.0  0.0   2944  1024 ?        Ss   15:24   0:00 /bin/sh /usr/sbin/svscanboot /service /service1
root     31330  0.0  0.0   1908   388 ?        S    15:24   0:00 /usr/sbin/svscan /service
root     31331  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31332  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/svscan
root     31333  0.0  0.0   1728   268 ?        S    15:24   0:00 /usr/sbin/readproctitle /service errors: ................................................................................................................................................................................................................................................................................................................................................................................................................
root     31334  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise qmail-imapd-ssl.993
indimail 31335  0.0  0.0   5928  1188 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/qmail-imapd-ssl.993/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 993 /usr/bin/couriertls -server -tcpd /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpam /usr/bin/imapd Maildir
root     31337  0.0  0.0   1744   316 ?        S    15:24   0:00 supervise log
qmaill   31338  0.0  0.0   1756   312 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/imapd-ssl.993
root     31339  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise qmail-pop3d-ssl.995
indimail 31341  0.0  0.0   5928  1184 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/qmail-pop3d-ssl.995/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 995 /usr/bin/couriertls -server -tcpd /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpam /usr/bin/pop3d Maildir
root     31343  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31345  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/pop3d-ssl.995
root     31347  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise pwdlookup
indimail 31348  0.0  0.0  44344  1288 ?        Sl   15:24   0:00 /usr/sbin/nssd -d debug
root     31354  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
root     31355  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise indisrvr.4000
root     31356  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31357  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/pwdlookup
indimail 31358  0.0  0.0   6360  1320 ?        S    15:24   0:00 /usr/sbin/indisrvr -i 0 -p 4000 -b 40
qmaill   31360  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/indisrvr.4000
root     31361  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise qscanq
root     31363  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31364  0.0  0.0   1756   304 ?        S    15:24   0:00 /usr/bin/multilog t -\* cleanq starting -\* deleting: \*: not sticky/var/log/svc/qscanq
root     31366  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise qmail-imapd.143
indimail 31367  0.0  0.0   5928  1204 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/qmail-imapd.143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 143 /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpam /usr/bin/imapd Maildir
root     31368  0.0  0.0   1744   316 ?        S    15:24   0:00 supervise log
qmaill   31369  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/imapd.143
root     31370  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise qmail-pop3d.110
indimail 31371  0.0  0.0   5928  1196 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/qmail-pop3d.110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 110 /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpam /usr/bin/pop3d Maildir
root     31372  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31373  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/pop3d.110
root     31374  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise fetchmail
indimail 31375  0.0  0.1   6392  2828 ?        S    15:24   0:03 /usr/bin/fetchmail --nodetach -f /etc/indimail/fetchmailrc
root     31376  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31377  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/fetchmail
root     31379  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise proxy-imapd-ssl.9143
indimail 31380  0.0  0.0   5928  1196 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/proxy-imapd-ssl.9143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 9143 /usr/bin/couriertls -server -tcpd /usr/bin/proxyimap /usr/bin/imapd Maildir
root     31383  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31384  0.0  0.0   1756   328 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/proxyIMAP.9143
root     31387  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise proxy-imapd.4143
indimail 31388  0.0  0.0   5928  1188 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/proxy-imapd.4143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 4143 /usr/bin/proxyimap /usr/bin/imapd Maildir
root     31393  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31395  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/proxyIMAP.4143
root     31396  0.0  0.0   1744   328 ?        S    15:24   0:00 supervise proxy-pop3d-ssl.9110
indimail 31397  0.0  0.0   5928  1316 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/proxy-pop3d-ssl.9110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 9110 /usr/bin/couriertls -server -tcpd /usr/bin/proxypop3 /usr/bin/pop3d Maildir
root     31400  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31401  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/proxyPOP3.9110
root     31404  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise freshclam
qscand   31405  0.0  0.0   3500  1332 ?        S    15:24   0:00 /usr/bin/freshclam -v --stdout --datadir=/var/indimail/clamd -f -d -c 2
root     31406  0.0  0.0   1744   316 ?        S    15:24   0:00 supervise log
qmaill   31407  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/freshclam
root     31408  0.0  0.0   1744   316 ?        S    15:24   0:00 supervise qmail-smtpd.366
indimail 31409  0.0  0.0   5928  1188 ?        S    15:24   0:00 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp.smtp.cdb -c /service/qmail-smtpd.366/variables/MAXDAEMONS -o -b 150 -u 555 -g 555 0 366 /usr/sbin/qmail-smtpd indimail.org /usr/sbin/vchkpass /usr/sbin/systpass /bin/false
root     31414  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
qmaill   31415  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/smtpd.366
root     31418  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise proxy-pop3d.4110
indimail 31420  0.0  0.0   5928  1192 ?        S    15:24   0:00 /usr/bin/tcpserver -v -c /service/proxy-pop3d.4110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 4110 /usr/bin/proxypop3 /usr/bin/pop3d Maildir
root     31424  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31425  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/proxyPOP3.4110
root     31426  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise qmail-send.25
root     31427  0.0  0.0   1748   308 ?        S    15:24   0:00 qmail-daemon ./Maildir/
root     31428  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise log
root     31429  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise qmail-smtpd.25
indimail 31430  0.0  0.0   5928  1184 ?        S    15:24   0:00 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp.smtp.cdb -c /service/qmail-smtpd.25/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 25 /usr/sbin/qmail-smtpd indimail.org /usr/sbin/vchkpass /usr/sbin/systpass /bin/false
qmaill   31431  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/deliver.25
root     31435  0.0  0.0   1744   328 ?        S    15:24   0:00 supervise log
qmaill   31437  0.0  0.0   1756   324 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/smtpd.25
qmails   31438  0.0  0.0   1928   420 ?        S    15:24   0:00 qmail-send
root     31439  0.0  0.0   1744   324 ?        S    15:24   0:00 supervise clamd
root     31440  0.0  0.0   6456  1392 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
qmailr   31441  0.0  0.0   6468  1396 ?        S    15:24   0:00 qmail-rspawn
qmailq   31442  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmails   31443  0.0  0.0   1900   388 ?        S    15:24   0:00 qmail-todo
qmails   31445  0.0  0.0   1928   416 ?        S    15:24   0:00 qmail-send
root     31446  0.0  0.0   6456  1392 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
root     31447  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qscand   31449  0.0  3.1  67996 65068 ?        S    15:24   0:01 /usr/sbin/clamd
qmailq   31450  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmails   31453  0.0  0.0   1928   428 ?        S    15:24   0:00 qmail-send
qmailr   31454  0.0  0.0   6468  1396 ?        S    15:24   0:00 qmail-rspawn
root     31455  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise qmail-spamlog
indimail 31456  0.0  0.0   1740   280 ?        S    15:24   0:00 /usr/bin/qmail-cat /tmp/spamfifo
qmaill   31457  0.0  0.0   1756   320 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/clamd
root     31458  0.0  0.0   6456  1428 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
qmails   31461  0.0  0.0   1928   424 ?        S    15:24   0:00 qmail-send
qmailq   31462  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
root     31463  0.0  0.0   6456  1432 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
root     31464  0.0  0.0   1744   316 ?        S    15:24   0:00 supervise log
qmaill   31465  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/spamlog
qmails   31467  0.0  0.0   1928   428 ?        S    15:24   0:00 qmail-send
qmailr   31468  0.0  0.0   6468  1528 ?        S    15:24   0:00 qmail-rspawn
root     31469  0.0  0.0   6456  1564 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
qmailr   31470  0.0  0.0   6468  1404 ?        S    15:24   0:00 qmail-rspawn
qmailq   31471  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmails   31472  0.0  0.0   1900   404 ?        S    15:24   0:00 qmail-todo
qmails   31473  0.0  0.0   1900   396 ?        S    15:24   0:00 qmail-todo
qmailr   31474  0.0  0.0   6468  1400 ?        S    15:24   0:00 qmail-rspawn
qmailq   31475  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmailq   31476  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
root     31477  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise mysql.3306
root     31478  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
mysql    31479  0.0  0.3  36652  6364 ?        Sl   15:24   0:00 /usr/libexec/mysqld --defaults-file=/etc/indimail/indimail.cnf --port=3306 --basedir=/usr --datadir=/var/indimail/mysqldb/data --myisam-recover=backup,force --memlock --skip-external-locking --delay-key-write=all --skip-safemalloc --sql-mode=NO_BACKSLASH_ESCAPES --log=/var/indimail/mysqldb/logs/logquery --log-output=FILE --log-isam=/var/indimail/mysqldb/logs/logisam --log-slow-queries=/var/indimail/mysqldb/logs/logslow --log-queries-not-using-indexes --log-warnings=2 --pid-file=/tmp/mysql.3306.pid
qmaill   31480  0.0  0.0   1756   324 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/mysql.3306
qmails   31481  0.0  0.0   1900   404 ?        S    15:24   0:00 qmail-todo
qmails   31482  0.0  0.0   1928   420 ?        S    15:24   0:00 qmail-send
root     31483  0.0  0.0   6456  1528 ?        S    15:24   0:00 qmail-lspawn ./Maildir/
qmailq   31485  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmailq   31486  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
qmailq   31487  0.0  0.0   1872   372 ?        S    15:24   0:00 qmail-clean
root     31488  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise inlookup.infifo
indimail 31489  0.0  0.0   6356  1260 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
qmailr   31490  0.0  0.0   6468  1396 ?        S    15:24   0:00 qmail-rspawn
qmails   31491  0.0  0.0   1900   400 ?        S    15:24   0:00 qmail-todo
root     31492  0.0  0.0   1744   320 ?        S    15:24   0:00 supervise log
qmaill   31493  0.0  0.0   1756   316 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/inlookup.infifo
qmailq   31494  0.0  0.0   1872   372 ?        S    15:24   0:00 qmail-clean
indimail 31495  0.0  0.0   6572   860 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
indimail 31496  0.0  0.0   6572   860 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
qmailq   31497  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
indimail 31498  0.0  0.0   6572   860 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
indimail 31499  0.0  0.0   6572   860 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
indimail 31500  0.0  0.0   6572   860 ?        S    15:24   0:00 /usr/sbin/inlookup -i 5
qmails   31501  0.0  0.0   1900   392 ?        S    15:24   0:00 qmail-todo
qmailq   31502  0.0  0.0   1872   368 ?        S    15:24   0:00 qmail-clean
root      1056  0.0  0.0   1740   316 ?        S    15:24   0:00 supervise qmail-qmtpd.209
indimail  1078  0.0  0.0   6640  1108 ?        S    15:24   0:00 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp.qmtp.cdb -c /service/qmail-qmtpd.209/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 209 /usr/sbin/qmail-qmtpd
qmaill    1089  0.0  0.0   1756   336 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/qmtpd.209
root      1058  0.0  0.0   1740   316 ?        S    15:24   0:00 supervise qmail-qmqpd.628
indimail  1102  0.0  0.0   6640   836 ?        S    15:24   0:00 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp.qmqp.cdb -c /service/qmail-qmqpd.628/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 628 /usr/sbin/qmail-qmqpd
qmaill    1110  0.0  0.0   1756   288 ?        S    15:24   0:00 /usr/bin/multilog t /var/log/svc/qmqpd.628
```
