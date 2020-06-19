# Installation (source and binary)

## indimail Introduction

Messaging Platform based on [indimail-mta](https://github.com/mbhangui/indimail-mta) for MTA (modified qmail), [IndiMail-VirtualDomains](https://github.com/mbhangui/indimail-virtualdomains) for Virtual Domains, [Courier-IMAP](https://www.courier-mta.org/imap/) for IMAP/POP3

* Look at [README-indimail](.github/README-indimail.md) for details on IndiMail, indimail-mta. indimail needs indimail-mta to be installed, instructions for installing the same are included below. You can also read [README](https://github.com/mbhangui/indimail-mta/blob/master/README.md) for details specific to installing indimail-mta alone.
* Look at [README-CLUSTER](.github/README-CLUSTER.md) for details on setting up an IndiMail Cluster
* Look at [INSTALL](.github/INSTALL-indimail.md) for very detailed Source Installation instructions. You may not need that if you follow instructions below in this document itself.
* Look at [INSTALL-MINI](.github/INSTALL-MINI.md) for instructions on setting up an MINI Indimail Installation which uses QMQP protocol.
* Look at [INSTALL-MYSQL](.github/INSTALL-MYSQL.md) for instructions on configuring a MySQL/MariaDB server for IndiMail.
* Look at [INSTALL-RPM](.github/INSTALL-RPM.md) for instructions on setting up IndiMail using RPM or Debian packages
* Look at [Quick-INSTALL](.github/Quick-INSTALL.md) for instructions on installation and setup of an IndiMail server.

# Source Installation

The steps below give instructions on building from source. If you need to deploy IndiMail on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the same package can be deployed on multiple hosts. To generate RPM packages for all components refer to [CREATE-Packages](.github/CREATE-Packages.md)

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

You can refer to the detailed installation for indimail-mta [here](https://github.com/mbhangui/indimail-mta/blob/master/README.md)

But in short you can install indimail-mta by follow the steps below

(check version in indimail-mta/indimail-mta-x/conf-version)

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./default.configure
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

You are here because you decided to do a complete source installation. If you use source installation method, you need to setup various configuration and services. You can configure indimail-mta using /usr/sbin/svctool. `svctool` is a general purpose utility to configure indimail-mta services.

You can also run the script `create_services` which invokes svctool to setup few default services to start a fully configured system. `create_services` will also put a systemd unit file `svscan.service` in `/lib/systemd/system`.

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

# Binary Packages Build

If you need to have indimail on multiple machines, you can build binary packages once and install the same package on multiple machines. The other big advantage of using a binary build is that the binary installation will give you fully functional, configured system using your hostname for defaults. You can always change these configuration files in /etc/indimail to cater to your requirements later. With a binary build, you don't need to run the `create_services` command.
You can also download pre-built binary packages from [openSUSE Build Service](https://build.opensuse.org/), described in the chapter '# Binary Builds on openSUSE Build Service`.

The steps for doing a binary build are

## Clone git repository

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ git clone https://github.com/mbhangui/indimail-mta.git
$ git clone https://github.com/mbhangui/indimail.git
$ git clone https://github.com/mbhangui/indimail-access.git
$ git clone https://github.com/mbhangui/indimail-auth.git
$ git clone https://github.com/mbhangui/indimail-utils.git
$ git clone https://github.com/mbhangui/indimail-spamfilter.git
```

## Build libqmail, libqmail-dev package

```
$ cd /usr/local/src/libqmail
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-mta package

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-access package

```
$ cd /usr/local/src/indimail-mta/indimail-access
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-auth package

```
$ cd /usr/local/src/indimail-mta/indimail-auth
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-utils package

```
$ cd /usr/local/src/indimail-mta/indimail-utils
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Build indimail-spamfilter package

```
$ cd /usr/local/src/indimail-mta/bogofilter-x
$ ./create_rpm    # for RPM
or
$ ./create_debian # for deb
```

## Install Packages

Installing and configuration is much simplied when you use the Binary Packages Build. The pre, post instlation scripts do all the hard work for you. For each of the packages built above you can follow the steps below, depending on your linux distribution. The location of the RPM file will be ~/rpmbuild/RPMS/x86_64 for rpm based distributions and ~/stage for debian/ubuntu distributions.

** For RPM based distributions **

`sudo rpm -ivh rpm_file`

** For Debian based distributions **

`sudo dpkg -i debian_file`


# Binary Builds on openSUSE Build Service

You can get binary RPM / Debian packages at

* [Stable Releases](http://download.opensuse.org/repositories/home:/indimail/)
* [Experimental Releases](http://download.opensuse.org/repositories/home:/mbhangui/)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable](https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail)
* [Experimental](https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail)

NOTE: Once you have setup your DNF / YUM / apt-get repo, you an also decide to install the additional software

1. [indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access) - IMAP/POP3 & fetchmail for mail retreival
2. [indimail-auth](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-auth) (nssd - providing Name Service Switch and pam-multi providing multiple PAM modules for flexible, configurable authentication methods)
3. [indimail-spamfilter](https://github.com/mbhangui/indimail-virtualdomains/tree/master/bogofilter-x) - SPAM filter capabillity using bogofilter - https://bogofilter.sourceforge.io
4. [indimail-utils](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils) (Multiple utilities that can work with indimail-mta - [altermime](http://pldaniels.com/altermime/), [ripMIME](https://pldaniels.com/ripmime/), [mpack](https://github.com/mbhangui/indimail-virtualdomains/tree/master/mpack-x), [fortune](https://en.wikipedia.org/wiki/Fortune_(Unix)) and [flash](https://github.com/mbhangui/indimail-virtualdomains/tree/master/flash-x) - customizable menu based admin interface)

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

The [docker repository](https://hub.docker.com/r/cprogrammer/indimail) can be used to pull docker/podman images
for indimail.

For latest details refer to [README](https://github.com/mbhangui/docker/blob/master/README.md)

# SUPPORT INFORMATION #

## IRC
IndiMail has an IRC channel ##indimail and ##indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)
