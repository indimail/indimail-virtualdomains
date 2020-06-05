DATE: Fri Jun  5 20:01:11 IST 2020

Announcing Release 3.2 of IndiMail.

You can find the ChangeLog at the bottom of this notes.

This is a major release with massive overhaul of the code.

WARNING!!! ALERT!!! WARNING!!!. If you are upgrading (not installing),
and you were using mysql-community-server version < 8.0, you
need to run the following command of indimail installation
$ sudo svctool --mysqlupgrade

Please read the link below for points on MySQL 8.0
https://mysqlserverteam.com/upgrading-to-mysql-8-0-here-is-what-you-need-to-know/

* Improved locking on NFS mounted Maildirs for quota calculations
* Upgraded fetchmail to 6.4.1
* Upgraded courier-imap to 5.0.10
* Load libmysqlclient dynamically in nssd, pam-multi
* courier-imap - added maildrop binary
* reduced memory softlimit for imap, pop3, poppass services
* LoadDbinfo.c - BUG - fixed uninitialized relayhosts variable

IndiMail has two RPM / DEB / yum / apt repositories for most of the Linux Distros at

## Stable Releases ##

### Repository Location ###
http://download.opensuse.org/repositories/home:/indimail/

### Install Instructions ###
https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail

Currently, the list of supported distributions for IndiMail is
```
    * SUSE Linux distributions
          o openSUSE Tumbleweed
          o openSUSE_Leap_15.2
          o openSUSE_Leap_15.1
          o SUSE Linux Enterprise 15
          o SUSE Linux Enterprise 15_SP2
          o SUSE Linux Enterprise 15_SP1
          o SUSE Linux Enterprise 12 SP4
          o SUSE Linux Enterprise 12 SP3
          o SUSE Linux Enterprise 12 SP2
          o SUSE Linux Enterprise 12 SP1
          o SUSE Linux Enterprise 12

    * Redhat based distributions
          o Fedora 32
          o Fedora 31
          o Red Hat Enterprise Linux 8
          o Red Hat Enterprise Linux 7
          o Red Hat Enterprise Linux 6
          o CentOS 8
          o CentOS 7
          o CentOS 6

    * Debian Distributions
          o Debian 10
          o Debian 9.0
          o Debian 8.0
          o Ubuntu 20.04
          o Ubuntu 19.10
          o Ubuntu 19.04
          o Ubuntu 18.04
          o Ubuntu 17.04
          o Ubuntu 16.04
```

## Bleeding Edge Releases ##

indimail has RPM / yum repositories for the latest features being added. You will find the RPMs at

### Repository Location ###
http://download.opensuse.org/repositories/home:/mbhangui/

### Install Instructions ###
https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail

## Docker images ##
indimail has [docker/podman](https://hub.docker.com/r/cprogrammer/indimail)images.
Take a look at https://github.com/mbhangui/docker

You can pull all the docker/podman image using the commands

docker pull cprogrammer/indimail:tag
or
podman pull cprogrammer/indimail:tag

Replace 'tag' with one of the following
```
centos8
centos7
fc32
fc31
debian10
debian9
debian8
focal
bionic
xenial
Tumbleweed
Leap15.2
```

## ChangeLog ##
* Fri Jun 05 2020 21:56:22 +0530 indimail-virtualdomains@indimail.org 3.2-1.2%{?dist}
Release 3.2 Start 21/05/2020
01. Added create_rpm script for RPM builds
02. changed logfifo path for fifologger to /run/indimail
03. changed nssd.sock path to /run/indimail
04. changed nssd.pid path to /run/indimail
05. nssd-1.2/configure.ac - fixed --enable-nssd-socket, --enable-nssd-config-path options
06. courier-imap- libcouriertls.c - use PROFILE=SYSTEM as default for ssl_cipher_list
07. ilocal_upgrade - ilocal_upgrade: upgrade pwdlookup, qmail-logifo
    services and nssd config file
08. indimail.spec - use /var/log for setup logs
09. vadddomain_handle, vdeldomain_handle - use root:qmail owner for
    spamignore, nodnscheck control files
10. ilocal_upgrade: fix spamignore permissions
12. indimail.spec: updated fetchmail configure options
12. indimail-x/config/nssd.opts - removed --enable-default-domain
13. indimail.spec - added missing --enable-default-domain parameter to
    configure for nssd, pam-multi
14. initsvc.c - use PREFIX/share/indimail instead of SHAREDDIR
15. nssd, pam-multi, indimail - use datarootdir instead of shareddir in configure scripts
16. pam-multi, nssd made into separate package - indimail-auth
17. altermime, ripmime, flash, mpack, fortune, nssd, pam-multi made into
    separate package - indimail-utils
18. moved out courier-imap, fetchmail into separate package - indimail-access
19. moved out bogofilter into a separate package - indimail-spamfilter
