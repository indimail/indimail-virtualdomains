# README

## indimail Introduction

Messaging Platform based on qmail for MTA Virtual Domains, Courier-IMAP for IMAP/POP3

* Look at [README-indimail] (README-indimail.md) for details on IndiMail, indimail-mta. indimail needs indimail-mta to be installed. Look [here] (https://github.com/mbhangui/indimail-mta) for details on installing indimail-mta
* Look at [README-CLUSTER] (README-CLUSTER.md) for details on setting up an IndiMail Cluster
* Look at [INSTALL.md] (INSTALL-indimail.md) for Source Installation instructions
* Look at [INSTALL-MINI] (INSTALL-MINI.md) for instructions on setting up an MINI Indimail Installation which uses QMQP protocol.
* Look at [INSTALL-MYSQL] (INSTALL-MYSQL.md) for instructions on configuring a MySQL/MariaDB server for IndiMail.
* Look at [INSTALL-RPM] (INSTALL-RPM.md) for instructions on setting up IndiMail using RPM or Debian packages
* Look at [Quick-INSTALL] (Quick-INSTALL.md) for instructions on installation and setup of an IndiMail server.

# Installation

## Compile libqmail

(check version in libqmail/conf-version)

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```

## Download indimail, indimail-mta and components

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
$ git clone https://github.com/mbhangui/indimail-mta.git
```

## Compile libdkim-x (with dynamic libaries)

(check version in indimail-mta/libdkim-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./default.configure
$ make
$ sudo make -s install-strip
```

## Compile libsrs2-x (with dynamic libaries)

(check version in indimail-mta/libsrs2-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build ucspi-tcp (with all patches applied)

(check version in indimail-mta/ucspi-tcp-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ make
$ sudo make install-strip
```

## Build bogofilter

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
(check version in indimail-virtualdomains/bogofilter-x/conf-version)
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build bogofilter-wordlist

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-wordlist-1.0
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build indimail-mta-x

(check version in indimail-mta/indimail-mta-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ make
$ sudo make install-strip
$ sudo sh ./svctool --config=users --nolog
```

## Build indimail

(check version in indimail-virtualdomains/indimail-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/indimail-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build courier-imap

(check version in indimail-virtualdomains/courier-imap-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/courier-imap-x
$ ./default.configure
$ sudo make install-strip
```

## Build nssd

(check version in indimail-virtualdomains/nssd-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build pam-multi

(check version in indimail-virtualdomains/pam-multi-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/pam-multi-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build optional packages

```
$ cd /usr/local/src/indimail-virtualdomains/altermime-x
$ ./default.configure
$ make
$ sudo make install-strip

$ cd /usr/local/src/indimail-virtualdomains/ripmime-x
$ ./default.configure
$ make
$ sudo make install-strip

$ cd /usr/local/src/indimail-virtualdomains/fortune-x
$ ./default.configure
$ make
$ sudo make install-strip

$ cd /usr/local/src/indimail-virtualdomains/mpack-x
$ ./default.configure
$ make
$ sudo make install-strip

$ cd /usr/local/src/indimail-virtualdomains/flash-x
$ ./default.configure
$ make
$ sudo make install-strip

$ cd /usr/local/src/indimail-virtualdomains/iwebadmin-2.0
$ ./default.configure
$ make
$ sudo make install-strip
```

## Setup & Configuration

Setup (this uses svctool a general purpose utility to configure indimail-mta
services. The create_services is a shell script which uses svctool to setup
indimail-mta. It will also put a systemd unit file indimail.service in
/lib/systemd/system

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ sudo sh ./create_services --servicedir=/services --mysqlPrefix=/usr
```

## Start Services

```
$ sudo systemctl start svscan
or
$ sudo service svscan start
or
$ /etc/init.d/svscan start
or
$ /usr/bin/qmailctl start
```

# Binary Builds

You can get binary RPM / Debian packages at

[Stable Releases] (http://download.opensuse.org/repositories/home:/indimail/)

[Experimental Releases] (http://download.opensuse.org/repositories/home:/mbhangui/)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories are [Stable] (https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail) and [experimental] (https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail-mta)

NOTE: Once you have setup your DNF / YUM / apt-get repo, you an also decide to install the additional software

1. indimail-auth (nssd - providing name service switch and pam-multi to provide multiple pam auth methods)
2. indimail-utils (Multiple utility that can work with indimail-mta - altermime, ripmime, mpack, fortune and flash - customizable menu based admin interface)
3. indimail-spamfilter - SPAM filter capabillity using bogofilter - https://bogofilter.sourceforge.io
4. indimail-saccess - IMAP/POP3 & fetchmail for mail retreival

```
Currently, the list of supported distributions for IndiMail is

    * SUSE
          o openSUSE_Leap_15.0
          o openSUSE_Leap_15.1
          o openSUSE_Leap_15.2
		  o openSUSE_Tumbleweed
          o SUSE Linux Enterprise 12
          o SUSE Linux Enterprise 12 SP1
          o SUSE Linux Enterprise 12 SP2
          o SUSE Linux Enterprise 12 SP3
          o SUSE Linux Enterprise 12 SP4
          o SUSE Linux Enterprise 12 SP5
          o SUSE Linux Enterprise 15
          o SUSE Linux Enterprise 15 SP1

    * Red Hat
          o Fedora 32
          o Fedora 31
          o Red Hat Enterprise Linux 6
          o Red Hat Enterprise Linux 7
          o CentOS 6
          o CentOS 7

    * Debian
          o Debian  8.0
          o Debian  9.0
          o Debian 10.0
          o Ubuntu 16.04
          o Ubuntu 17.04
          o Ubuntu 18.04
          o Ubuntu 19.04
          o Ubuntu 19.10
          o Ubuntu 20.04
```

# Docker / Podman Repository
The docker repository for indimail is at

https://hub.docker.com/r/cprogrammer/indimail

for Docker
```
docker pull cprogrammer/indimail:tag
docker run image_id indimail
```
or

for Podman
```
podman pull cprogrammer/indimail:tag
podman run image_id indimail
```

where image_id is the IMAGE ID of the docker / podman container obtained by running the **docker images** or the **podman images** command and tag is one of

tag|OS Distribution
----|----------------------
xenial|Ubuntu 16.04
bionic|Ubuntu 18.04
disco|Ubuntu 19.04
focal|Ubuntu 20.04
centos7|CentOS 7
debian8|Debian 8
debian9|Debian 9
debian10|Debian10
fc31|Fedora Core 31
fc32|Fedora Core 32
Tumbleweed|openSUSE Tumbleweed
Leap15.2|openSUSE Leap 15.2

# SUPPORT INFORMATION #

## IRC
IndiMail has an IRC channel ##indimail and ##indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here] (https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support] (mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here] (https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here] (https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel] (mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here]
(https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe []here] (http://groups.google.com/group/indimail)
4. Archive at [Google Groups] (http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker] (http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)
