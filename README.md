# indimail
Messaging Platform based on qmail for MTA Virtual Domains, Courier-IMAP for IMAP/POP3

Look at indimail-3.x/doc/README for details. indimail needs indimail-mta to be installed. Look at
https://github.com/mbhangui/indimail-mta
for details on installing indimail-mta

Look at indimail-3.x/doc/INSTALL for Source Installation instructions

# Compile libqmail
(check version in libqmail/conf-version)
```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/libqmail.git
$ cd /usr/local/src/libqmail
$ ./default.configure
$ make
$ sudo make install-strip
```

# Download indimail, indimail-mta and components
```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
$ git clone https://github.com/mbhangui/indimail-mta.git
```

# Compile libdkim-x (with dynamic libaries)
(check version in indimail-mta/libdkim-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/libdkim-x
$ ./default.configure
$ make
$ sudo make -s install-strip
```

# Compile libsrs2-x (with dynamic libaries)
(check version in indimail-mta/libsrs2-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/libsrs2-x
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build ucspi-tcp (with all patches applied)
(check version in indimail-mta/ucspi-tcp-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/ucspi-tcp-x
$ make
$ sudo make install-strip
```

# Build bogofilter
```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-x
(check version in indimail-virtualdomains/bogofilter-x/conf-version)
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build bogofilter-wordlist
```
$ cd /usr/local/src/indimail-virtualdomains/bogofilter-wordlist-1.0
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build indimail-mta-x
(check version in indimail-mta/indimail-mta-x/conf-version)
```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ make
$ sudo make install-strip
$ sudo sh ./svctool --config=users --nolog
```

# Build indimail
(check version in indimail-virtualdomains/indimail-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/indimail-x
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build courier-imap
(check version in indimail-virtualdomains/courier-imap-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/courier-imap-x
$ ./default.configure
$ sudo make install-strip
```

# Build nssd
(check version in indimail-virtualdomains/nssd-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build pam-multi
(check version in indimail-virtualdomains/pam-multi-x/conf-version)
```
$ cd /usr/local/src/indimail-virtualdomains/pam-multi-x
$ ./default.configure
$ make
$ sudo make install-strip
```

# Build optional packages
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
```

# Setup & Configuration

Setup (this uses svctool a general purpose utility to configure indimail-mta
services. The create_services is a shell script which uses svctool to setup
indimail-mta. It will also put a systemd unit file indimail.service in
/lib/systemd/system

```
$ cd /usr/local/src/indimail-mta/indimail-mta-x
$ sudo sh ./create_services --servicedir=/services --mysqlPrefix=/usr
```

# Start Services
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

Stable Releases       - http://download.opensuse.org/repositories/home:/indimail/

Experimental Releases - http://download.opensuse.org/repositories/home:/mbhangui/

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories are at

https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail

&

https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail

NOTE: Once you have setup your DNF / YUM / apt-get repo, you an also decide to install the additional software

1. indimail-auth (nssd - providing name service switch and pam-multi to provide multiple pam auth methods)
2. indimail-utils (Multiple utility that can work with indimail-mta - altermime, ripmime, mpack, fortune and flash - customizable menu based admin interface)
3. indimail-spamfilter - SPAM filter capabillity using bogofilter - https://bogofilter.sourceforge.io

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
```
or

for Podman
```
podman pull cprogrammer/indimail:tag
```

where tag is one of

xenial   for ubuntu 16.04

bionic   for ubuntu 18.04

disco    for ubuntu 19.04

focal    for ubuntu 20.04

centos7  for centos7

debian8  for debian8

debian9  for debian9

debian10 for debian10

fc32     for fc32

fc31     for fc31
