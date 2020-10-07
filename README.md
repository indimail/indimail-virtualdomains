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

This document contains instructions for building indimail-mta from source.

# Source Compiling/Linking

The steps below give instructions to build from source. If you need to deploy indimail-mta on multiple hosts, it is better to create a set of RPM / Deb binary packages. Once generated, the package/packages can be deployed on multiple hosts. To generate RPM packages for all components refer to [Binary Packages](.github/CREATE-Packages.md)

You can also use docker / podman images to deploy indimail. Look at the chapter [Docker / Podman Repository](#docker-/-podman-repository) below on how to do that. The big advantage of using a docker / podman image is you can save your configuration with the `docker commit ..` or `podman commit` to checkpoint your entire build and deploy the exact configuration on multiple hosts.

Doing a source build can be daunting for many. There are total of 18 sub-packages to be built, out of which 6 (libdkim, libsrs2, ucspi-tcp, courier-imap, indimail-mta, indimail) are required, 6 (bogofilter, bogofilter-wordlist, fetchmail, nssd, pam-multi, iwebadmin) are recommended and remaining 6 are optional. On Linux, you can always use the pre-built binaries from the DNF / YUM / APT repositories given in the chapter [Binary Builds on OBS](#binary-builds-on-opensuse-build-service) towards the end of this document.

Doing a source build requires you to have all the development packages installed. Linux distributions are known to be crazy. You will have different package names for different distirbutions. e.g.

db-devel, libdb-devel, db4-devel on different systems, just to get Berkeley db installed. There is an easy way out to find out what your distribution needs.

* For RPM based distribtions, locate your .spec file (e.g. indimail.spec in indimail-virtualdomains/indimail-x directory, libqmail/libqmail.spec). Open the RPM spec file and look for `BuildRequires`. This will tell you what you require for your distribution. If there is a specific version of development package required, you will find `%if %else` statements. Use dnf / yum / zypper to install your development package.
* For debian based distribution, locate your debian subdirectory (e.g. indimail-virtualdomains/indimail-x/debian, libqmail/debian). In this directory you will find files with `.dsc` extension. Look at the line having `Build-Depends`. Use `apt-get install package` to install the package. If your debian distribution has few libraries different than the default, you will find a `.dsc` filename with a name corresponding to your distribution. (e.g. indimail-Debain\_10.dsc)

**Note**

This is a rough list of packages required. If you want the exact packages, look BuildRequires in the \*.spec file or Build-Depends in the debian/control or debian/\*.dsc files

**RPM Based Distributions**
Install the following packages using dnf/yum

```
Universal
gcc gcc-c++ make autoconf automaake libtool pkgconfig
sed findutils diffutils gzip xz binutils coreutils grep flex bison
glibc glibc-devel procps openssl openssl-devel mysql-devel
libqmail-devel libqmail readline readline-devel ncurses-devel
pam-devel libgcrypt-devel gdbm-devel libidn-devel pcre-devel
gettext-devel python3 python3-devel (python python-devel on ancient distros)

opensuse - openldap2-devel instead of openldap-devel
```

**Debian Based Distributions**
Install the following packages using apt

```
Universal
cdbs, debhelper, gcc, g++, automake, autoconf, libtool
libqmail-dev, libqmail, libldap2-dev, libssl-dev,
mime-support, m4, gawk, openssl, procps, sed, bison
findutils, diffutils, readline, libreadline-dev, xz, gzip,
binutils, coreutils, grep, flex, libncurses5-dev, libncurses5,
libpam0g-dev, libpcre3-dev, libgdbm-dev, libdb-dev, libgcrypt20-dev,
libgamin-dev, python, libidn11-dev

Debian 9, Debian 10 - default-libmysqlclient-dev
Remaining - libmysqlclient-dev,
Ubuntu 16.04 - libcom-err2, libmysqlclient-dev
```

FreeBSD / Darwin OSX
FreeBSD - Install the following using pkg

```
# pkg install mysql80-server mysql80-client libidn
```

- You also need either MariaDB (Linux only) or MySQL community server (All Unix distributions)
- You can get mysql-community-server for all distributions [here](https://dev.mysql.com/downloads/mysql/)
- You can get MariaDB [here](https://mariadb.org/download/)

If you need MariaDB for Mac OSX, you can try MacPorts or Brew.

## Compile libqmail

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```
(check version in libqmail/conf-version)

## Download indimail, indimail-mta and components

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
$ git clone https://github.com/mbhangui/indimail-mta.git
```

NOTE: For Darwin (Mac OSX), install [MacPorts](https://www.macports.org/) or brew. You can look at this [document](https://paolozaino.wordpress.com/2015/05/05/how-to-install-and-use-autotools-on-mac-os-x/) for installing MacPorts.

```
# port install autoconf libtool automake pkgconfig
# port install openssl
# port update outdated
```

## Compile libdkim-x (with dynamic libaries)

```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./default.configure
$ make
$ sudo make -s install-strip
```

(check version in indimail-mta/libdkim-x/conf-version)

## Compile libsrs2-x (with dynamic libaries)

```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-mta/libsrs2-x/conf-version)

## Build ucspi-tcp

To configure the build for ucspi-tcp, you need to configure conf-prefix, conf-sysconfdir and conf-shared. Defaults are given in the table below. You can also use the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail

**FreeBSD**

config file|value
-----------|------
conf-prefix|/usr/local
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail

The build below depens on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ ./default.configure
$ make or ./qmake
$ sudo make install or sudo ./qmake install
```

(check version in indimail-mta/ucspi-tcp-x/conf-version)

## Build bogofilter

Optional. Required only if you want to use bogofilter for spam filtering

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
$ ./default.configure
$ make
$ sudo make install-strip
```

NOTE: for Darwin
```
# port install db48
```

(check version in indimail-virtualdomains/bogofilter-x/conf-version)

## Build bogofilter-wordlist

Optional. Required only if you want to use bogofilter for spam filtering

```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-wordlist-1.0
$ ./default.configure
$ make
$ sudo make install-strip
```

## Build indimail-mta-x

To configure the build for indimail-mta, you need to configure conf-prefix, conf-qmail, conf-sysconfdir, conf-shared and conf-libexec. Defaults are given in the table below. You can also use the script default.configure to set the below values.

**Linux**

config file|value
-----------|------
conf-prefix|/usr
conf-qmail|/var/indimail
conf-sysconfdir|/etc/indimail
conf-shared|/usr/share/indimail
conf-libexec|/usr/libexec/indimail

**FreeBSD**

config file|value
-----------|------
conf-prefix|/usr/local
conf-qmail|/var/indimail
conf-sysconfdir|/usr/local/etc/indimail
conf-shared|/usr/local/share/indimail
conf-libexec|/usr/local/libexec/indimail

You can refer to the detailed installation for indimail-mta [here](https://github.com/mbhangui/indimail-mta/blob/master/README.md)

But in short you can install indimail-mta by follow the steps below

The build below depens on several Makefiles. For the build to operate without errors, you need to run default.configure the first time and everytime after you do a `make clean`. If you don't run default.configure, you can run replace `make` with `./qmake`

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ ./default.configure
$ make or ./qmake
$ sudo make install or sudo ./qmake install
$ sudo sh ./svctool --config=users --nolog
```

(check version in indimail-mta/indimail-mta-x/conf-version)

Note: for Darwin
```
# port install openldap mrtg
```

## Build indimail

Rquired for Virtual Domains function.

```
$ cd /usr/local/src/indimail-virtualdomains/indimail-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/indimail-x/conf-version)

## Build courier-imap

Required for IMAP, POP3 to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomains/courier-imap-x
$ ./default.configure
$ sudo make install-strip
```

NOTE: for Darwin
```
# port install libidn2 pcre db48
```

(check version in indimail-virtualdomains/courier-imap-x/conf-version)

### Build fetchmail

Optional. Required only if you want fetchmail to retrieve your mails

```
$ cd /usr/local/src/indimail-virtualdomans/fetchmail-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/fetchmail-x/conf-version)

## Build nssd

Optional component. Required only if you require the Standard C library routines to use Name Service Switch to authenticate from a MySQL db (e.g. for authenticated SMTP, IMAP, POP3, etc). Your passwd(5) database gets extended to indimail's MySQL database. You will also need to edit /etc/nsswitch.conf and have a line like this `passwd: files nssd`. Check the man page for nssd(8) and nsswitch.conf(5)


```
$ cd /usr/local/src/indimail-virtualdomains/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

NOTE: Darwin doesn't have nsswitch. So don't waste time compiling this package

(check version in indimail-virtualdomains/nssd-x/conf-version)

## Build pam-multi

Optional. Required only if you require PAM authentication for authenticated SMTP or extra PAM other than /etc/shadow authentication for IMAP / POP3

```
$ cd /usr/local/src/indimail-virtualdomains/pam-multi-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/pam-multi-x/conf-version)

### Build altermime

Optional. Required only if you want altermime to add content to your emails before delivery. e.g. adding disclaimers

```
$ cd /usr/local/src/indimail-virtualdomans/altermime-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/altermime-x/conf-version)

### Build ripmime

Optional. Required only if you want extract attachments from your emails

```
$ cd /usr/local/src/indimail-virtualdomans/ripmime-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/ripmime-x/conf-version)

### Build mpack

Optional. Required only if you want to pack a zip file and attach it to your email.

```
$ cd /usr/local/src/indimail-virtualdomans/mpack-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/mpack-x/conf-version)

### Build flash

Optional. Required only if you want a configurable ncurses based menu system to configure a system for administering emails using a dumb terminal

```
$ cd /usr/local/src/indimail-virtualdomans/flash-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/flash-x/conf-version)

### Build fortune

Optional. Required only if you want fortune cookies to be sent out in your outgoing emails.

```
$ cd /usr/local/src/indimail-virtualdomans/fortune-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/fortune-x/conf-version)

## Build iwebadmin

Required for a web administration front-end for administering your indimail-users and ezmlm mailing lists. You can do tasks like adding, deleting, modifying users, change user password, update quota, set vacation messages, etc.

```
$ cd /usr/local/src/indimail-virtualdomains/iwebadmin-x
$ ./default.configure
$ make
$ sudo make install-strip
```

(check version in indimail-virtualdomains/iwebadmin-x/conf-version)

## Setup & Configuration

You are here because you decided to do a complete source installation. If you use source installation method, you need to setup various configuration and services. You can configure indimail/indimail-mta using /usr/sbin/svctool. `svctool` is a general purpose utility to configure indimail/indimail-mta services and configuration.

You can also run the script `create_services` which invokes svctool to setup few default services to start a fully configured system. `create_services` will also put a systemd(1) unit file `svscan.service` in `/lib/systemd/system`. For FreeBSD systems, it will configure indimail to be started by rc(8) by creating a rc script in /usr/local/rc.d/svscan.

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ sudo ./create_services
```

The script create_services does the following.

1. Creates MySQL user 'indimail' with password 'ssh-1.5-'. This user has access to indimail database.
2. Creates MySQL user 'mysql' with '4-57343-'. This user has all MySQL privileges.
3. Creates MySQL user 'admin' with 'benhur20'. This user has shutdown privilege in MySQL.
4. Creates user 'admin' in indimail internal database for adminclient(8). This is used by indisrvr(8) for access to various IndiMail(7) programs. The password of this user is 'benhur20'
5. You can also set MYSQL\_PASS, PRIV\_PASS, ADMIN\_PASS environent variables to set your own passwords before running create\_services.
6. If you change the MySQL password for indimail after running create\_services, edit /etc/indimail/control/host.mysql. On FreeBSD this file will be /usr/local/etc/indimail/host.mysql

NOTE: The Darwin Mac OSX system is broken for sending emails because you can't remove /usr/sbin/sendmail. [System Integrity Protection (SIP)](https://en.wikipedia.org/wiki/System_Integrity_Protection) ensures that you cannot modify anything in /bin, /sbin, /usr, /System, etc. You could disable it by using csrutil in recover mode but that is not adviseable. See [this](https://www.howtogeek.com/230424/how-to-disable-system-integrity-protection-on-a-mac-and-why-you-shouldnt/). indimail-mta requires services in /service to configure all startup items. On Mac OS X, it uses `/etc/synthetic.conf' to create a virtual symlink of /service to /usr/local/etc/indimail/service. This file is created/modified by 'svctool --add-boot' command. For program that need to send mails, you will need to call /usr/local/bin/sendmail (indimal-mta's sendmail replacement). The OS and all utilites like cron, mailx, etc will continue to use /usr/sbin/sendmail. There is nothing you can do about it, other than fooling around with SIP.

## Start Services

```
$ sudo systemctl start svscan # Linux
or
$ sudo service svscan start # Linux/FreeBSD
or
$ /etc/init.d/svscan start # Linux
or
$ sudo launchctl start org.indimail.svscan # Mac OSX
or
$ qmailctl start # Universal
```

After starting svscan as given above, your system will be ready to send and receive mails, provided you have set your system hostname, domain name IP addresses and setup mail exchanger in DNS. You can look at this [guide](https://www.godaddy.com/garage/configuring-dns-for-email-a-quick-beginners-guide/) to do that.

## Check Status of Services

The svstat command can be used to query the status of various services. You can query for all services like below. You can query the status of a single service like running a command like this.

```
% sudo svstat /service/qmail-smtpd.25
```
The argument to svstat should be a directory in /service. Each directory in /service refers to an indimail-mta/indimail service. e.g. `/service/qmail-smtpd.25` refers to the SMTP service serving port 25.

```
$ sudo svstat /service/*
/service/dnscache: up (pid 120532) 4394 seconds
/service/fetchmail: down 4394 seconds
/service/greylist.1999: up (pid 120502) 4394 seconds
/service/indisrvr.4000: up (pid 120494) 4394 seconds
/service/inlookup.infifo: up (pid 120465) 4394 seconds
/service/mrtg: up (pid 120495) 4394 seconds
/service/mysql.3306: up (pid 120471) 4394 seconds
/service/proxy-imapd.4143: down 4394 seconds
/service/proxy-imapd-ssl.9143: down 4394 seconds
/service/proxy-pop3d.4110: down 4394 seconds
/service/proxy-pop3d-ssl.9110: down 4394 seconds
/service/pwdlookup: up (pid 120515) 4394 seconds
/service/qmail-daned.1998: up (pid 120413) 4394 seconds
/service/qmail-imapd.143: up (pid 120405) 4394 seconds
/service/qmail-imapd-ssl.993: up (pid 120475) 4394 seconds
/service/qmail-logfifo: up (pid 120499) 4394 seconds
/service/qmail-pop3d.110: up (pid 120407) 4394 seconds
/service/qmail-pop3d-ssl.995: up (pid 120497) 4394 seconds
/service/qmail-poppass.106: up (pid 120474) 4394 seconds
/service/qmail-qmqpd.628: down 4394 seconds
/service/qmail-qmtpd.209: up (pid 120487) 4394 seconds
/service/qmail-send.25: up (pid 120469) 4394 seconds
/service/qmail-smtpd.25: up (pid 120416) 4394 seconds
/service/qmail-smtpd.366: up (pid 120516) 4394 seconds
/service/qmail-smtpd.465: up (pid 120492) 4394 seconds
/service/qmail-smtpd.587: up (pid 120473) 4394 seconds
/service/qscanq: up (pid 120500) 4394 seconds
/service/udplogger.3000: up (pid 120463) 4394 seconds
```

## Create Virtual Domains

The commands below will look familiar to you if you have used [vpopmail](https://www.inter7.com/vpopmail-virtualized-email/). IndiMail-virtualdomains has many commands with the same name but is a totally new package with all code written in djb style. Also the feature list is extensive compated to vpopmail. But unlike vpopmail, indimail-virtualdomains supports MySQL database only. The idea for doing indimail-virtualdomains comes from looking at how vpopmail works. You can extend what many of indimail-virtualdomains programs by creating a script with the same name in /usr/libexec/indimail. These scripts will be passed the same arguments that you pass the original programs. The man pages for the commands will have more details.

##### STEP 1  #############################################
Add a virtual domain

For this example, we will add a domain "indimail.org"

For a non-clustered domain you would execute the following command

```
% sudo /usr/bin/vadddomain indimail.org
    or
% sudo /usr/bin/vadddomain indimail.org password-for-postmaster

For a clustered domain you would execute the following command

% sudo /usr/bin/vadddomain -D indimail -S localhost \
    -U indimail -P ssh-1.5- -p 3306 -c indimail.org password-for-postmaster
    or
% sudo /usr/bin/vadddomain -D indimail -S localhost \
    -U indimail -P ssh-1.5- -p 3306 -c indimail.org password-for-postmaster
```

vadddomain will modify the following qmail files (default locations used)
/etc/indimail/control/locals
/etc/indimail/control/rcpthosts
/etc/indimail/control/morercpthosts (if rcpthosts > than 50 lines)
/etc/indimail/control/virtualdomains
/etc/indimail/users/assign
/etc/indimail/users/cdb

NOTE: FreeBSD/Darwin has /usr/local/etc/indimail instead of /etc/indimail

It will also create a domains directory 
 ~indimail/domains/indimail.org
and user's home directory and Maildir
 ~indimail/domains/indimail.org/postmaster/Maildir

If you do not specify a password on the command line, it will prompt for a password for the postmaster.

Then it will send a kill -HUP signal to qmail-send telling it to re-read the control files.

Note: setting up DNS MX records for the virtual domain is not covered in this INSTALL file. You can look at this [guide](https://www.godaddy.com/garage/configuring-dns-for-email-a-quick-beginners-guide/) to do that.

##### STEP 2  #############################################
Add a new user.

You can use vadduser to add users.

```
% sudo /usr/bin/vadduser newuser@indimail.org
    or
% sudo /usr/bin/vadduser newuser@indimail.org <password-for-newuser>
```

##### STEP 3  #############################################
Delete a user

```
% sudo /usr/bin/vdeluser newuser@indimail.org (for the indimail.org virtualdomain example)
```

##### STEP 4  #############################################
Delete a virtual domain

```
% sudo /usr/bin/vdeldomain indimail.org
```

##### STEP 5  #############################################
Changing a users password

```
% sudo /usr/bin/vpasswd user@indimail.org
        or
% sudo /usr/bin/vpasswd user@indimail.org <password-for-user@indimail.org>
```

##### STEP 6  #############################################
**Man pages**

A lot of the underlying indimail details are not covered in this file. This is on purpose. If you want to find out the internal workings of indimail and qmail look into all files in /usr/share/indimail/doc and /usr/share/man/man?

As a first step, do

```
% man indimail
```

**Send / Receive Mails**

At this stage, your setup is ready to send mails to the outside world. To receive mails, you need to create your actual domain (instead of example.com) using vadddomain and setup a mail exchanger record for your domain (MX record).  To send mails, you can either use SMTP or use sendmail (which is actually /usr/bin/sendmail). You can do the following to send a test mail. The mail command is part of the BSD mail/mailx package

```
% ( echo 'First M. Last'; uname -a) | mail -s "IndiMail Installation" manvendra@indimail.org
```

You can also play around with the system at this point. Try POP3 (110), POP3s (995), IMAP (143), IMAPs (993), SMTP (25), SMTPS (465), Submission (587). You can use telnet or nc command to connect to these ports. You could also configure your mail client to connect to this ports and set it up for regular mail usage.

Replace First M. Last with your name.

# Binary Packages Build

If you need to have indimail on multiple machines, you can build binary packages once and install the same package on multiple machines. The other big advantage of using a binary build is that the binary installation will give you fully functional, configured system using your hostname for defaults. You can always change these configuration files in /etc/indimail to cater to your requirements later. With a binary build, you don't need to run the `create_services` command. To generate RPM packages locally for all components refer to [Binary Packages](.github/CREATE-Packages.md).

You can also download pre-built binary packages from [openSUSE Build Service](https://build.opensuse.org/), described in the chapter [Binary Builds on OBS](#binary-builds-on-opensuse-build-service).

NOTE: binary package for FreeBSD and OSX is in my TODO list.

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
4. [indimail-utils](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils) (Multiple utilities that can work with indimail/indimail-mta - [altermime](http://pldaniels.com/altermime/), [ripMIME](https://pldaniels.com/ripmime/), [mpack](https://github.com/mbhangui/indimail-virtualdomains/tree/master/mpack-x), [fortune](https://en.wikipedia.org/wiki/Fortune_(Unix)) and [flash](https://github.com/mbhangui/indimail-virtualdomains/tree/master/flash-x) - customizable menu based admin interface)

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

# SUPPORT INFORMATION

## IRC / Matrix

* Join me [#indimail:matrix.org](https://matrix.to/#/#indimail:matrix.org)
* IndiMail has an IRC channel #indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)
