![IndiMail](indimail_logo.png "IndiMail")

Table of Contents
=================

   * [INTRODUCTION](#introduction)
   * [LICENSING](#licensing)
   * [TERMINOLOGY used for commands](#terminology-used-for-commands)
   * [indimail-mta Internals](#indimail-mta-internals)
      * [Programs](#programs)
         * [qmail-multi](#qmail-multi)
         * [qmail-queue](#qmail-queue)
         * [qscheduler, qmail-start, slowq-start, qmta-send](#qscheduler-qmail-start-slowq-start-qmta-send)
         * [todo-proc, qmail-send](#todo-proc-qmail-send)
            * [Preprocessing](#preprocessing)
            * [Delivery](#delivery)
               * [Retry Schedules](#retry-schedules)
            * [Cleanup](#cleanup)
            * [Global &amp; Queue Specific Concurrency, Parallelism limits](#global--queue-specific-concurrency-parallelism-limits)
      * [Setting Environment Variables](#setting-environment-variables)
      * [Taking Backups](#taking-backups)
      * [Notes](#notes)
   * [IndiMail Queue Mechanism](#indimail-queue-mechanism)
   * [Using systemd to start IndiMail](#using-systemd-to-start-indimail)
   * [Eliminating Duplicate Emails during local delivery](#eliminating-duplicate-emails-during-local-delivery)
   * [Using procmail with IndiMail](#using-procmail-with-indimail)
   * [Writing Filters for IndiMail](#writing-filters-for-indimail)
      * [Filtering during SMTP (before mail gets queued)](#filtering-during-smtp-before-mail-gets-queued)
         * [Using FILTERARGS environment variable](#using-filterargs-environment-variable)
         * [Using QMAILQUEUE with qmail-qfilter](#using-qmailqueue-with-qmail-qfilter)
         * [Using QMAILQUEUE with your own program](#using-qmailqueue-with-your-own-program)
      * [Filtering during local / remote delivery](#filtering-during-local--remote-delivery)
         * [Using FILTERARGS environment variable](#using-filterargs-environment-variable-1)
         * [Using control file filterargs](#using-control-file-filterargs)
         * [Using QMAILLOCAL or QMAILREMOTE environment variables](#using-qmaillocal-or-qmailremote-environment-variables)
      * [Using dot-qmail(5) or valias(1)](#using-dot-qmail5-or-valias1)
      * [Using IndiMail rule based filter - vfilter](#using-indimail-rule-based-filter---vfilter)
      * [Examples Filters](#examples-filters)
         * [FILTERARGS script](#filterargs-script)
         * [QMAILQUEUE script](#qmailqueue-script)
         * [QMAILREMOTE script](#qmailremote-script)
         * [QMAILLOCAL script](#qmaillocal-script)
   * [IndiMail Delivery mechanism explained](#indimail-delivery-mechanism-explained)
      * [Delivery Mode](#delivery-mode)
      * [Addresses](#addresses)
      * [qmail-users](#qmail-users)
      * [Extension Addresses](#extension-addresses)
   * [Controlling Delivery Rates](#controlling-delivery-rates)
   * [Delivery Instructions for a virtual domain](#delivery-instructions-for-a-virtual-domain)
   * [Setting up TLS for SMTP (qmail-smtpd) and Remote Delivery (qmail-remote)](#setting-up-tls-for-smtp-qmail-smtpd-and-remote-delivery-qmail-remote)
      * [Using tcpserver to provide encrypted SMTPS service](#using-tcpserver-to-provide-encrypted-smtps-service)
      * [Using dotls provide encrypted SMTPS service or STARTTLS capability](#using-dotls-provide-encrypted-smtps-service-or-starttls-capability)
      * [Using qmail-smtpd to provide SMTPS and STARTTLS](#using-qmail-smtpd-to-provide-smtps-and-starttls)
      * [Setting up qmail-remote to do STARTTLS](#setting-up-qmail-remote-to-do-starttls)
      * [Updating RSA and DH parameters](#updating-rsa-and-dh-parameters)
   * [Distributing your outgoing mails from Multiple IP addresses](#distributing-your-outgoing-mails-from-multiple-ip-addresses)
   * [Handling Bounces](#handling-bounces)
      * [Bounce Address Tag Validation (BATV)](#bounce-address-tag-validation-batv)
      * [Using environment variable BOUNCEPROCESSOR](#using-environment-variable-bounceprocessor)
      * [Using environment variable BOUNCERULES or control files bounce.envrules.](#using-environment-variable-bouncerules-or-control-files-bounceenvrules)
      * [Using BOUNCEQUEUE environment variable to queue bounces](#using-bouncequeue-environment-variable-to-queue-bounces)
   * [Setting Disclaimers in your emails](#setting-disclaimers-in-your-emails)
      * [Option 1 - using /etc/indimail/control/filterargs](#option-1---using-etcindimailcontrolfilterargs)
      * [Option 2 - Set the FILTERARGS environment variable](#option-2---set-the-filterargs-environment-variable)
   * [Email Archiving](#email-archiving)
      * [using environment variable EXTRAQUEUE](#using-environment-variable-extraqueue)
      * [using control file mailarchive](#using-control-file-mailarchive)
   * [Envrules](#envrules)
   * [Domain Specific Queues](#domain-specific-queues)
   * [indimail-mini / qmta Installation](#indimail-mini--qmta-installation)
      * [indimail-mini - Using QMQP protocol provided by qmail-qmqpc / qmail-qmqpd](#indimail-mini---using-qmqp-protocol-provided-by-qmail-qmqpc--qmail-qmqpd)
         * [How do I set up a QMQP service?](#how-do-i-set-up-a-qmqp-service)
         * [Client Setup - How do I install indimail-mini to use qmail-qmqpc](#client-setup---how-do-i-install-indimail-mini-to-use-qmail-qmqpc)
      * [qmta - Using a minimal standalone qmta-send MTA](#qmta---using-a-minimal-standalone-qmta-send-mta)
         * [How do I set up a standalone MTA using qmta-send](#how-do-i-set-up-a-standalone-mta-using-qmta-send)
   * [Using /usr/sbin/alternatives](#using-usrsbinalternatives)
   * [Post Handle Scripts](#post-handle-scripts)
   * [Authenticated SMTP Mechanisms](#authenticated-smtp-mechanisms)
      * [LOGIN and PLAIN authentication mechanism](#login-and-plain-authentication-mechanism)
      * [Challenge Response Authentication Mechanisms](#challenge-response-authentication-mechanisms)
      * [Salted Challenge Response Authentication Mechanism](#salted-challenge-response-authentication-mechanism)
      * [Authentication SMTP mechanism in qmail-smtpd](#authentication-smtp-mechanism-in-qmail-smtpd)
      * [Enabling authentication methods for qmail-remote](#enabling-authentication-methods-for-qmail-remote)
   * [Relay Mechanism in IndiMail](#relay-mechanism-in-indimail)
      * [Using tcp.smtp](#using-tcpsmtp)
      * [Using control file relayclients](#using-control-file-relayclients)
      * [Using MySQL relay table](#using-mysql-relay-table)
      * [Set up Authenticated SMTP](#set-up-authenticated-smtp)
      * [Using control file relaydomains](#using-control-file-relaydomains)
      * [Using control file relaymailfrom](#using-control-file-relaymailfrom)
   * [CHECKRECIPIENT - Check Recipients during SMTP](#checkrecipient---check-recipients-during-smtp)
   * [SMTP Access List](#smtp-access-list)
   * [IndiMail Control Files Formats](#indimail-control-files-formats)
   * [Inlookup database connection pooling service](#inlookup-database-connection-pooling-service)
   * [Setting limits for your domain](#setting-limits-for-your-domain)
   * [SPAM and Virus Filtering](#spam-and-virus-filtering)
   * [SPAM Control using bogofilter](#spam-control-using-bogofilter)
   * [Training bogofilter](#training-bogofilter)
   * [SPAM Control using badip control file](#spam-control-using-badip-control-file)
   * [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi)
      * [Using tcprules](#using-tcprules)
      * [Using envdir for SMTP service under supervise(8)](#using-envdir-for-smtp-service-under-supervise8)
   * [Using spamassasin with IndiMail](#using-spamassasin-with-indimail)
   * [SPF implementation in IndiMail](#spf-implementation-in-indimail)
      * [SPF Macro Support](#spf-macro-support)
      * [SPF Macro Syntax](#spf-macro-syntax)
   * [SRS implementation in IndiMail](#srs-implementation-in-indimail)
      * [Configuration Parameters](#configuration-parameters)
   * [Greylisting in IndiMail](#greylisting-in-indimail)
      * [Enabling qmail-greyd greylisting server](#enabling-qmail-greyd-greylisting-server)
      * [Enabling greylisting in SMTP](#enabling-greylisting-in-smtp)
   * [Configuring dovecot as the IMAP/POP3 server](#configuring-dovecot-as-the-imappop3-server)
      * [Installation](#installation)
         * [For RPM based systems](#for-rpm-based-systems)
         * [For Debian based systems](#for-debian-based-systems)
      * [Configuring Authentication Methods](#configuring-authentication-methods)
         * [Basic Configuration](#basic-configuration)
         * [Use dovecot's SQL driver](#use-dovecots-sql-driver)
         * [Using checkpassword](#using-checkpassword)
         * [Using pam-multi](#using-pam-multi)
         * [Using indimail's Name Service Switch](#using-indimails-name-service-switch)
      * [Start dovecot](#start-dovecot)
      * [Migration from courier-imap to dovecot](#migration-from-courier-imap-to-dovecot)
         * [IMAP migration](#imap-migration)
         * [POP3 migration](#pop3-migration)
         * [Courier IMAP/POP3](#courier-imappop3)
         * [Dovecot configuration](#dovecot-configuration)
         * [Manual conversion](#manual-conversion)
   * [Configuring DKIM](#configuring-dkim)
      * [Create your DKIM signature](#create-your-dkim-signature)
      * [Create your DNS records](#create-your-dns-records)
      * [Set SMTP to sign with DKIM signatures](#set-smtp-to-sign-with-dkim-signatures)
      * [Set SMTP to verify DKIM signatures](#set-smtp-to-verify-dkim-signatures)
      * [DKIM sign during remote delivery](#dkim-sign-during-remote-delivery)
      * [DKIM sign during local delivery](#dkim-sign-during-local-delivery)
      * [DKIM sign during mail injection](#dkim-sign-during-mail-injection)
      * [DKIM Author Domain Signing Practices](#dkim-author-domain-signing-practices)
      * [Testing outbound signatures](#testing-outbound-signatures)
   * [iwebadmin – Web Administration of IndiMail](#iwebadmin--web-administration-of-indimail)
   * [Publishing statistics for IndiMail Server](#publishing-statistics-for-indimail-server)
   * [RoundCube Installation for IndiMail](#roundcube-installation-for-indimail)
   * [Setting up MySQL](#setting-up-mysql)
      * [Storing Passwords](#storing-passwords)
      * [Database Initialization](#database-initialization)
      * [MySQL Startup](#mysql-startup)
      * [Creating MySQL Users](#creating-mysql-users)
      * [Creating MySQL Configuration](#creating-mysql-configuration)
      * [MySQL Control File](#mysql-control-file)
      * [Configuring MySQL/MariaDB to use SSL/TLS](#configuring-mysqlmariadb-to-use-ssltls)
      * [Configure MySQL/MariaDB access for svctool](#configure-mysqlmariadb-access-for-svctool)
   * [Using Docker Engine to Run IndiMail / IndiMail-MTA](#using-docker-engine-to-run-indimail--indimail-mta)
   * [Installation &amp; Repositories](#installation--repositories)
      * [Installing Indimail using DNF/YUM/APT Repository](#installing-indimail-using-dnfyumapt-repository)
      * [Binary Builds on openSUSE Build Service and Copr](#binary-builds-on-opensuse-build-service-and-copr)
      * [Docker / Podman Repository](#docker--podman-repository)
      * [GIT Repository](#git-repository)
   * [Support Information](#support-information)
      * [IRC / Matrix](#irc--matrix)
      * [Mailing list](#mailing-list)
   * [Features](#features)
      * [Speed](#speed)
      * [Setup](#setup)
      * [Security](#security)
      * [Message construction](#message-construction)
      * [SMTP service](#smtp-service)
      * [Queue management](#queue-management)
      * [Bounces](#bounces)
      * [Routing by domain](#routing-by-domain)
      * [Remote SMTP delivery](#remote-smtp-delivery)
      * [Local delivery](#local-delivery)
      * [Other](#other)
      * [Brief Feature List](#brief-feature-list)
   * [Current Compilation status of all IndiMail &amp; related packages](#current-compilation-status-of-all-indimail--related-packages)
   * [Performance / Benchmarks](#performance--benchmarks)
      * [Observations](#observations)
      * [Results](#results)
   * [Credits](#credits)
   * [History](#history)
   * [See also](#see-also)

Created by [gh-md-toc](https://github.com/ekalinin/github-markdown-toc)

# INTRODUCTION

IndiMail is a messaging Platform comprising of multiple software packages including

[**indimail-virtualdomains**](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains")

* [virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains")
* [Email Parsing System](http://www.inter7.com/software/ "eps")

[**indimail-mta**](https://github.com/mbhangui/indimail-mta "indimail-mta")

* [qmail](http://cr.yp.to/qmail.html "qmail")
* [daemontools](https://cr.yp.to/daemontools.html "daemontools")
* [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp")
* [serialmail](https://cr.yp.to/serialmail.html "serialmail")
* [qmailanalog](http://cr.yp.to/qmailanalog.html "qmailanalog")
* [dot-forward](https://cr.yp.to/dot-forward.html "dot-foward")
* [fastforward](https://cr.yp.to/fastforward.html "fastforward")
* [mess822](https://cr.yp.to/mess822.html "mess822")
* [libtai](http://cr.yp.to/libtai.html "libtai")
* [dkim](https://en.wikipedia.org/wiki/DomainKeys_Identified_Mail "DKIM")
* [SRS2](https://en.wikipedia.org/wiki/Sender_Rewriting_Scheme "SRS2")

[**ezmlm-idx**](https://github.com/mbhangui/ezmlm-idx "ezmlm-idx")

* [ezmlm-idx mailing list manager](https://untroubled.org/ezmlm/ "ezmlm-idx")

[indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access "indimail-access")

* [courier IMAP/POP3](https://www.courier-mta.org/imap/ "courier-imap")
* [fetchmail](https://www.fetchmail.info "fetchmail")

[**indimail-auth**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-auth "indimail-auth")

* [nssd - providing Name Service Switch](https://github.com/mbhangui/indimail-virtualdomains/tree/master/nssd-x "nssd")
* [pam-multi - PAM modules for flexible, configurable authentication methods](https://github.com/mbhangui/indimail-virtualdomains/tree/master/pam-multi-x "pam-multi")

[**indimail-utils**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils "indimail-utils")

* Utilities ([altermime](http://pldaniels.com/altermime/ "altermime"), [ripMIME](https://pldaniels.com/ripmime/ "ripmime"), [mpack](https://github.com/mbhangui/indimail-virtualdomains/tree/master/mpack-x "mpack"), [fortune](https://en.wikipedia.org/wiki/Fortune_\(Unix\ "fortune") and [flash](https://github.com/mbhangui/indimail-virtualdomains/tree/master/flash-x "flash"))
* [logalert](https://github.com/mbhangui/indimail-virtualdomains/tree/master/logalert-x "logalert")

[**indimail-spamfilter**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/bogofilter-x "bogofilter")

* [bogofilter - A Bayesian Spam Filter](https://bogofilter.sourceforge.io/ "bogofilter")

Core **IndiMail** consists of three main packages - [indimail-mta](https://github.com/mbhangui/indimail-mta "indimail-mta"), [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains") and [indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access "indimail-access")

* **indimail-mta** is a re-engineered version of [qmail](http://cr.yp.to/qmail.html "qmail"). **indimail-mta** provides you a MTA with all the features and functionality of **[qmail](http://cr.yp.to/qmail.html "qmail")** plus many additional features. <b>indimail-mta</b> is an independent package. You can install it without requiring <b>indimail-virtualdomains</b> or <b>indimail-access</b>.
* **indimail-virtualdomains** provides you tools to create and manage multiple virtual domains with its own set of users, who can send and receive mails. <b>indimail-virtualdomains</b> is dependent on <b>indimail-mta</b> to function.
* **indimail-access** provides you IMAP/POP3 protocols using [courier-imap](https://www.courier-mta.org/imap/) & mail retrieval using [fetchmail](https://www.fetchmail.info/) program. It allows the users to access their emails delivered by <b>indimail-mta</b> locally on a host to users in /etc/passwd or to <b>virtual users</b> on a host (or multiple hosts) on the network having <b>virtual domains</b> created by <b>indimail-virtualdomains</b>. It is not a **MUST** that you have to install the indimail-access package. Since indimail-mta provides multiple authentication methods, protocols, PAM, NSS modules/plugins, you should be able to use your favourite IMAP/POP3 servier with indimail.

This document will refer to **IndiMail** as a combined package of indimail-virtualdomains, indimail-mta & other packages namely indimail-access, indimail-auth, indimail-utils, indimail-spamfilter.

When you install indimail-virtualdomains, a shared library from the package is dynamically loaded by indimail-mta to provide virtual domain support in indimail-mta, along with the ability to work with IMAP/POP3 retrieval daemons. The shared library is a file named <u>indimail.so</u> and is placed in your system directory for shared libraries (<u>/lib</u>, <u>/lib64</u>, etc).

indimail-virtualdomains provides programs to manage multiple virtual domains on a single host. It allows extending any of the domains across multiple servers. With indimail-mta installed, two compoments - **qmail-rspawn**, **qmail-remote** can act as an SMTP router/interceptor. Both the components use a **MySQL** database to know the location of each and every user. Similarly, the Mail Delivery Agent (MDA) **vdelivermail** uses the same MySQL database, allowing it to re-route any email to any server where the user's mailbox is present. Additionally, indimail-virtualdomains provide **proxyimap** and **proxypop3** - proxies for IMAP and POP3 protocols, to retrieve the user's mailbox from any server. To deliver or retrieve any email, the user doesn't have to connect to any specific serer. This is a very powerful feature which allows IndiMail to provide native horizontal scalability. It knows the location of each and every user and can distribute or retrieve mails for any user residing on any server located anywhere on the network. If one uses IndiMail, one can simply add one more server and cater to more users without changing any configuration, software or hardware of existing servers. To add a server is so easy. You setup a new server, copy a file named <u>mcdinfo</u> from an existing server, add an entry to the <u>mcdinfo</u> file and run the <b>dbinfo</b> command and instantaly the new server becomes part of the existing cluster and all other hosts in the cluster become aware of the new server. To remove a server is also very easy. Just edit and remove entry for a host in the <u>mcdinfo</u> file on any host and run the <u>dbinfo</u> command.

Since the location of every user is known, the architecure does not force a filesystem architecture like NFS to be used for provisioning large number of users (typically in an ISP/MSP environment). In the architecture below, you can keep on increasing the number of servers (incoming relay, outgoing relay or mailstores) to cater to large number of users. This allows you to scale indimail easily to serve millions of users using commodity hardware. You can read the feature list to get an idea of the changes and new features that have been added over the original packages. You can see a pictorial representation of the architecture below

![architecture](indimail_arch.png "IndiMail Architecture")

IndiMail allows servers to be distributed anywhere geographically. This is useful especially if you have users at different parts of the globe. e.g. Your Brazil users can have their server located in Brazil, Bombay users in Bombay, Delhi users in Delhi. And yet when your Brazil users comes to Delhi for a visit, he or she can access all emails sitting in Delhi by accessing the Delhi server. IndiMail provides this distributed feature using proxies for SMTP, IMAP and POP3 protocols. The proxy servers run using [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp") and are by default configured under supervise. You can use any IMAP/POP3 server behind the proxy. You can extend the domain across multiple servers without using any kind of NAS storage. Without something like indimail, to horizontally scale your email domain across multiple hosts, you will have to have your storage accessible on all hosts which do mail delivery or mail retrieval.

The ability of IndiMail to know user's location also allows IndiMail to setup a heterogeneous messaging environment. If you have IndiMail, you can have a server running MS exchange, few servers running IBM Lotus Notes, and few servers running IndiMail and all using a single domain. A utility called `hostcntrl` allows you to add foreign users to IndiMail's database. This feature also allows you to migrate your users from a proprietary platform to IndiMail without causing downtime or disruption to existing users. In fact, this method has been used very successfully in migrating corporate users out of MS Exchange & IBM Lotus Notes to IndiMail without the end users realizing it.

To migrate from an existing proprietary mail server like MS Exchange requires 5 steps.

1. You simply set up a new installation with IndiMail and create the existing domain using **vadddomain**.
2. Add the IP address of the Exchange Server in ***host_table*** and the SMTP port of the Exchange Server in the table ***smtp_port***.
3. Add users on the exchange server to a table called ***hostcntrl*** (either manually or using the utility **hostcntrl**).
4. Modify your user's mail configuration to use SMTP, IMAP Proxy, POP3 Proxy ports on the IndiMail server (**proxypop3**, **proxyimap**)
5. Change the MX to point to the indimail-mta server.

IndiMail is highly configurable. No hard-coded directories of qmail like /var/qmail/control or /var/qmail/queue. All directories are configurable at run time. IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration. To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance. Multiple queues help you to distribute your IO load across multiple disks at software level. On older filesystems which have performance issues with large number of files in a single directory, the multi-queue property eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). On a modern filesystem like [EXT4](https://en.wikipedia.org/wiki/Ext4), [ZFS](https://en.wikipedia.org/wiki/ZFS), [BTRFS](https://en.wikipedia.org/wiki/Btrfs), [XFS](https://en.wikipedia.org/wiki/XFS) you can do good with just a single queue. You can see a pictorial representation of the queue as well as read the [indimail-mta INTERNALS](#indimail-mta-internals).

IndiMail is a pure messaging solution. It does not provide calendars, todo lists, address books, meeting requests and a web mail front-end. However, you can use [RoundCubemail](https://roundcube.net/ "RoundCubemail") or any web mail front-end that works with IMAP or POP3 protocol with IndiMail. If you decide to install [RoundCubemail](https://roundcube.net/ "RoundCubemail"), you can install the **ircube** package from the IndiMail's DNF/YUM/Debian [Repository](https://build.opensuse.org/package/show/home:indimail/ircube "ircube") to have a fully functional web mail front-end. The ircube package provides plugins for Rouncube Mail to manage your passwords, vacation and SPAM filters.

IndiMail administrators can use a web administration tool called **iwebadmin**. It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/master/iwebadmin-x "iwebadmin") or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/iwebadmin "iwebadmin Repository") on the openSUSE build Service.

IndiMail comes with a tcl/tk administration client called [**indium**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indium-x "indium"). It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indium-x "indium") or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/indium "indium repository") on the openSUSE Build Service.

To install IndiMail you can take the help of the following documents in the [indimail-x/doc subdirectory](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-x/doc "Documents Subdirectory") of [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains") github repository. You can also jump to the section [Installing Indimail using DNF/YUM/APT Repository](#installing-indimail-using-dnfyumapt-repository) towards the bottom of this document.

File | Description
---- | --------------------
README-indimail.md|Introduction to IndiMail (this file which is also available as a wiki)
[INSTALL-indimail.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-indimail.md)|Detailed Source Installation Instructions. A simpler version is [this](https://github.com/mbhangui/indimail-virtualdomains/blob/master/README.md)
[INSTALL-RPM.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-RPM.md)|Install Instructions using RPM
[INSTALL-MYSQL.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-MYSQL.md)|MySQL specific Installation Instructions
[README-CLUSTER.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/README-CLUSTER.md)|Steps on configuring a clustered setup
[Quick-INSTALL.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/Quick-INSTALL.md)|A minimal documentation on Installation/Configuration
[Man Pages](https://github.com/mbhangui/indimail-virtualdomains/wiki/1-Man-Pages)|Link to Man Pages for all General commands, administrative commands, file formats, APIs and Miscelleneous topics

If you desire to do a full source installation and have enough experience on your belt, you can dive into this [document](https://github.com/mbhangui/indimail-virtualdomains/blob/master/README.md "README").

Once you have installed IndiMail, you will find all man pages in /usr/share/man and documents in /usr/share/indimail/doc. You can do read the man page for indimail-mta, indimail in section 7 of man pages, to get started with understanding IndiMail.

```
$ man indimail-mta
$ man indimail
$ man indimail-control
```

#DISCLAIMER

There is no warranty implied or otherwise with this package. I believe in OpenSource Philosophy and this is an attempt to give back to the OpenSource community. I welcome anyone who can report bugs or provide some assistance for building few missing features (building new features, testing and documentation).

# LICENSING

IndiMail uses GPLv3 License. See file [LICENSE](https://github.com/mbhangui/indimail-virtualdomains/blob/master/LICENSE "GPLv3"). Additional licenses (if any) may be found in subfolder of each component that IndiMail uses. If you need some other license for your use, let me know.

# TERMINOLOGY used for commands

```
$ command      - command was executed by a non-privileged user
# command      - command was executed by the `root` user
$ sudo command - command requires root privilege to run. sudo was used to gain root privileges
```

# indimail-mta Internals

Here's the data flow in the indimail-mta suite:

```
 qmail-smtpd ----> qmail-queue, qmail-multi   ---> todo-proc
               /    or qmail-queue clients        /   \
qmail-inject _/                                  /     \___ qmail-clean
                              __________________/
                             |
                             |       .--> qmail-rspawn ---> qmail-remote
                       qmail-send -->.
                                     .--> qmail-lspawn ---> qmail-local

```

As a minimum, you an go through the following online man pages - [qmail-smtpd](https://github.com/mbhangui/indimail-mta/wiki/qmail-smtpd.8), [qmail-inject](https://github.com/mbhangui/indimail-mta/wiki/qmail-inject.8), [sendmail](https://github.com/mbhangui/indimail-mta/wiki/isendmail.8), [qmail-queue](https://github.com/mbhangui/indimail-mta/wiki/qmail-queue.8), [qmail-multi](https://github.com/mbhangui/indimail-mta/wiki/qmail-multi.8), [qmail-queue-clients](https://github.com/mbhangui/indimail-mta/wiki/qmail-queue-clients.7), [qmail-internals](https://github.com/mbhangui/indimail-mta/wiki/qmail-internals.8), [qmail-send](https://github.com/mbhangui/indimail-mta/wiki/qmail-send.8), [todo-proc](https://github.com/mbhangui/indimail-mta/wiki/todo-proc.8), [qmail-lspawn](https://github.com/mbhangui/indimail-mta/wiki/qmail-lspawn.8), [qmail-local](https://github.com/mbhangui/indimail-mta/wiki/qmail-local.8), [qmail-rspawn](https://github.com/mbhangui/indimail-mta/wiki/qmail-rspawn.8), [qmail-remote](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8), [dot-qmail](https://github.com/mbhangui/indimail-mta/wiki/dot-qmail.5), [qmail-clean](https://github.com/mbhangui/indimail-mta/wiki/qmail-clean.8), [qscheduler](https://github.com/mbhangui/indimail-mta/wiki/qscheduler.8) and [indimail-control](https://github.com/mbhangui/indimail-mta/wiki/indimail-control.5).

The diagram below shows how <b>qmail-multi</b>(8) and <b>qmail-queue</b>(8) works ![qmail-multi](qmail_multi.png)

Every message is added to a central queue directory by <b>qmail-queue</b>. <b>qmail-queue</b> is called by programs like <b>qmail-smtpd</b>, <b>qmail-qmqpd</b>, <b>qmail-qmtpd</b>, <b>qmail-inject</b>, <b>sendmail</b> to submit messages to the queue. One can also use <b>qmail-multi</b> to queue the message. The main purpose of <b>qmail-multi</b> is to select a queue as discussed in [IndiMail Queue Mechanism](#indimail-queue-mechanism) and run the message through a filter. Here is a pictorial representation of the IndiMail queue. ![Pictorial](indimail_queue.png)

Every message is then pre-processed by <b>todo-proc</b> and then processed by <b>qmail-send</b> for delivery, in cooperation with <b>qmail-lspawn</b>, <b>qmail-rspawn</b> and cleaned up by <b>qmail-clean</b>. These five programs are long-running daemons. The diagram also shows a separate queue named <b>slowq</b>. This queue is special. It is a single queue that has <b>slowq-send</b> processing it instead of <b>todo-proc</b>, <b>qmail-send</b> pair. This queue has a feature where the deliveries can be rate controlled. <b>slowq-send</b> is like the orignal qmail's <b>qmail-send</b> and unlike it, <b>slowq-send</b> does both pre-processing and scheduling and is not as fast as <b>qmail-send</b> and hence the name. However the queue <b>slowq</b> ain't a queue where we require speed, and so it is ok.

The queue is designed to be crash-proof, provided that the underlying filesystem is crash-proof. All cleanups are handled by <b>qmail-send</b> and <b>qmail-clean</b> without human intervention.

## Programs

### qmail-multi

<b>qmail-multi</b> is a frontend for <b>qmail-queue</b>. It selects one of the multiple queues, sets the environment variable <b>QUEUEDIR</b> and calls <b>qmail-queue</b>. It can additionally run the message through a filter before passing th emessage to <b>qmail-queue</b>. <b>qmail-multi</b> is discussed in detail in [IndiMail Queue Mechanism](#indimail-queue-mechanism). In the current version of IndiMail, <b>qmail-queue</b> can do what <b>qmail-multi</b> does other than filter the message. So if one doesn't have any filter defined by FILTERARGS environment variable, <b>qmail-multi</b> is not required.

### qmail-queue

indimail-mta has multiple queues. Each queue is represented by a number <u>X</u>, where <u>X</u> is the number 1, 2, upto <u>N</u>, where <u>N</u> is the total number of queues configured. Each message in any of these queues is identified by a unique number, let's say 3016451. This number is related to the inode number of file created in queueX/pid directory. More on that below. From now on, we will refer to 3016451 as <u>inode</u> The queue is organized into several directories, each of which may contain files related to message 3016451:

file|Description
----|-----------
queueX/mess/<u>inode</u>|the message
queueX/todo/<u>inode</u>|the envelope: where the message came from, where it's going
queueX/intd/<u>inode</u>|the envelope, under construction by <b>qmail-queue</b>
queueX/info/<u>inode</u>|the envelope sender address, after preprocessing
queueX/local/<u>inode</u>|local envelope recipient addresses, after preprocessing
queueX/remote/<u>inode</u>|remote envelope recipient addresses, after preprocessing
queueX/bounce/<u>inode</u>|permanent delivery errors

Here are all possible states for a message.
* \+ means a file exists
* \- means it does not exist
* \? means it may or may not exist.

Message State|Possible states
-------------|---------------
S1|-mess -intd -todo -info -local -remote -bounce
S2|+mess -intd -todo -info -local -remote -bounce
S3|+mess +intd -todo -info -local -remote -bounce
S4|+mess ?intd +todo ?info ?local ?remote -bounce (queued)
S5|+mess -intd -todo +info ?local ?remote ?bounce (preprocessed)

Guarantee: If queueX/mess/<u>inode</u> exists, it has inode number <u>inode</u>.

<b>qmail-queue</b>'s job is to accept a message from a client and submit it to the queue. It reads the message from file descriptor zero and the envelope from file descriptor one.

<b>qmail-queue</b> adds a Received field to the message that looks like one of these

*   Received: indimail-mta queue 37166 by host argos.indimail.org (invoked by alias); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta queue 37166 by host argos.indimail.org (invoked by uid 555); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta queue 37166 by host argos.indimail.org (invoked by uid xxx); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta queue 37166 by host argos.indimail.org (invoked from network ::1, by uid 1024); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta queue 37166 by host argos.indimail.org (invoked for bounce); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-virtual 37166 by host argos.indimail.org (invoked by uid 555); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta smtpd 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta qmtpd 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta qmqpd 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta notify 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256); Fri, 21 Oct 2022 23:04:36 +0530
*   Received: indimail-mta mini-smtpd 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256) ESMTPSA; Fri, 21 Oct 2022 23:04:36 +0530

where:

* 37166 is <b>qmail-queue</b>'s process ID.
* invoked by alias means <b>qmail-queue</b> was invoked by a process with uid of alias user. This will be through <b>qmail-local</b> reading & processing a dot-qmail file
* invoked by uid xxx <b>qmail-queue</b> was invoked by a process with uid of xxx. This will be through <b>qmail-inject</b>, <b>sendmail</b> or any user run program that calls <b>qmail-queue</b>. The uid of the user is xxx. When the uid is 555, it will be delivery to a virtual domain address.
* invoked from network means <b>qmail-queue</b> was invoked by user qmaild received from ::1. This means the mail was sent using SMTP by connecting to the IP ::1.
* invoked from bounce means that this was a bounce generated by <b>qmail-send</b> running under uid qmails
* Received: indimail-virtual 37166 by host argos.indimail.org (invoked by uid 555). This is the Received line put by <b>vdelivermail</b> delivering to a virtual domain address.
* Received: indimail-mta smtpd 37166 (HELO argos.indimail.org) (testuser01@example.com@::1) by argos.indimail.org with ESMTPSA (TLSv1.3 TLS_AES_256_GCM_SHA384 bits=256) - This is the Received line put by <b>qmail-smtpd</b> which tells the HELO argument was argos.indimail.org, using bits=256 channel with TLSv1.3 and TLS_AES_256_GCM_SHA384 cipher. Also this was an authenticated SMTP session using the account testuser01@example.com
* Fri, 21 Oct 2022 23:04:36 +0530 is time and date (RFC 5322 format) at which <b>qmail-queue</b> created the message.

<b>qmail-queue</b> places a messages in the queue in four stages:

1. To add a message to the queue, <b>qmail-queue</b> first creates a file in a separate directory, queueX/pid, with a unique name. The filesystem assigns that file a unique inode number. <b>qmail-queue</b> looks at that number, say 3016451. By the guarantee above, message 3016451 must be in state S1.
2. The queueX/pid file is renamed to queueX/mess/split/<u>inode</u>, and the message is written to the file, moving to state S2. Here split is the remainder left from dividing inode number by the compile time conf-split value. For example, if inode is 3016451 and conf-split is the default, 151, then split is 75 (3016451 divided by 151 is 19976 which gives a remainder of (3016451 - 19976 * 151) = 75)
3. The file queueX/intd/<u>inode</u> is created and the envelope is written to it in the form

	`u1011\0p28966\0Ftuser@example.com\0Tuser1@a.com\0Tuser2@b.com\0`

	It means the above message was sent by user tuser@example.com with uid 1011, process ID 28966 to two users <u>user1@a.com</u>, <u>user2@b.com</u>. At this point, we have moved to state S3
4. queueX/todo/<u>inode</u> is linked to queueX/intd/<u>inode</u>, moving the state to S4. At this instant, message has been successfully queued for further classification into local or remoe delivery by <b>todo-proc</b>. After classification, <b>todo-proc</b> communicates the inode number to <b>qmail-send</b>for delivery.

At the moment queueX/todo/<u>inode</u> is created, the message has been queued. <b>qmail-send</b> eventually (within 25 minutes notices the new message, but to speed things up, <b>qmail-queue</b> writes a single byte to lock/trigger, a named pipe that <b>qmail-send</b> watches. When trigger contains readable data, qmail- send is awakened, empties trigger, and scans the todo directory.

<b>qmail-queue</b> starts a 24-hour timer before touching any files, and commits suicide if the timer expires.

Once a message is deposited in one of the indimail's queues, it will be sent by few programs working cooperatively. We will now look at qscheduler, qmail-start, <b>qmail-send</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, <b>qmail-local</b>, and <b>qmail-remote</b>.

### qscheduler, qmail-start, slowq-start, qmta-send

<b>qscheduler</b> invokes multiple instances of <b>qmail-start</b> to invoke <b>qmail-send</b>, <b>todo-proc</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, and <b>qmail-clean</b> under the proper uids and gids for each of the multiple queues. The number of instances it runs is defined by the environment variable <b>QUEUE\_COUNT</b>. For each instance the queue is defined by <b>qscheduler</b> by setting the environment variable <b>QUEUEDIR</b>. A queue is defined by the integers defined by environment variables <b>QUEUE\_START</b> and <b>QUEUE\_COUNT</b> as described in section [IndiMail Queue Mechanism](#indimail-queue-mechanism). <b>qscheduler</b> also monitors <b>qmail-send</b> and <b>todo-proc</b> and restart them if they go down. Apart from having a fixed number of queues defined by <b>QUEUE\_COUNT</b>, <b>qscheduler</b> can set the number of queues dynamically based on queue load. The dynamic queue option can be set by passing \-d option to <b>qscheduler</b>

<b>qmail-start</b> invokes <b>qmail-send</b>, <b>todo-proc</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, and <b>qmail-clean</b>, under the proper uids and gids for a single queue. These five daemons cooperate to deliver messages from the queue. <b>qmail-start</b> should be used if you desire to run only one queue. For running multiple parallel queues run <b>qscheduler</b>.

<b>slowq-start</b> invokes <b>slowq-send</b>, <b>qmail-lspawn></b>, <b>qmail-rspawn</b>, and <b>qmail-clean</b>, under the proper uids and gids for a single queue. <b>slowq-send</b> is a special daemon that does the work of both <b>todo-proc</b> and <b>qmail-send</b> but handles a single, special queue named <u>slowq</u>. It also has an inbuilt dedicated todo processor and doesn't require a seperate <b>todo-proc</b> process. These four daemons cooperate to deliver messages from the queue with control on the delivery rates. We will talk about this in the chapter [Controlling Delivery Rates](#controlling-delivery-rates).

<b>qmta-send</b> does the work of <b>todo-proc</b>, <b>qmail-send</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b> and <b>qmail-clean</b> in a single daemon. It handles a single special queue named <u>qmta</u>. You can use <b>qmta-send</b> instead of <b>todo-proc</b>/<b>qmail-send</b> for small systems which have negligible or sporadic mail traffic. Single Board Computers are an excellent fit for <b>qmta-send</b>. <b>qmta-send</b> can be started on the command line, in RC scripts or in cron/shell scripts. It can be invoked to do just one time delivery without running as a daemon. It can also be enabled to start at boot by enabling <b>qmta-service</b> <b>systemd.unit(5)</b> configuration using the <b>systemctl(1)</b> command. You can learn more about <b>qmta-send</b> in the chapter [qmta - Using a minimal standalone qmta-send MTA](#qmta---using-a-minimal-standalone-qmta-send-mta)

<b>qscheduler</b>, <b>qmail-start</b>, <b>slowq-start</b>, <b>qmta-send</b> can be passed an argument - <b>defaultdelivery</b>. If <b>defaultdelivery</b> supplied, <b>qmail-start</b> or <b>qscheduler</b> passes it to <b>qmail-lspawn</b>. You can also have a control file named defaultdelivery. The mailbox type is picked up from the <b>defaultdelivery</b> control file. The table below outlines the choices for <b>defaultdelivery</b> control file

Mailbox Format |Name|Location|defaultdelivery|Comments
---------------|----|--------|---------------|--------
mbox|Mailbox|$HOME|./Mailbox|most common, works with most MUAs
maildir|Maildir|$HOME|./Maildir/|more reliable, less common MUA support
mbox|username|/var/spool/mail|See INSTALL.vsm|traditional mailbox

### todo-proc, qmail-send

Once a message has been queued, <b>todo-proc</b> must decide which recipients are local and which recipients are remote. It may also rewrite some recipient addresses. <b>todo-proc</b>/<b>qmail-send</b> process messages in the queue and pass them to <b>qmail-rspawn</b> and <b>qmail-lspawn</b>. We will talk about their function in the order that a message in the queue would experience them: preprocessing, delivery, and cleanup.

#### Preprocessing

<b>todo-proc</b> does the preprocessing and like queuing, this is done in stages:

1. When <b>todo-proc</b> notices queueX/todo/<u>inode</u>, it knows that the message <u>inode</u> is in S4. <b>todo-proc</b> deletes queueX/info/split/<u>inode</u>, queueX/local/split/<u>inode</u>, and queueX/remote/split/<u>inode</u>, if they exist. Then it reads through queueX/todo/<u>inode</u>.
2. A new queueX/info/split/inode is created, containing the envelope sender address.
3. If the message has local recipients, they're added to queueX/local/split/<u>inode</u>.
4. If the message has remote recipients, they're added to queueX/remote/split/<u>inode</u>.
5. queueX/intd/<u>inode</u> is deleted. The message is still in S4 at this point.
6. queueX/todo/<u>inode</u> is deleted, moving to stage S5. At this instant the message has been succcesfully preprocessed. Recipients are considered local if the domain is listed in control/locals or the entire recipient or domain is listed in control/virtualdomains. If the recipient is virtual, the local part ofthe address is rewritten as specified in virtualdomains.

#### Delivery

Messages at S5 are handled as follows. Initially, all recipients in queueX/local/split/<u>inode</u> and queueX/remote/split/<u>inode</u> are marked NOT DONE, meaning that <b>qmail-send</b> should attempt to deliver to them. On its own schedule, <b>qmail-send</b> sends delivery commands to <b>qmail-lspawn</b> and <b>qmail-rspawn</b> using channels set up by qmail-start. When it receives responses from <b>qmail-lspawn</b> or <b>qmail-rspawn</b> that indicate successful delivery or permanent error, <b>qmail-send</b> changes their status in queueX/local/split/<u>inode</u> or queueX/remote/split/<u>inode</u> to DONE, meaning that it should not attempt further deliveries. When <b>qmail-send</b> receives a permanent error, it also records that in queueX/bounce/split/<u>inode</u>. Bounce messages are also handled on <b>qmail-send</b>'s schedule. Bounces are handled by injecting a bounce message based on queueX/mess/split/<u>inode</u> and queueX/bounce/split/<u>inode</u>, and deleting queueX/bounce/split/<u>inode</u>. When all ofthe recipients in queueX/local/split/<u>inode</u> or queueX/remote/split/<u>inode</u> are marked DONE, the respective local or remote file is removed.

<b>qmail-send</b> may at its leisure try to deliver a message to a NOT DONE address. If the message is successfully delivered, <b>qmail-send</b> marks the address as DONE. If the delivery attempt meets with permanent failure, <b>qmail-send</b> first appends a note to queueX/bounce/split/<u>inode</u>, creating queueX/bounce/split/<u>inode</u> if necessary; then it marks the address as DONE. Note that queueX/bounce/split/<u>inode</u> is not crash-proof.

<b>qmail-send</b> may handle queueX/bounce/split/<u>inode</u> at any time, as follows: it

1. injects a new bounce message, created from queueX/bounce/split/<u>inode</u> and queueX/mess/split/<u>inode</u>;
2. deletes queueX/bounce/split/<u>inode</u>.

When all addresses in queueX/local/split/<u>inode</u> are DONE, <b>qmail-send</b> deletes queueX/local/split/<u>inode</u>. Same for queueX/remote/split/<u>inode</u>. 

When queueX/local/split/<u>inode</u> and queueX/remote/split/<u>inode</u> are gone, <b>qmail-send</b> eliminates the message, as follows. First, if queueX/bounce/split/<u>inode</u> exists, <b>qmail-send</b> handles it as described above. Once queueX/bounce/split/<u>inode</u> is definitely gone, <b>qmail-send</b> deletes queueX/info/split/<u>inode</u>, moving to S2, and finally queueX/mess/split/<u>inode</u>, moving to S1.

##### Retry Schedules

Each message has its own retry schedule. The longer a message remains undeliverable, the less frequently qmail tries to send it. The retry schedule is not configurable. The tables below show the retry schedule for a message that's undeliverable to a recipient until it bounces (default queuelifetime of 604800 seconds). Local messages a similar, but more frequent, schedule than remote messages. <b>qmail-send</b> uses a simple formula to determine the times at which messages in the queue are retried. If attempts is the number of failed delivery attempts so far, and birth is the time at which a message entered the queue (determined from the creation time ofthe queue/info file), then:

nextretry = birth + (attempts \* <u>chanskip</u>) \* (attempts \* <u>chanskip</u>)

where <u>chanskip</u> is a retry factor equal to 10 for local deliveries and 20 for remote deliveries.

* Local Message Retry Schedule

```
   qmail-send delivery retry times, for chanskip=10 (local)
Attempt ======= after =======     == delay until next =
       seconds  dd hh mm ss       seconds  dd hh mm ss
...................... ....................... ...................... 
   #00       0 [00 00:00:00]           100 [00 00:01:40]
   #01     100 [00 00:01:40]           300 [00 00:05:00]
   #02     400 [00 00:06:40]           500 [00 00:08:20]
   #03     900 [00 00:15:00]           700 [00 00:11:40]
   #04    1600 [00 00:26:40]           900 [00 00:15:00]
   #05    2500 [00 00:41:40]          1100 [00 00:18:20]
   #06    3600 [00 01:00:00]          1300 [00 00:21:40]
   #07    4900 [00 01:21:40]          1500 [00 00:25:00]
   #08    6400 [00 01:46:40]          1700 [00 00:28:20]
   #09    8100 [00 02:15:00]          1900 [00 00:31:40]
   #10   10000 [00 02:46:40]          2100 [00 00:35:00]
   #11   12100 [00 03:21:40]          2300 [00 00:38:20]
   #12   14400 [00 04:00:00]          2500 [00 00:41:40]
   #13   16900 [00 04:41:40]          2700 [00 00:45:00]
   #14   19600 [00 05:26:40]          2900 [00 00:48:20]
   #15   22500 [00 06:15:00]          3100 [00 00:51:40]
   #16   25600 [00 07:06:40]          3300 [00 00:55:00]
   #17   28900 [00 08:01:40]          3500 [00 00:58:20]
   #18   32400 [00 09:00:00]          3700 [00 01:01:40]
   #19   36100 [00 10:01:40]          3900 [00 01:05:00]
   #20   40000 [00 11:06:40]          4100 [00 01:08:20]
   #21   44100 [00 12:15:00]          4300 [00 01:11:40]
   #22   48400 [00 13:26:40]          4500 [00 01:15:00]
   #23   52900 [00 14:41:40]          4700 [00 01:18:20]
   #24   57600 [00 16:00:00]          4900 [00 01:21:40]
   #25   62500 [00 17:21:40]          5100 [00 01:25:00]
   #26   67600 [00 18:46:40]          5300 [00 01:28:20]
   #27   72900 [00 20:15:00]          5500 [00 01:31:40]
   #28   78400 [00 21:46:40]          5700 [00 01:35:00]
   #29   84100 [00 23:21:40]          5900 [00 01:38:20]
   #30   90000 [01 01:00:00]          6100 [00 01:41:40]
   #31   96100 [01 02:41:40]          6300 [00 01:45:00]
   #32  102400 [01 04:26:40]          6500 [00 01:48:20]
   #33  108900 [01 06:15:00]          6700 [00 01:51:40]
   #34  115600 [01 08:06:40]          6900 [00 01:55:00]
   #35  122500 [01 10:01:40]          7100 [00 01:58:20]
   #36  129600 [01 12:00:00]          7300 [00 02:01:40]
   #37  136900 [01 14:01:40]          7500 [00 02:05:00]
   #38  144400 [01 16:06:40]          7700 [00 02:08:20]
   #39  152100 [01 18:15:00]          7900 [00 02:11:40]
   #40  160000 [01 20:26:40]          8100 [00 02:15:00]
   #41  168100 [01 22:41:40]          8300 [00 02:18:20]
   #42  176400 [02 01:00:00]          8500 [00 02:21:40]
   #43  184900 [02 03:21:40]          8700 [00 02:25:00]
   #44  193600 [02 05:46:40]          8900 [00 02:28:20]
   #45  202500 [02 08:15:00]          9100 [00 02:31:40]
   #46  211600 [02 10:46:40]          9300 [00 02:35:00]
   #47  220900 [02 13:21:40]          9500 [00 02:38:20]
   #48  230400 [02 16:00:00]          9700 [00 02:41:40]
   #49  240100 [02 18:41:40]          9900 [00 02:45:00]
   #50  250000 [02 21:26:40]         10100 [00 02:48:20]
   #51  260100 [03 00:15:00]         10300 [00 02:51:40]
   #52  270400 [03 03:06:40]         10500 [00 02:55:00]
   #53  280900 [03 06:01:40]         10700 [00 02:58:20]
   #54  291600 [03 09:00:00]         10900 [00 03:01:40]
   #55  302500 [03 12:01:40]         11100 [00 03:05:00]
   #56  313600 [03 15:06:40]         11300 [00 03:08:20]
   #57  324900 [03 18:15:00]         11500 [00 03:11:40]
   #58  336400 [03 21:26:40]         11700 [00 03:15:00]
   #59  348100 [04 00:41:40]         11900 [00 03:18:20]
   #60  360000 [04 04:00:00]         12100 [00 03:21:40]
   #61  372100 [04 07:21:40]         12300 [00 03:25:00]
   #62  384400 [04 10:46:40]         12500 [00 03:28:20]
   #63  396900 [04 14:15:00]         12700 [00 03:31:40]
   #64  409600 [04 17:46:40]         12900 [00 03:35:00]
   #65  422500 [04 21:21:40]         13100 [00 03:38:20]
   #66  435600 [05 01:00:00]         13300 [00 03:41:40]
   #67  448900 [05 04:41:40]         13500 [00 03:45:00]
   #68  462400 [05 08:26:40]         13700 [00 03:48:20]
   #69  476100 [05 12:15:00]         13900 [00 03:51:40]
   #70  490000 [05 16:06:40]         14100 [00 03:55:00]
   #71  504100 [05 20:01:40]         14300 [00 03:58:20]
   #72  518400 [06 00:00:00]         14500 [00 04:01:40]
   #73  532900 [06 04:01:40]         14700 [00 04:05:00]
   #74  547600 [06 08:06:40]         14900 [00 04:08:20]
```

* Remote Message Retry Schedule

```
   qmail-send delivery retry times, for chanskip=20 (remote)
Attempt ======= after =======      == delay until next =
       seconds  dd hh mm ss        seconds  dd hh mm ss
   #00       0 [00 00:00:00]           400 [00 00:06:40]
   #01     400 [00 00:06:40]          1200 [00 00:20:00]
   #02    1600 [00 00:26:40]          2000 [00 00:33:20]
   #03    3600 [00 01:00:00]          2800 [00 00:46:40]
   #04    6400 [00 01:46:40]          3600 [00 01:00:00]
   #05   10000 [00 02:46:40]          4400 [00 01:13:20]
   #06   14400 [00 04:00:00]          5200 [00 01:26:40]
   #07   19600 [00 05:26:40]          6000 [00 01:40:00]
   #08   25600 [00 07:06:40]          6800 [00 01:53:20]
   #09   32400 [00 09:00:00]          7600 [00 02:06:40]
   #10   40000 [00 11:06:40]          8400 [00 02:20:00]
   #11   48400 [00 13:26:40]          9200 [00 02:33:20]
   #12   57600 [00 16:00:00]         10000 [00 02:46:40]
   #13   67600 [00 18:46:40]         10800 [00 03:00:00]
   #14   78400 [00 21:46:40]         11600 [00 03:13:20]
   #15   90000 [01 01:00:00]         12400 [00 03:26:40]
   #16  102400 [01 04:26:40]         13200 [00 03:40:00]
   #17  115600 [01 08:06:40]         14000 [00 03:53:20]
   #18  129600 [01 12:00:00]         14800 [00 04:06:40]
   #19  144400 [01 16:06:40]         15600 [00 04:20:00]
   #20  160000 [01 20:26:40]         16400 [00 04:33:20]
   #21  176400 [02 01:00:00]         17200 [00 04:46:40]
   #22  193600 [02 05:46:40]         18000 [00 05:00:00]
   #23  211600 [02 10:46:40]         18800 [00 05:13:20]
   #24  230400 [02 16:00:00]         19600 [00 05:26:40]
   #25  250000 [02 21:26:40]         20400 [00 05:40:00]
   #26  270400 [03 03:06:40]         21200 [00 05:53:20]
   #27  291600 [03 09:00:00]         22000 [00 06:06:40]
   #28  313600 [03 15:06:40]         22800 [00 06:20:00]
   #29  336400 [03 21:26:40]         23600 [00 06:33:20]
   #30  360000 [04 04:00:00]         24400 [00 06:46:40]
   #31  384400 [04 10:46:40]         25200 [00 07:00:00]
   #32  409600 [04 17:46:40]         26000 [00 07:13:20]
   #33  435600 [05 01:00:00]         26800 [00 07:26:40]
   #34  462400 [05 08:26:40]         27600 [00 07:40:00]
   #35  490000 [05 16:06:40]         28400 [00 07:53:20]
   #36  518400 [06 00:00:00]         29200 [00 08:06:40]
   #37  547600 [06 08:06:40]         30000 [00 08:20:00]
   #38  577600 [06 16:26:40]         30800 [00 08:33:20]
   #39  608400 [07 01:00:00]         31600 [00 08:46:40]
   #40  640000 [07 09:46:40]         32400 [00 09:00:00]
   #41  672400 [07 18:46:40]         33200 [00 09:13:20]
   #42  705600 [08 04:00:00]         34000 [00 09:26:40]
   #43  739600 [08 13:26:40]         34800 [00 09:40:00]
   #44  774400 [08 23:06:40]         35600 [00 09:53:20]
   #45  810000 [09 09:00:00]         36400 [00 10:06:40]
   #46  846400 [09 19:06:40]         37200 [00 10:20:00]
   #47  883600 [10 05:26:40]         38000 [00 10:33:20]
   #48  921600 [10 16:00:00]         38800 [00 10:46:40]
   #49  960400 [11 02:46:40]         39600 [00 11:00:00]
   #50 1000000 [11 13:46:40]         40400 [00 11:13:20]
   #51 1040400 [12 01:00:00]         41200 [00 11:26:40]
   #52 1081600 [12 12:26:40]         42000 [00 11:40:00]
   #53 1123600 [13 00:06:40]         42800 [00 11:53:20]
   #54 1166400 [13 12:00:00]         43600 [00 12:06:40]
   #55 1210000 [14 00:06:40]         44400 [00 12:20:00]
   #56 1254400 [14 12:26:40]         45200 [00 12:33:20]
   #57 1299600 [15 01:00:00]         46000 [00 12:46:40]
   #58 1345600 [15 13:46:40]         46800 [00 13:00:00]
   #59 1392400 [16 02:46:40]         47600 [00 13:13:20]
   #60 1440000 [16 16:00:00]         48400 [00 13:26:40]
   #61 1488400 [17 05:26:40]         49200 [00 13:40:00]
   #62 1537600 [17 19:06:40]         50000 [00 13:53:20]
   #63 1587600 [18 09:00:00]         50800 [00 14:06:40]
   #64 1638400 [18 23:06:40]         51600 [00 14:20:00]
   #65 1690000 [19 13:26:40]         52400 [00 14:33:20]
   #66 1742400 [20 04:00:00]         53200 [00 14:46:40]
   #67 1795600 [20 18:46:40]         54000 [00 15:00:00]
   #68 1849600 [21 09:46:40]         54800 [00 15:13:20]
   #69 1904400 [22 01:00:00]         55600 [00 15:26:40]
   #70 1960000 [22 16:26:40]         56400 [00 15:40:00]
   #71 2016400 [23 08:06:40]         57200 [00 15:53:20]
   #72 2073600 [24 00:00:00]         58000 [00 16:06:40]
   #73 2131600 [24 16:06:40]         58800 [00 16:20:00]
   #74 2190400 [25 08:26:40]         59600 [00 16:33:20]
```
#### Cleanup

When both queueX/local/split/<u>inode</u> and queueX/remote/split/<u>inode</u> have been removed, the message is dequeued by:

1. Processing queueX/bounce/split/<u>inode</u>, if it exists.
2. Deleting queueX/info/split/<u>inode</u>.
3. Deleting queueX/mess/split/<u>inode</u>.

Partially queued and partially dequeued messages left when a system crash interrupts <b>qmail-queue</b> or <b>qmail-send</b> are deleted by <b>qmail-send</b> using <b>qmail-clean</b>, another long-running daemon started by <b>qmail-start</b>. Messages with a queueX/mess/split/<u>inode</u> file and possibly an queueX/intd/<u>inode</u> but no todo, info, local, remote, or bounce, are safe to delete after 36 hours because <b>qmail-queue</b> kills itself after 24 hours. Similarly, files in the pid directory more than 36 hours old are also deleted.

If the computer crashes while <b>qmail-queue</b> is trying to queue a message, or while <b>qmail-send</b> is eliminating a message, the message may be left in state S2 or S3.

When <b>qmail-send</b> sees a message in state S2 or S3, other than one it is currently eliminating, where queueX/mess/split/<u>inode</u> is more than 36 hours old, it deletes queueX/intd/split/<u>inode</u> if that exists, then deletes queueX/mess/split/<u>inode</u>. Note that any <b>qmail-queue</b> handling the message must be dead.

Similarly, when <b>qmail-send</b> sees a file in the queueX/pid directory that is more than 36 hours old, it deletes it.

Cleanups are not necessary if the computer crashes while <b>qmail-send</b> is delivering a message. At worst a message may be delivered twice. (There is no way for a distributed mail system to eliminate the possibility of duplication. What if an SMTP connection is broken just before the server acknowledges successful receipt of the message. The client must assume the worst and send the message again. Similarly, if the computer crashes just before <b>qmail-send</b> marks a message as DONE, the new <b>qmail-send</b> must assume the worst and send the message again. The usual solutions in the database literature e.g., keeping log files amount to saying that it's the recipient's computer's job to discard duplicate messages.)

#### Global & Queue Specific Concurrency, Parallelism limits

<b>qmail-lspawn</b> and <b>qmail-rspawn</b> can do multiple concurrent deliveries. The default concurrency limit is 5 for local deliveries and 10 for remote deliveries. These can be increased upto a maximum of 500 by setting it in the control files <b>concurrencylocal</b> for local deliveries and <b>concurrencyremote</b> for remote deliveries. These two (like any other indimail control files) lie in <u>/etc/indimail/control</u> directory. These concurrency limits are inherited by each of the indimail's multiple queues. Additionally indimail allows you to have queue specific concurrency limits. e.g. You can have the control files <b>concurrencyl.queue2</b>, <b>concurrencyr.queue2</b> for setting local, remote concurrency specific to <u>/var/indimail/queue/queue2</u>.

## Setting Environment Variables

indimail-mta can be fine tuned and configured using environment variables (> 250) of them. See the man page for indimail-env(5) and tcp-environ(5). This feature gives a significant edge to indimail-mta over other MTAs. It gives you the total flexibility to configure and customize indimail-mta. There are many methods of setting them.

1. Setting them in variables directory. All indimail services are configured as supervised services in <u>/service</u> directory. Each of these services have a directory named named after the service and a subdir inside it named <u>variables</u>. In the <u>variables</u> directory, you just need to create a file to create an environment variable. The name of the environment variable is the filename and the value of the environment variable is the content of the file. An empty file, removes the environment variable. As an exercise, explore the directory <u>/service/qmail-smtpd.25/variables</u>. All IndiMail services use the program <b>envdir</b>(8) to set environment variables using files in the <u>variables</u> directory. You can have a file named .<u>envdir</u> in the variables directory. This file can contain multiple lines with each line naming directories that should be looked up for setting additional environment variables. In fact, the default installation creates .<u>envdir</u> linking to an additional directory: <u>/etc/indimail/control/global_vars</u>. Any variables in <u>global_vars</u> serve as global environment variables for all services. You can also have .<u>envdir</u> as a directory or as a link to another directory having environment variables. Regardless of having .<u>envdir</u> as a file, directory or a symbolic link, there is a built-in safety mechansim that prevents infinite recursive traversal. You can also have environment variables as key=value pairs in a file named .<u>envfile</u>.

2. Using environment directory <u>defaultqueue</u> in <b>/etc/indimail/control</b> directory. This is just like the supervise <u>variables</u> directory. The environment variables configured in this directory get used when calling <b>qmail-inject</b>, <b>sendmail</b> and few other programs (See NOTE below). Read the man page for <b>qmail-inject</b>. Just like the <u>variables</u> directory mentioned above, you can have .<u>envdir</u> and .<u>envfile</u> in <u>defaultqueue</u> directory. The <u>defaultqueue</u> directory gets skipped if you have the <b>QUEUE_BASE</b> environment variable already set. <b>envdir</b> will exit 111 if it has trouble reading any directories when processing the <u>defaultqueue</u> directory or any extra directories because while processing. However, failure to read any environment variable file doesn't result in errors. This allows the administrator to set environment variables having access to specific users on the system. <b>envdir</b> will continue the processing skipping files for which it lacks permissions.

3. Using environment directory .<u>defaultqueue</u> in $HOME. This too is just like the supervise <u>variables</u> directory. The environment variables configured in this directory get used when calling <b>qmail-inject</b>, <b>sendmail</b>. Here $HOME refers to the home directory of the user and is totally under the control of the user to set it. Read the man page for <b>qmail-inject</b>. This directory too can have .<u>envdir</u> or .<u>envfile</u> to set additional environment variables. But unlike <u>defaultqueue</u>, any error processing this directory, additional directories from processig .<u>envdir</u> are ignored and no change is made to the existing set of environment variables. If you set <b>QUEUE_BASE</b> in this directory, then <u>/etc/indimail/control/defaultqueue</u> gets skipped, allowing you to override system configured environment variables. Programs that set environment variables from $HOME/.defaultqueue directory are condredirect, dot-forward, fastforward, filterto, forward, maildirserial, new-inject, qmail-inject, qmail-qread, qmail-showctl, qmonitor, qmta-send, qnotify, qreceipt, queue-fix, replier, rrforward and rrt. Out of these programs, qmail-inject, new-inject, qmail-qread and maildirserial will process .<u>defaultqueue</u> regardless of the uid with they run, whereas the remaining programs process .<u>defaultqueue</u> when running with non-zero uid. You can however skip .<u>defaultqueue</u> processing by setting <b>SKIP_LOCAL_ENVIRONMENT</b> environment variable.

4. Using control files <b>from.envrules</b>, <b>fromd.envrules</b>, <b>rcpt.envrules</b>, <b>auth.envrules</b> - These are control files used by programs like <b>qmail-smtpd</b>, <b>qmail-inject</b>. They match on the sender or recipient address. Here you can set or unset environment variables for a sender, recipient. You can also use any regular expression to match multipler sender or recipients. To know these environment variables, read the man pages for <b>qmail-smtpd</b>, <b>qmail-inject</b>, <b>spawn-filter</b>.

5. Using control file domainqueue - This can be used to set environment variable for any recipient domain. Read the man page for <b>qmail-smtpd</b>, <b>qmail-inject</b>. You can configure <b>domainqueue</b> to have indimail-mta configure itself differently for different domains.

6. If you have installed ezmlm / ezmlm-idx, then you have an additional directory for configuring environment variables in <u>/etc/indimail/ezmlm/global_vars</u>.

7. Using the program <b>ctrlenv</b>. <b>ctrlenv</b> can use a cdb file with simple assignments of variables to addresses and run a program with new environment variables. It works exactly like the simple and wildcard assignments in the qmail assign file. See the man page for <b>ctrlenv</b> and <b>cdb-database</b>.

8. Nothing prevents a user from writing a shell script to set environment variables before calling any of indimail-mta programs. If you are familiar with UNIX, you will know how to set them. The mechanism for setting environment variables in a shell depends on the shell you are using and is beyond the scope of this document. You can read the man pages for the shell that your script uses.

It is trivial to display the environment variable that would be set for your service by using the envdir command along with the env command. In fact this is what the `svctool --print-variables --service-name=xxxx` or `minisvc --print-variables --service-name=xxx` does internally. You can show environment variable set for a user (point 3 above) using the command `qmail-showctl -E`

Display environment variables set for defaultqueue

	$ envdir -c /etc/indimail/control/defaultqueue env
	USE_FSYNC=
	QUEUE_START=1
	QMAILQUEUE=/usr/sbin/qmail-dkim
	QUEUE_COUNT=5
	DKIMSIGN=/etc/indimail/control/domainkeys/%/default
	CONFSPLIT=151
	VIRUSCHECK=1
	QHPSI=/usr/bin/clamdscan %s --config=/etc/clamd.d/scan.conf --fdpass --quiet --no-summary
	USE_SYNCDIR=
	QUEUE_BASE=/var/indimail/queue
	MIN_FREE=52428800

Display environment variables set for SMTPS service

	$ sudo minisvc --print-variables=qmail-smtpd.465
	or
	$ sudo envdir -c /service/qmail-smtpd.465/variables env
	LOGFILTER=/run/indimail/logfifo
	DISABLE_PLUGIN=
	CONTROLDIR=/etc/indimail/control
	INFIFO=infifo
	USE_FSYNC=
	PLUGIN1=/usr/lib/indimail/plugins/rblsmtpd.so
	CHECKRECIPIENT=1
	QUEUE_START=1
	CHECKRELAY=
	USE_DLMOPEN=1
	SMTPS=
	DEFAULT_DOMAIN=argos.indimail.org
	QMAILQUEUE=/usr/sbin/qmail-dkim
	FIFODIR=/var/indimail/inquery
	SPAMEXITCODE=0
	PLUGIN0_dir=/var/indimail
	AUTHMODULES=/usr/sbin/sys-checkpwd /usr/sbin/vchkpass
	QUEUE_COUNT=5
	PORT=465
	SPAMFILTER=/usr/bin/bogofilter -p -d /etc/indimail
	RBLEHLO=1
	PASSWD_CACHE=
	MAXPERIP=25
	SOFT_MEM=536870912
	QUERY_CACHE=
	PLUGIN0=/usr/lib/indimail/plugins/qmail_smtpd.so
	CONFSPLIT=151
	PLUGIN0_init=smtp_init
	BODYCHECK=
	DKIMVERIFY=
	VIRUSCHECK=1
	VIRTUAL_PKG_LIB=/usr/lib64/libindimail.so.3.0.0
	QHPSI=/usr/bin/clamdscan %s --config=/etc/clamd.d/scan.conf --fdpass --quiet --no-summary
	USE_SYNCDIR=
	RBLCOMMAND=/usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org
	UNSIGNED_SUBJECT=
	REJECTSPAM=0
	MASQUERADE=
	MAXDAEMONS=75
	QUEUE_BASE=/var/indimail/queue
	MAKE_SEEKABLE=1
	MIN_FREE=52428800
	PATH=/bin:/usr/bin:/usr/sbin:/sbin
	LOCALIP=0
	CERTDIR=/etc/indimail/certs

Display environment variables set for the user <u>localuser</u> by using qmail-showctl

	$ qmail-showctl -E
	------------------ begin show env ----------------------------
	HOME=/home/localuser
	QMAILDEFAULTHOST=argos.indimail.org
	TLS_CIPHER_LIST=ALL:!aNULL:!ADH:!eNULL:!LOW:!EXP:RC4+RSA:+HIGH:+MEDIUM:-EDH-RSA-DES-CBC3-SHA:-EDH-DSS-DES-CBC3-SHA:-DES-CBC3-SHA
	:-DES-CBC3-MD5:+SSLv3:+TLSv1:-SSLv2:!DES:!MD5:!PSK:!RC4:!3DES:!SHA1:!SHA256:!SHA384
	MAKE_SEEKABLE=1
	QUEUE_BASE=/var/indimail/queue
	QUEUE_START=1
	defaultdomain=indimail.org
	LOGFILTER=/run/indimail/logfifo
	BIGTODO=0
	VIRUSCHECK=1
	DKIMSIGNOPTIONS=-z 2
	SMTP_RATE=400
	SPAMFILTER=/usr/bin/bogofilter -p -d /etc/indimail
	CONFSPLIT=23
	REJECTSPAM=0
	QMAILQUEUE=/usr/sbin/qmail-dkim /usr/sbin/qmail-spamfilter
	USER=localuser
	DYNAMIC_QUEUE=1
	SPAMEXITCODE=0
	QMAILINJECT_FORCE_SRS=1
	DKIMSIGNOPTIONSEXTRA=-z 4
	QUEUE_COUNT=1
	DKIMSIGN=/home/localuser/domainkeys/private

NOTE: The program <b>envdir</b> that indimail-mta uses, is powerful because of it's recursive feature. It has the ability to hyperlink additional directories/files having environment variables using .<u>envdir</u> and .<u>envfile</u>.

NOTE: The following clients use <u>defaultqueue</u> from <b>/etc/indimail/control</b> and .<u>defaultqueue</u> from $HOME - <b>condredirect</b>, <b>dot-forward</b>, <b>fastforward</b>, <b>filterto</b>, <b>forward</b>, <b>maildirserial</b>, <b>new-inject</b>, <b>qmail-inject</b>, <b>qmail-qread</b>, <b>qmail-showctl</b>, <b>qmta-send</b>, <b>qnotify</b>, <b>qreceipt</b>, <b>queue-fix</b>, <b>replier</b>, <b>rrforward</b>, <b>rrt</b>, <b>qmail-tcpto</b>, <b>qmail-tcpok</b>.

## Taking Backups

Once you have setup your indimail-mta system, you need to take regular backups. There are three types of backup.

1. Backup of the mails. If you are using indimail-mta alone, you will need to backup the home directory of users for which indimail-mta does local deliveries. If you are using IndiMail Virtual Domains, the program <b>vadduser</b> will be creating home directories for users in the filesystem that <u>/home/mail</u> directory belongs to. You can run `df -k /home/mail` to know the filesystem. You can use a different parent directory for the users by setting the BASE_PATH environment variable in $HOME/.defaultqueue or /etc/indimail/control/defaultqueue. You need to backup these directories using your backup tool, script or commands like tar, rsync, etc. indimail/indimail-mta doesn't provide you a tool to backup the user mails. Consult your system administrator if you have any doubts regarding backup up the base directory.

2. Backup of IndiMail, indimail-mta configuration, services configuration. If you are using the IndiMail Virtual Domains, then your data in the MySQL indimail database needs to be backed up too. This can be done by running the command

   ```
   $ sudo /usr/sbin/svctool --backup=/backup_path_to_dir --mysqlPrefix=/usr --servicedir=/service
   ```
3. You also need to take a snapshot of your current configuration and keep it safe somewhere. This can be useful if you change something and things do not work. The entire configuration for indimail, indimail-mta and all services can be obtained by running the command

   ```
   Use The below command for an exhaustive configuration dump of
   indimail/indimail-mta. You can redirect the output to a file.
   This option requires root privileges

   $ sudo /usr/sbin/svctool --dumpconfig

   Use the below command for displaying the most routine
   configuration. This option doesn't require root privileges
   unless you use the -s option.

   $ qmail-showctl -a
   ```

## Notes

Currently queueX/info/split/<u>inode</u> serves two purposes: first, it records the envelope sender; second, its modification time is used to decide when a message has been in the queue too long. In the future queueX/info/split/<u>inode</u> may store more information. Any non-backwards-compatible changes will be identified by version numbers.

When <b>qmail-queue</b> has successfully placed a message into the queue, it pulls a trigger offered by <b>qmail-send</b>. Here is the current triggering mechanism: lock/trigger is a named pipe. Before scanning todo/, <b>qmail-send</b> opens lock/trigger O\_NDELAY for reading. It then selects for readability on lock/trigger. <b>qmail-queue</b> pulls the trigger by writing a byte O\_NDELAY to lock/trigger. This makes lock/trigger readable and wakes up <b>qmail-send</b>. Before scanning todo/ again, <b>qmail-send</b> closes and reopens lock/trigger. When you use dynamic queues using <b>qscheduler</b>, instead of the trigger mechanism, <b>qmail-queue</b> uses posix message queues to communicate with <b>todo-proc</b>.

# IndiMail Queue Mechanism

IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration. To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance. Multiple queues eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). When you have massive injecting rates, your software may place multiple files in a single directory. This drastically reduces file system performance for few of the older filesystems. IndiMail avoids this by injecting your email in a queue consisting of multiple directories and mails distributed as evenly as possible across these directories.

Balancing of emails across multiple queues is achieved by the programs <b>qmail-queue</b> or <b>qmail-multi</b>. <b>qmail-multi</b> allows the message to be passed through any filter defined by the <b>FILTERARGS</b> environment variable. Any <b>qmail-queue</b> frontend can use <b>qmail-multi</b>. The list of <b>qmail-queue</b> frontends in IndiMail are

1. sendmail
2. qmail-inject
3. qmail-smtpd
4. qmail-qmqpd
5. qmail-qmtpd
6. qreceipt
7. condredirect
8. dotforward
9. fastforward
10. forward
11. maildirserial
12. new-inject
13. ofmipd
14. replier
15. rrforward

You just need to configure the following environment variables to have the <b>qmail-queue</b>(8) frontends use multiple queues.

1. QUEUE\_BASE – Base directory where all queues will be placed
2. QUEUE\_COUNT – number of queues
3. QUEUE\_START – numeric prefix of the first queue
and optionally
4. QMAILQUEUE - /usr/sbin/qmail-multi if you want to run messages through a filter (defaults to <b>qmail-queue</b> if not set)

When using dynamic queues, the following additional variables are needed

5. QUEUE\_MAX - The maximum number of queues that <b>qscheduler</b> will use.
6. QUEUE\_LOAD - The ratio of concurrency to the total concurrency percentage is the queue load. When the average QUEUE_LOAD for all queues goes beyond <b>QUEUE\_LOAD</b>, qscheduler increases the queue count.

All the above variables are bypassed if the <b>QUEUEDIR</b> is set.

7. QUEUEDIR - Not required if you set QUEUE\_BASE, QUEUE\_COUNT, QUEUE\_START. Else it should be full path to a queue (e.g. /var/indimail/queue/queue3). If QUEUEDIR is set, then QUEUE\_BASE, QUEUE\_COUNT and QUEUE\_START are not used.

<b>qmail-queue</b>'s job is to queue a message after selecting a queue. indimail-mta uses multiple queue. If QUEUEDIR is set, <b>qmail-queue</b> queues the message in the queue defined by QUEUEDIR. If QUEUEDIR is not set, it uses three environment variables QUEUE\_START, QUEUE\_COUNT and QUEUE\_BASE to select a queue. QUEUE\_BASE is the common basename for all the queues. e.g. QUEUE\_BASE=/var/indimail/queue. Now, if QUEUE\_START is 1, QUEUE\_COUNT is 5, then <b>qmail-multi</b> will generate a random number and use the modulus operator to get a number ranging from 1 to 5. Older versions of indimail-mta, <b>qmail-queue</b> did not have the ability to select a queue. It depended on <b>qmail-multi</b> to do that. <b>qmail-multi</b> works exactly like <b>qmail-queue</b> for selecting a queue. If QUEUEDIR is set, it simply execs qmail-queue after passing the message through a filter (if defined by setting FILTERARGS environment variable). If QUEUEDIR is not set, it selects a queue and then sets the QUEUEDIR environment variable to set any of the 5 queues in /var/indimail/queue. e.g. QUEUEDIR=/var/indimail/queue/queueX, where X is the number selected between 1 to 5. It then does a exec of <b>qmail-queue</b>. Throughout this document we will abbreviate /var/indimail/queue/queueX as queueX.

e.g. If you want IndiMail to use 10 queues, this is what you will do

```
$ sudo /bin/bash
# for i in qmail-smtpd.25 qmail-smtpd.465 qmail-smtpd.587 qmail-send.25 \
  qmail-qmqpd.628 qmail-qmtpd.209
do
  echo 10 > /service/$i/variables/QUEUE_COUNT
  echo “/var/indimail/queue” > /service/$i/variables/QUEUE_BASE
  echo “1” > /service/$i/variables/QUEUE_START
done
```

You also need to make sure that you have ten queues in /var/indimail/queue.

```
$ sudo /bin/bash
# for i 1 2 3 4 5 6 7 8 9 10
do
  /usr/bin/queue-fix /var/indimail/queue/queue”$i” > /dev/null
done
# exit
$ ls -ld var/indimail/queue/queue*
drwxr-x---. 12 qmailq qmail 4096 Mar 30  2017 /var/indimail/queue/queue1
drwxr-x---. 12 qmailq qmail 4096 Dec  7 10:45 /var/indimail/queue/queue10
drwxr-x---. 12 qmailq qmail 4096 Mar 30  2017 /var/indimail/queue/queue2
drwxr-x---. 12 qmailq qmail 4096 Mar 30  2017 /var/indimail/queue/queue3
drwxr-x---. 12 qmailq qmail 4096 Mar 30  2017 /var/indimail/queue/queue4
drwxr-x---. 12 qmailq qmail 4096 Mar 30  2017 /var/indimail/queue/queue5
drwxr-x---. 12 qmailq qmail 4096 Dec  7 10:45 /var/indimail/queue/queue6
drwxr-x---. 12 qmailq qmail 4096 Dec  7 10:45 /var/indimail/queue/queue7
drwxr-x---. 12 qmailq qmail 4096 Dec  7 10:45 /var/indimail/queue/queue8
drwxr-x---. 12 qmailq qmail 4096 Dec  7 10:45 /var/indimail/queue/queue9
```

Now all you need is a restart of all services (that depend on the queue configuration) to use the new QUEUE\_BASE, QUEUE\_COUNT, QUEUE\_START environment variables

```
$ sudo svc -r /service/qmail-smtpd* /service/qmail-send.25 /service/qmail-qm?pd.*
```

indimail-mta also has a special queue <b>slowq</b> where the deliveries can be rate controlled for messages injected into this queue. This is achived by setting the environment variable <b>QUEUEDIR</b>=<u>/var/indimail/queue/slowq</u>. indimail-mta provies you various [methods](#setting-environment-variables) to set environment variables. One of the method is using <b>domainqueue</b> control file discussed in [Controlling Delivery Rates](#controlling-delivery-rates).

# Using systemd to start IndiMail

IndiMail compoments get started by <b>svscan</b>. IndiMail does not have it's own startup service, but rather places all its services under indimail-mta's svscan directory <u>/service</u>. To start indimail services all that is required is to run <b>svscan</b>. Depending on your OS you can use various methods to start <b>svscan</b> automatically. <b>systemd</b> has become the defacto init replacement and can be configured to start <b>svscan</b> by using a <b>systemd unit</b> file. [systemd](http://en.wikipedia.org/wiki/Systemd "systemd") is a system and service manager for Linux, compatible with SysV and LSB init scripts. systemd provides aggressive parallelization capabilities, uses socket and D-Bus activation for starting services, offers on-demand starting of daemons, keeps track of processes using Linux cgroups, supports snapshots and restoring of the system state, maintains mount and automount points and implements an elaborate transactional dependency-based service control logic. It can work as a drop-in replacement for sysvinit. Personally I don't like systemd. It is horribly complicated and too vast to be understood easily. The <b>[RC](https://www.freebsd.org/cgi/man.cgi?rc(8))</b> used by distributions like FreeBSD, [openrc](https://wiki.gentoo.org/wiki/OpenRC) of gentoo are much simpler. But let us without further ado look at how to use <b>systemd</b>.

The first step is to write the service configuration file for IndiMail in <u>/lib/systemd/system/svscan.service</u>. You can have PrivateTmp=yes if you desire indimail to use private /tmp which is inaccessible to outside. This will give you better security when you write scripts that use <u>/var/tmp</u> or <u>/tmp</u>.

```
[Unit]
Description=IndiMail Messaging Platform
After=local-fs.target network.target

[Service]
ExecStart=/usr/libexec/indimail/svscanboot /service
ExecStop=/etc/init.d/indimail stop
Restart=on-failure
Type=simple
PrivateTmp=no

[Install]
Alias=indimail.service
Alias=indimail-mta.service
WantedBy=multi-user.target
```

You can override values in the above file by creating a file override.conf in <u>/etc/systemd/system/svscan.service.d</u>. As an example, on a raspberry pi system, you should have svscan started only after the system clock is synchronized with a NTP source (many SBC don't have battery backed Real Time Clock - RTC). This ensures that svscan gets started when the system has a correct date, time so that logs created will not have absurd timestamps.

```
[Unit]
Wants=time-sync.target
After=local-fs.target remote-fs.target time-sync.target network.target network-online.target systemd-networkd-wait-online.service
```

So if you have a system without a battery backed RTC, you should do this (even when you do a binary installation)

```
$ sudo mkdir /etc/systemd/system/svscan.service.d
$ sudo cp /usr/share/indimail/boot/systemd.override.conf /etc/systemd/system/svscan.service.d/override.conf
$ sudo systemctl daemon-reload
```

NOTE: From Fedora 15 onwards, upstart has been replaced by a service called systemd. Due to improper rpm package upgrade scripts, some system services previously enabled in Fedora 14, may not be enabled after upgrading to Fedora 15. To determine if a service is impacted, run the systemctl status command as shown below.

```
# systemctl is-enabled svscan.service && echo "Enabled on boot" || echo "Disabled on boot"
```

To enable indimail service on boot, run the following systemctl command

`# systemctl enable svscan.service`

Now to start IndiMail you can use any of the below commands (commands for starting indimail on distributions other than Linux are also mentioned).

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

NOTE: FreeBSD uses /usr/local/etc/rc.d/svscan. OSX uses LaunchDaemon with the configuration in /Library/LaunchDaemons/org.indimail.svscan.plist

You can automate the above service creation for systemd by running the initsvc(1) command. In fact the command works for Linux, FreeBSD and OSX to enable IndiMail to get started at boot (even though FreeBSD, OSX do not use systemd).

```
# /usr/sbin/initsvc -on  (to enable indimail service)
# /usr/sbin/initsvc -off (to disable indimail service)
```

You can now also query the status of the running IndiMail service by using the systemctl command

```
# systemctl status svscan.service
● svscan.service - SVscan Service
     Loaded: loaded (/usr/lib/systemd/system/svscan.service; enabled; vendor preset: disabled)
     Active: active (running) since Sun 2021-12-26 07:44:17 IST; 4h 11min ago
   Main PID: 1041 (svscan)
      Tasks: 229 (limit: 9405)
     Memory: 2.3G
     CGroup: /system.slice/svscan.service
             ├─ 1041 /usr/sbin/svscan /service
             ├─ 1066 supervise log .svscan
             ├─ 1068 /usr/sbin/multilog t /var/log/svc/svscan
             ├─ 1080 supervise qmail-qmqpd.628
             ├─ 1081 supervise log qmail-qmqpd.628
             ├─ 1082 supervise resolvconf
             ├─ 1083 supervise log resolvconf
             ├─ 1084 supervise mpdev
             ├─ 1085 supervise log mpdev
             ├─ 1086 supervise qmail-imapd.143
             ├─ 1087 supervise log qmail-imapd.143
             ├─ 1088 supervise qmail-poppass.106
             ├─ 1089 supervise log qmail-poppass.106
             ├─ 1090 supervise qmail-daned.1998
             ├─ 1091 supervise log qmail-daned.1998
             ├─ 1092 supervise qmail-imapd-ssl.993
             ├─ 1093 supervise log qmail-imapd-ssl.993
             ├─ 1094 supervise qmail-smtpd.25
             ├─ 1095 supervise log qmail-smtpd.25
             ├─ 1096 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.poppass.cdb -X -c variables/MAXDAEMONS -C 25 -o -b 40 -n /etc/indimail/certs/servercert.pem -u 1024 -g 1014 0 106 /usr/sbin/qmail-poppass argos.indimail.org /usr/sbin/vchkpass /bin/false
             ├─ 1097 /usr/sbin/multilog t /var/log/svc/qmqpd.628
             ├─ 1098 supervise indisrvr.4000
             ├─ 1099 supervise log indisrvr.4000
             ├─ 1100 supervise qmail-pop3d.110
             ├─ 1101 supervise log qmail-pop3d.110
             ├─ 1102 supervise slowq-send
             ├─ 1103 supervise log slowq-send
             ├─ 1104 supervise update
             ├─ 1105 /usr/sbin/qmail-daned -w /etc/indimail/control/tlsa.white -t 30 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/tlsa.context
             ├─ 1106 supervise log update
             ├─ 1107 supervise qmail-smtpd.366
             ├─ 1108 supervise log qmail-smtpd.366
             ├─ 1109 supervise freshclam
             ├─ 1110 supervise log freshclam
             ├─ 1111 supervise fclient
             ├─ 1112 supervise log fclient
             ├─ 1113 supervise qmail-pop3d-ssl.995
             ├─ 1114 supervise log qmail-pop3d-ssl.995
             ├─ 1115 supervise qmail-qmtpd.209
             ├─ 1116 supervise log qmail-qmtpd.209
             ├─ 1117 supervise mrtg
             ├─ 1118 supervise log mrtg
             ├─ 1119 supervise rsync.873
             ├─ 1120 supervise log rsync.873
             ├─ 1121 supervise qmail-logfifo
             ├─ 1122 supervise log qmail-logfifo
             ├─ 1123 supervise qscanq
             ├─ 1124 supervise log qscanq
             ├─ 1125 supervise qmail-send.25
             ├─ 1126 supervise log qmail-send.25
             ├─ 1127 supervise greylist.1999
             ├─ 1128 supervise log greylist.1999
             ├─ 1129 supervise qmail-smtpd.465
             ├─ 1130 supervise log qmail-smtpd.465
             ├─ 1131 supervise proxy-imapd.4143
             ├─ 1132 supervise log proxy-imapd.4143
             ├─ 1133 supervise fetchmail
             ├─ 1134 supervise log fetchmail
             ├─ 1135 supervise qmail-smtpd.587
             ├─ 1136 supervise log qmail-smtpd.587
             ├─ 1137 supervise pwdlookup
             ├─ 1138 supervise log pwdlookup
             ├─ 1140 /usr/bin/freshclam -v --stdout --datadir=/var/indimail/clamd -d -c 2 --config-file=/etc/freshclam.conf
             ├─ 1141 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 993 /usr/bin/couriertls -server -tcpd /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/libexec/indimail/imapmodules/authindi /usr/bin/imapd Maildir
             ├─ 1143 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 150 -u 1024 -g 1015 0 366 /usr/sbin/qmail-smtpd
             ├─ 1144 /usr/sbin/indisrvr -i 0 -p 4000 -b 40 -t 300 -n /etc/indimail/certs/servercert.pem
             ├─ 1145 supervise proxy-imapd-ssl.9143
             ├─ 1146 supervise log proxy-imapd-ssl.9143
             ├─ 1147 supervise dnscache
             ├─ 1148 supervise log dnscache
             ├─ 1149 supervise proxy-pop3d-ssl.9110
             ├─ 1150 supervise log proxy-pop3d-ssl.9110
             ├─ 1151 supervise inlookup.infifo
             ├─ 1152 supervise log inlookup.infifo
             ├─ 1153 supervise clamd
             ├─ 1154 supervise log clamd
             ├─ 1155 supervise proxy-pop3d.4110
             ├─ 1156 supervise log proxy-pop3d.4110
             ├─ 1157 supervise mysql.3306
             ├─ 1158 supervise log mysql.3306
             ├─ 1159 supervise udplogger.3000
             ├─ 1160 supervise log udplogger.3000
             ├─ 1161 /bin/sh ./run
             ├─ 1162 /usr/sbin/multilog t /var/log/svc/poppass.106
             ├─ 1163 /bin/sh ./run
             ├─ 1164 /usr/bin/tcpserver -v -h -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 1024 -g 1015 0 25 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so
             ├─ 1166 /usr/sbin/cleanq -l -s 200 /var/indimail/qscanq/root/scanq
             ├─ 1167 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 995 /usr/bin/couriertls -server -tcpd /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/libexec/indimail/imapmodules/authindi /usr/bin/pop3d Maildir
             ├─ 1168 /usr/sbin/multilog t /var/log/svc/indisrvr.4000
             ├─ 1169 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 143 /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/libexec/indimail/imapmodules/authindi /usr/bin/imapd Maildir
             ├─ 1171 /usr/sbin/qmail-greyd -w /etc/indimail/control/greylist.white -t 30 -g 24 -m 2 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/greylist.context
             ├─ 1172 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 110 /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/libexec/indimail/imapmodules/authindi /usr/bin/pop3d Maildir
             ├─ 1173 slowq-send /var/indimail/queue/slowq
             ├─ 1174 /usr/bin/qmail-cat /run/indimail/logfifo
             ├─ 1175 /usr/sbin/multilog t /var/log/svc/smtpd.366
             ├─ 1176 /usr/sbin/mysqld --defaults-file=/etc/indimail/indimail.cnf --port=3306 --basedir=/usr --datadir=/var/indimail/mysqldb/data --memlock --ssl --require-secure-transport --skip-external-locking --delay-key-write=all --skip-name-resolve --sql-mode=NO_ENGINE_SUBSTITUTION,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,STRICT_TRANS_TABLES --explicit-defaults-for-timestamp=TRUE --general-log=1 --general-log-file=/var/indimail/mysqldb/logs/general-log --slow-query-log=1 --slow-query-log-file=/var/indimail/mysqldb/logs/slowquery-log --log-queries-not-using-indexes --log-error-verbosity=3 --pid-file=/run/mysqld/mysqld.3306.pid
             ├─ 1177 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 9143 /usr/bin/couriertls -server -tcpd /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 1178 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 4143 /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 1179 /usr/sbin/udplogger -p 3000 -t 10 0
             ├─ 1180 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 4110 /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 1181 /usr/sbin/clamd --config-file=/etc/clamd.d/scan.conf
             ├─ 1182 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 1024 -g 1015 0 587 /usr/lib/indimail/plugins/qmail_smtpd.so argos.indimail.org /usr/sbin/sys-checkpwd /usr/sbin/vchkpass /bin/false
             ├─ 1184 /usr/bin/tcpserver -v -h -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 1024 -g 1015 0 465 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so argos.indimail.org /usr/sbin/sys-checkpwd /usr/sbin/vchkpass /bin/false
             ├─ 1185 /usr/bin/dnscache
             ├─ 1187 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 9110 /usr/bin/couriertls -server -tcpd /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 1188 /usr/sbin/multilog t /var/log/svc/update
             ├─ 1190 multilog t ./main
             ├─ 1191 /usr/sbin/multilog t /var/log/svc/inlookup.infifo
             ├─ 1194 /usr/sbin/multilog t /var/log/svc/slowq
             ├─ 1195 /usr/sbin/multilog t /var/log/svc/imapd.143
             ├─ 1196 /usr/sbin/multilog t /var/log/svc/logfifo
             ├─ 1197 /usr/sbin/multilog t /var/log/svc/pop3d.110
             ├─ 1200 /usr/sbin/multilog t /var/log/svc/pop3d-ssl.995
             ├─ 1201 /usr/sbin/multilog t /var/log/svc/mrtg
             ├─ 1203 /usr/sbin/multilog t /var/log/svc/mpdev
             ├─ 1204 /usr/sbin/multilog t /var/log/svc/smtpd.25
             ├─ 1205 /usr/sbin/multilog t /var/log/svc/resolvconf
             ├─ 1206 /usr/sbin/multilog t /var/log/svc/qmtpd.209
             ├─ 1208 /usr/sbin/multilog t /var/log/svc/freshclam
             ├─ 1209 /usr/sbin/multilog t /var/log/svc/fclient
             ├─ 1211 /usr/sbin/multilog t /var/log/svc/daned.1998
             ├─ 1213 /usr/sbin/multilog t /var/log/svc/imapd-ssl.993
             ├─ 1214 /usr/sbin/multilog t /var/log/svc/pwdlookup
             ├─ 1215 /usr/sbin/multilog t /var/log/svc/greylist.1999
             ├─ 1217 /usr/sbin/multilog t /var/log/svc/rsyncd.873
             ├─ 1218 /usr/sbin/multilog t /var/log/svc/proxyIMAP.4143
             ├─ 1219 /usr/sbin/multilog t /var/log/svc/smtpd.465
             ├─ 1220 /usr/sbin/multilog t /var/log/svc/deliver.25
             ├─ 1221 /usr/sbin/multilog t /var/log/svc/smtpd.587
             ├─ 1224 /usr/sbin/multilog t /var/log/svc/proxyIMAP.9143
             ├─ 1225 /usr/sbin/multilog t /var/log/svc/proxyPOP3.9110
             ├─ 1226 /usr/sbin/multilog t /var/log/svc/proxyPOP3.4110
             ├─ 1227 /usr/sbin/multilog t /var/log/svc/clamd
             ├─ 1229 /usr/sbin/multilog t /var/log/svc/mysql.3306
             ├─ 1230 /usr/sbin/multilog t /var/log/svc/udplogger.3000
             ├─ 1231 /usr/sbin/multilog t /var/log/svc/fetchmail
             ├─ 1232 /usr/sbin/multilog t /var/log/svc/qscanq
             ├─ 1290 /bin/sh ./run
             ├─ 1291 /usr/bin/inotify -n /etc
             ├─ 1292 /bin/sh ./run
             ├─ 1380 qmail-lspawn ./Maildir/ /var/indimail/queue/slowq
             ├─ 1381 qmail-rspawn /var/indimail/queue/slowq
             ├─ 1382 qmail-clean /var/indimail/queue/slowq slowq-send
             ├─ 1713 /usr/sbin/qscheduler -d ./Maildir/
             ├─ 1716 qmail-send -d /var/indimail/queue/queue1
             ├─ 1717 qmail-send -d /var/indimail/queue/queue2
             ├─ 1718 qmail-send -d /var/indimail/queue/queue3
             ├─ 1719 qmail-send -d /var/indimail/queue/queue4
             ├─ 1720 qmail-send -d /var/indimail/queue/queue5
             ├─ 1721 qmail-lspawn ./Maildir/ /var/indimail/queue/queue4
             ├─ 1722 qmail-rspawn /var/indimail/queue/queue4
             ├─ 1723 qmail-clean /var/indimail/queue/queue4 todo-proc
             ├─ 1724 todo-proc -d /var/indimail/queue/queue4
             ├─ 1725 qmail-clean /var/indimail/queue/queue4 qmail-send
             ├─ 1726 qmail-lspawn ./Maildir/ /var/indimail/queue/queue3
             ├─ 1727 qmail-rspawn /var/indimail/queue/queue3
             ├─ 1728 qmail-clean /var/indimail/queue/queue3 todo-proc
             ├─ 1729 todo-proc -d /var/indimail/queue/queue3
             ├─ 1730 qmail-clean /var/indimail/queue/queue3 qmail-send
             ├─ 1731 qmail-lspawn ./Maildir/ /var/indimail/queue/queue1
             ├─ 1732 qmail-rspawn /var/indimail/queue/queue1
             ├─ 1733 qmail-clean /var/indimail/queue/queue1 todo-proc
             ├─ 1734 todo-proc -d /var/indimail/queue/queue1
             ├─ 1735 qmail-clean /var/indimail/queue/queue1 qmail-send
             ├─ 1736 qmail-lspawn ./Maildir/ /var/indimail/queue/queue5
             ├─ 1737 qmail-rspawn /var/indimail/queue/queue5
             ├─ 1738 qmail-clean /var/indimail/queue/queue5 todo-proc
             ├─ 1739 todo-proc -d /var/indimail/queue/queue5
             ├─ 1740 qmail-clean /var/indimail/queue/queue5 qmail-send
             ├─ 1744 qmail-lspawn ./Maildir/ /var/indimail/queue/queue2
             ├─ 1746 qmail-rspawn /var/indimail/queue/queue2
             ├─ 1747 qmail-clean /var/indimail/queue/queue2 todo-proc
             ├─ 1748 todo-proc -d /var/indimail/queue/queue2
             ├─ 1749 qmail-clean /var/indimail/queue/queue2 qmail-send
             ├─ 3247 /usr/sbin/nssd -d notice
             ├─ 3255 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 3278 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 3279 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 3280 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 3281 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 3282 /usr/sbin/inlookup -i 5 -c 5184000
             ├─12992 sleep 300
             ├─13031 sh -c "\n    sleep 10; /usr/libexec/pistop/client sleep 2>&1"
             ├─13038 /usr/bin/sh /usr/libexec/pistop/client sleep
             └─13039 sleep 20

Dec 26 07:44:27 argos.indimail.org qmail-lspawn[1721]: (nssd) 1721: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-lspawn[1726]: (nssd) 1726: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-lspawn[1731]: (nssd) 1731: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-rspawn[1722]: (nssd) 1722: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-rspawn[1727]: (nssd) 1727: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-rspawn[1732]: (nssd) 1732: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-lspawn[1736]: (nssd) 1736: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-rspawn[1737]: (nssd) 1737: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-rspawn[1746]: (nssd) 1746: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
Dec 26 07:44:27 argos.indimail.org qmail-lspawn[1744]: (nssd) 1744: _open_socket: connect: /run/indimail/nssd.sock: (errno 2) No such file or directory
```

IndiMail also provides the svps command which gives a neat display on the status of all services configured for IndiMail. You can omit the -a flag to omit the logging processes.

```
$ sudo svps -a
------------ svscan ---------------
/usr/sbin/svscan /service          up      9227 secs  pid   44997

------------ main -----------------
/service/fetchmail                 down    9226 secs spid   45058
/service/qmail-qmqpd.628           down    9226 secs spid   45007
/service/inlookup.infifo           up      9196 secs  pid   46146
/service/pwdlookup                 up      9196 secs  pid   46148
/service/dnscache                  up      9226 secs  pid   45119
/service/freshclam                 up      9226 secs  pid   45069
/service/greylist.1999             up      9226 secs  pid   45102
/service/indisrvr.4000             up      9226 secs  pid   45090
/service/mrtg                      up      9226 secs  pid   45125
/service/mysql.3306                up      9226 secs  pid   45148
/service/proxy-imapd.4143          up      9226 secs  pid   45128
/service/proxy-imapd-ssl.9143      up      9226 secs  pid   45141
/service/proxy-pop3d.4110          up      9226 secs  pid   45147
/service/proxy-pop3d-ssl.9110      up      9226 secs  pid   45120
/service/qmail-daned.1998          up      9226 secs  pid   45067
/service/qmail-imapd.143           up      9226 secs  pid   45064
/service/qmail-imapd-ssl.993       up      9226 secs  pid   45072
/service/qmail-logfifo             up      9226 secs  pid   45091
/service/qmail-pop3d.110           up      9226 secs  pid   45104
/service/qmail-pop3d-ssl.995       up      9226 secs  pid   45114
/service/qmail-poppass.106         up      9226 secs  pid   45081
/service/qmail-qmtpd.209           up      9226 secs  pid   45107
/service/qmail-send.25             up      9226 secs  pid   45131
/service/qmail-smtpd.25            up      9226 secs  pid   45066
/service/qmail-smtpd.366           up      9226 secs  pid   45065
/service/qmail-smtpd.465           up      9226 secs  pid   45124
/service/qmail-smtpd.587           up      9226 secs  pid   45136
/service/qscanq                    up      9226 secs  pid   45096
/service/udplogger.3000            up      9226 secs  pid   45150

------------ logs -----------------
/service/.svscan/log               up      9226 secs  pid   45024
/service/clamd/log                 up      9226 secs  pid   45123
/service/fetchmail/log             up      9226 secs  pid   45137
/service/freshclam/log             up      9226 secs  pid   45074
/service/greylist.1999/log         up      9226 secs  pid   45097
/service/indisrvr.4000/log         up      9226 secs  pid   45063
/service/inlookup.infifo/log       up      9226 secs  pid   45118
/service/mrtg/log                  up      9226 secs  pid   45099
/service/mysql.3306/log            up      9226 secs  pid   45142
/service/proxy-imapd.4143/log      up      9226 secs  pid   45101
/service/proxy-imapd-ssl.9143/log  up      9226 secs  pid   45126
/service/proxy-pop3d.4110/log      up      9226 secs  pid   45143
/service/proxy-pop3d-ssl.9110/log  up      9226 secs  pid   45117
/service/pwdlookup/log             up      9226 secs  pid   45132
/service/qmail-daned.1998/log      up      9226 secs  pid   45044
/service/qmail-imapd.143/log       up      9226 secs  pid   45075
/service/qmail-imapd-ssl.993/log   up      9226 secs  pid   45070
/service/qmail-logfifo/log         up      9226 secs  pid   45135
/service/qmail-pop3d.110/log       up      9226 secs  pid   45103
/service/qmail-pop3d-ssl.995/log   up      9226 secs  pid   45105
/service/qmail-poppass.106/log     up      9226 secs  pid   45046
/service/qmail-qmqpd.628/log       up      9226 secs  pid   45113
/service/qmail-qmtpd.209/log       up      9226 secs  pid   45106
/service/qmail-send.25/log         up      9226 secs  pid   45129
/service/qmail-smtpd.25/log        up      9226 secs  pid   45073
/service/qmail-smtpd.366/log       up      9226 secs  pid   45068
/service/qmail-smtpd.465/log       up      9226 secs  pid   45140
/service/qmail-smtpd.587/log       up      9226 secs  pid   45121
/service/qscanq/log                up      9226 secs  pid   45139
/service/udplogger.3000/log        up      9226 secs  pid   45149
```

# Eliminating Duplicate Emails during local delivery

Often you will find program like MS outlook, notorious for sending duplicate emails, flooding your inbox. IndiMail allows you to quickly deal with this proprietary nonsense by turning on duplicate eliminator in **vdelivermail**(8) - the default MDA. To turn on the duplicate eliminator in <b>vdelivermail</b>, you need to set **ELIMINATE_DUPS** and **MAKE_SEEKABLE** environment variables.

```
$ sudo /bin/bash
# echo 1>/service/qmail-send.25/variables/ELIMINATE_DUPS
# echo 1>/service/qmail-send.25/variables/MAKE_SEEKABLE
# svc -r /service/qmail-send.25
```

If you do not use <b>vdelivermail</b> and want to use your own delivery agent? Fear not by using <b>ismaildup</b>(1). <b>ismaildup</b> expects the email on standard input and is easily scriptable like the example below in a .qmail file.

`|ismaildup /usr/bin/maildirdeliver /home/manny/Maildir/`

will deliver mails to /home/manny/Maildir while discarding duplicates.

If you are not happy with the 900 seconds (15 minutes) time interval for checking duplicates, you can change it by setting the DUPLICATE_INTERVAL environment variable. The following will not allow a single duplicate to be entertained within 24 hours

```
$ sudo /bin/bash
# echo 86400 > /service/qmail-send.25/variables/DUPLICATE_INTERVAL
# svc -r /service/qmail-send.25
# exit
```

# Using procmail with IndiMail

IndiMail follows the traditional UNIX philosophy.

`Write programs that do one thing and do it well. Write programs to work together.`
`Write programs to handle text streams, because that is a universal interface`

This allows IndiMail to interface with many programs written by others. IndiMail uses a powerful filter mechanism called vfilter(8). You may already be familiar with procmail. procmail is a mail delivery agent (MDA) capable of sorting incoming mail into various directories and filtering out messages. There are three ways in which you can use procmail with IndiMail.

1. inside .qmail

   `| preline procmail`

2. edit .qmail-default

   `| preline -f procmail -p -m /etc/indimail/procmailrc`

3. have an alias
   You can use valias(1) to create an alias to call procmail. The following alias calls procmail to deliver the mail using /etc/indimail/procmailrc as a procmail recipe

   ```
   valias -i "|/usr/bin/preline -f /usr/bin/procmail -p -m /etc/indimail/procmailrc" testuser@example.com
   ```

The following procmailrc puts virus infected mails in /tmp/Maildir and calls **maildirdeliver**(1) to deliver the mail to /home/mail/T2Zsym/example.com/testuser01/Maildir.

```
SHELL=/bin/bash
VERBOSE="no"
unset DTLINE
unset RPLINE
:0w
*^X-Virus-Status: INFECTED
/tmp/Maildir/.Virus
:0w
| /usr/bin/maildirdeliver /home/mail/T2Zsym/example.com/testuser01/Maildir
```

You can replace maildirdeliver in the last line with <b>vdelivermail</b>(8)

`| /usr/sbin/vdelivermail '' bounce-no-mailbox`

# Writing Filters for IndiMail

IndiMail provides multiple methods by which you can intercept an email in transit and modify the email headers or the email body. A filter is a simple program that expects the raw email on standard input and outputs the message text back on standard output. The program /bin/cat can be used as a filter which simply copies the standard input to standard output without modifying anything. Some methods can be used before the mail gets queued and some methods can be used before the execution of local / remote delivery.

It is not necessary for a filter to modify the email. You can have a filter just to extract the headers or body and use that information for some purpose. IndiMail also provides the following programs - 822addr(1), 822headerfilter(1), 822bodyfilter(1), 822field(1), 822fields(1), 822header(1), 822body(1), 822headerok(1), 822received(1), 822date(1), 822fields(1) to help in processing emails.

Let us say that we have written a script /usr/local/bin/myfilter. The myfilter program expects the raw email on stdin and outputs the email back (maybe modiying it) on stdout.

## Filtering during SMTP (before mail gets queued)

### Using FILTERARGS environment variable

The below configuration causes all inbound SMTP email to be fed through the filter /usr/local/bin/myfilter. You can use the programs 822header(1), 822body(1) inside myfilter to get and manipulate the headers and body (See 1.5.1).

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-smtpd.25/variables/FILTERARGS
# svc -r /service/qmail-smtpd.25

NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).
```

### Using QMAILQUEUE with qmail-qfilter

You can use <b>qmail-qfilter</b>(1). <b>qmail-qfilter</b> allows you to run multiple filters passed as command line arguments to <b>qmail-qfilter</b>. Since QMAILQUEUE doesn't allow you to specify multiple arguments, you can write a shell script which calls <b>qmail-qfilter</b> and have the shell script defined as QMAILQUEUE environment variable. The way qmail-qfilter works is simple - The original content is avaible on fd 0. If your filter writes anything on the stdout, that becomes the new mail. If doesn't output anything on the stdout, the mail is passed unchanged. The envelope is available on fd 0. If your scripts writes on fd 3, that becomes the new envelope. A filter that exits 0, passes the mail unchanged. A filter that simply executes /bin/cat also passes the mail unchanged, except that qmail-qfilter does a bit more work to make the new content as fd 0 for the next filter in the chain. You can have multiple filters passed to qmail-qfilter each separated by `--` (two dashes).

```
$ sudo /bin/bash
# echo /usr/bin/qmail-qfilter /usr/local/bin/myfilter > /usr/local/bin/qfilter
# chmod +x /usr/local/bin/qfilter
# echo /usr/local/bin/qfilter > /service/qmail-smtpd.25/variables/QMAILQUEUE
# echo /usr/bin/qmail-dk > /service/qmail-smtpd.25/variables/QQF_QMAILQUEUE
# svc -r /service/qmail-smtpd.25
```

To make it easy, indimail-mta provides a qmail-qfilter frontend, <b>qfrontend</b> that calls all your filters. All you have to do is write your script with a filename that starts with `qf-` and place them in /usr/libexec/indimail/qfilters directory. To enable the filters edit the file /etc/indimail/control/qfilters and add the filename (without the full path) in this file. Take a look at existing filters in the /usr/libexec/indimail/qfilters.

NOTE: you can define QQF\_MAILQUEUE to /usr/bin/qmail-nullqueue to discard the mail (blackhole) if you want to discard mails matching certain conditions that you want.

### Using QMAILQUEUE with your own program

When you want to use your own program as QMAILQUEUE, then your program is responsible for queuing the email. It is trivial to queue the email by calling <b>qmail-queue</b>(8). Your script can read the stdin for the raw message (headers + body) and pipe the output (maybe after modifications) to <b>qmail-queue</b>(8). If you are doing DK/DKIM signing, you can execute <b>qmail-dk</b>(8) instead of <b>qmail-queue</b>(8). You can have <b>qmail-dk</b>(8) call <b>qmail-dkim</b>(8) and <b>qmail-dkim</b>(8) calls <b>qmail-queue</b>(8). Assuming you want to do DK/DKIM signing, and myfilter calls <b>qmail-dk</b>(8), you can do the following

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-smtpd.25/variables/QMAILQUEUE
# echo /usr/bin/qmail-dkim  > /service/qmail-smtpd.25/variables/DKQUEUE
# echo /usr/bin/qmail-queue > /service/qmail-smtpd.25/variables/DKIMQUEUE
# svc -r /service/qmail-smtpd.25
```

NOTE: You can set the environment variable NULLQUEUE before calling <b>qmail-queue</b> to discard the mail completely (blackhole).

## Filtering during local / remote delivery

### Using FILTERARGS environment variable

The below configuration causes all local / remote deliveries to be fed through the filter /usr/local/bin/myfilter. You can use the programs 822header(1), 822body(1) inside myfilter to get and manipulate the headers and body.

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-send.25/variables/FILTERARGS
# svc -r /service/qmail-send.25
```

If you want to filter only for local delivery or only for remote delivery, you can use the environment variable QMAILLOCAL or QMAILREMOTE. QMAILLOCAL is defined only for local deliveries while QMAILREMOTE is defined only for remote deliveries.
NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).
e.g. the below script skips filtering for remote deliveries

```
#!/bin/sh
if [ -n “$QMAILREMOTE” ] ; then
    exec /bin/cat
fi
# rest of the script
...
...
exit 0
```

### Using control file filterargs

The control file filterargs gives you control to run filters individually for local or remote deliveries. It also allows you to run your filter for both local and remote deliveries. See <b>spawn-filter</b>(8) for full description on this control file.
e.g. The following entry in /etc/indimail/control/filterargs causes all mails to yahoo.com be fed through the filter dk-filter(8) for DK/DKIM signing.
yahoo.com:remote:/usr/bin/dk-filter
NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).

### Using QMAILLOCAL or QMAILREMOTE environment variables

If you define QMAILLOCAL, indimail will execute the program/script defined by the QMAILLOCAL variable for all local deliveries. The arguments passed to this program/script will be the same as that for <b>qmail-local</b>(8).
Similarly, if you define QMAILREMOTE, indimail will execute the program/script defined by the QMAILREMOTE variable for all remote deliveries. The argument passed to this program/script are the same as that for <b>qmail-remote</b>(8).
The raw email (header + body) is available on stdin. You can use 822header(8), 822body(8) for getting the headers and body. After your program is through with filtering, the output should be piped to <b>qmail-local</b>(8) for local deliveries and <b>qmail-remote</b>(8) for remote deliveries. You need to also call <b>qmail-local</b> / <b>qmail-remote</b> with the same arguments. i.e

```
exec qmail-local  "$@"     #(for local deliveries)
exec qmail-remote "$@"     #(for remote deliveries)
```

NOTE: You can exit with value 0 instead of calling <b>qmail-local</b> / <b>qmail-remote</b> to discard the mail completely (blackhole)

## Using dot-qmail(5) or valias(1)

Both .qmail files and valias mechanism allows you to execute your own programs for local deliveries. See the man pages for dot-qmail(5) and valias(1) for more details. After manipulating the original raw email on stdin, you can pipe the out to the program maildirdeliver(1) for the final delivery.
Assuming you write the program myscript to call maildirdeliver program, you can use the valias command to add the following alias

`$ valias -i "|/usr/local/bin/myfilter" testuser01@example.com`

Now any mail sent to testuser01@example.com will be given to the program /usr/local/bin/myfilter as standard input.
NOTE: you can exit with value 0 instead of calling the maildirdeliver program to discard the mail completely (blackhole).

## Using IndiMail rule based filter - vfilter

IndiMail's vfilter(8) mechanism allows you to create rule based filter based on any keyword in the message headers or message body. You can create a vfilter by calling the <b>vcfilter</b>(1) program.

```
$ vcfilter -i -t myfilter -h 2 -c 0 -k "failure notice" -f /NoDeliver -b "2|/usr/local/bin/myfilter" testuser01@example.com
```

NOTE: you can exit with value 0 instead of putting anything on standard output to discard the mail completely (blackhole).

## Examples Filters

e.g. the below filter looks for emails having "failure notice" in the subject line and inserts the line "sorry about that" in the first line of the message body and puts the line “sent by IndiMail Messaging platform” in the last line

### FILTERARGS script

```
#!/bin/sh
# create a temporary file
tmp_file=`mktemp -p /var/tmp -t myfilter.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
# Copy the stdin
/bin/cat > $tmp_file
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
subject=`/usr/bin/822header -I Subject < $tmp_file`
echo $subject | grep "failure notice" > /dev/null 2>&1
if [ $? -eq 0 ] ; then
    (
    /usr/bin/822header < $tmp_file
    echo
    echo "sorry about that"
    /usr/bin/822body < $tmp_file
    echo "sent by IndiMail Messaging platform"
    )
else
    /bin/cat $tmp_file
fi
/bin/rm -f $tmp_file
exit 0
```

### QMAILQUEUE script

```
#!/bin/sh
# create a temporary file
inp_file=`mktemp -p /var/tmp -t myfilteri.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
out_file=`mktemp -p /var/tmp -t myfiltero.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
/bin/cat > $inp_file
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
subject=`/usr/bin/822header -I Subject < $inp_file`
echo $subject | grep "failure notice" > /dev/null 2>&1
if [ $? -eq 0 ] ; then
    (
    /usr/bin/822header < $inp_file
    echo
    echo "sorry about that"
    /usr/bin/822body < $inp_file
    echo "sent by IndiMail Messaging platform"
    ) > $out_file
    if [ $? -ne 0 ] ; then
        /bin/rm -f $inp_file $out_file
        exit 111
    fi
    exec 0<$out_file
else
    exec 0<$inp_file
fi
/bin/rm -f $inp_file $out_file
# queue the message
exec /usr/bin/qmail-queue
exit 111
```

### QMAILREMOTE script

```
#!/bin/sh
# This scripts expects qmail-remote arguments on command line
# argv0          - qmail-remote
# argv1          - host   (host)
# argv2          - sender (sender)
# argv3          - qqeh   (qmail queue extra header)
# argv4          - size
# argv5 .. argvn - recipients
#
#
host=$1
sender=$2
qqeh=$3
size=$4
shift 4
#
# if needed you can modify host, sender, qqeh, size args above
#
if [ -z "$QMAILREMOTE" ] ; then # execute qmail-local
    # call spawn-filter so that features like
    # FILTERARGS, SPAMFILTER are not lost
    exec -a qmail-local /usr/bin/spawn-filter "$@"
fi
if [ " $CONTROLDIR" = " " ] ; then
   FN=/etc/indimail/control/filterargs
else
   FN=$CONTROLDIR/filterargs
fi
if [ -n "$SPAMFILTER" -o -n "$FILTERARGS" -o -f $FN ] ; then
   # execute spawn-filter if you have filters defined for remote/local deliveries
   PROG="bin/spawn-filter"
else
   PROG="bin/qmail-remote"
fi
# create a temporary file
inp_file=`mktemp -p /var/tmp -t myfilteri.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
out_file=`mktemp -p /var/tmp -t myfiltero.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
/bin/cat > $inp_file
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
subject=`/usr/bin/822header -I Subject < $inp_file`
echo $subject | grep "failure notice" > /dev/null 2>&1
if [ $? -eq 0 ] ; then
    (
    /usr/bin/822header < $inp_file
    echo
    echo "sorry about that"
    /usr/bin/822body < $inp_file
    echo "sent by IndiMail Messaging platform"
    ) > $out_file
    if [ $? -ne 0 ] ; then
        /bin/rm -f $inp_file $out_file
        exit 111
    fi
    exec 0<$out_file
else
    exec 0<$inp_file
fi
/bin/rm -f $inp_file $out_file
# $PROG points to spawn-filter if FILTERARGS or SPAMFILTER is set
# use $PROG so that features like FILTERARGS, SPAMFILTER are not lost
exec -a qmail-remote $PROG "$host" "$sender" "$qqeh" $size $*
exit 111
```

### QMAILLOCAL script

```
#!/bin/sh
# This scripts expects qmail-local arguments on command line
# argv0          - qmail-local
# argv1          - user
# argv2          - homedir
# argv3          - local
# argv4          - dash
# argv5          - ext
# argv6          - domain
# argv7          - sender
# argv8          - defaultdelivery (mbox, Maildir)
# argv9          - qqeh
#
user=$1
homedir=$2
local=$3
dash=$4
ext=$5
domain=$6
sender=$7
defaultdel=$8
qqeh=$9

if [ -z "$QMAILLOCAL" ] ; then # execute qmail-remote
    # call spawn-filter so that features like
    # FILTERARGS, SPAMFILTER are not lost
    exec -a qmail-remote /usr/bin/spawn-filter "$@"
fi
# create a temporary file
inp_file=`mktemp -p /var/tmp -t myfilteri.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
out_file=`mktemp -p /var/tmp -t myfiltero.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
/bin/cat > $inp_file
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
subject=`/usr/bin/822header -I Subject < $inp_file`
echo $subject | grep "failure notice" > /dev/null 2>&1
if [ $? -eq 0 ] ; then
    (
    /usr/bin/822header < $inp_file
    echo
    echo "sorry about that"
    /usr/bin/822body < $inp_file
    echo "sent by IndiMail Messaging platform"
    ) > $out_file
    if [ $? -ne 0 ] ; then
        /bin/rm -f $inp_file $out_file
        exit 111
    fi
    exec 0<$out_file
else
    exec 0<$inp_file
fi
/bin/rm -f $inp_file $out_file

# call spawn-filter so that features like
# FILTERARGS, SPAMFILTER are not lost
exec -a qmail-local /usr/bin/spawn-filter "$@"
exit 111

1.5.5 valias / vfilter script
#!/bin/sh
# create a temporary file
inp_file=`mktemp -p /var/tmp -t myfilteri.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file
    exit 111
fi
out_file=`mktemp -p /var/tmp -t myfiltero.XXXXXXXXXXXXXXX`
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
/bin/cat > $inp_file
if [ $? -ne 0 ] ; then
    /bin/rm -f $inp_file $out_file
    exit 111
fi
subject=`/usr/bin/822header -I Subject < $inp_file`
echo $subject | grep "failure notice" > /dev/null 2>&1
if [ $? -eq 0 ] ; then
    (
    /usr/bin/822header < $inp_file
    echo
    echo "sorry about that"
    /usr/bin/822body < $inp_file
    echo "sent by IndiMail Messaging platform"
    ) > $out_file
    if [ $? -ne 0 ] ; then
        /bin/rm -f $inp_file $out_file
        exit 111
    fi
    exec 0<$out_file
else
    exec 0<$inp_file
fi
/bin/rm -f $inp_file $out_file
#
# unset RPLINE so that maildirdeliver does not add a duplicate Return-Path line
# unset PWSTRUCT so that password structure cached is removed for vuserinfo to
# work correctly
#
unset RPLINE PWSTRUCT
dir=`/usr/bin/vuserinfo -d testuser01@example.com | cut -d: -f2 |cut -c2-`

if [ $? -ne 0 -o " $dir" = " " ] ; then
    echo "unable to get user's homedir" 1>&1
    exit 111
fi
exec /usr/bin/maildirdeliver "$dir"/Maildir
exit 111
```

# IndiMail Delivery mechanism explained

Any email that needs to be delivered needs to be put into a queue before it can be taken up for delivery. Email can be submitted to the queue using <b>qmail-queue</b> command or qmail\_open() function. The following programs use the qmail\_open() API -
<b>condredirect</b>, <b>dot-forward</b>, <b>fastforward</b>, <b>filterto</b>, <b>forward</b>, <b>maildirserial</b>, <b>new-inject</b>, <b>ofmipd</b>, <b>qmail-inject</b>, <b>qmail-local</b>, <b>qmail-qmqpd</b>, <b>qmail-qmtpd</b>, <b>qmail-queue</b>, <b>qmail-send</b>, <b>qreceipt</b>, <b>replier</b>, <b>rrforward</b>, <b>qmail-smtpd</b>. Of these, <b>qmail-smtpd</b> and <b>qmail-qmtpd</b> accept an email for a domain only if the domain is listed in rcpthosts. Once an email is accepted into the queue, <b>qmail-send</b>(8) decides if the mail is to be delivered locally or to a remote address. If the email address corresponds to a domain listed in locals or virtualdomains control file, steps are taken to have the email delivered locally.

## Delivery Mode

The delivery mode depends on the argument passed to qscheduler during startup. The script /service/qmail-send.25/run passes the content of the file /etc/indimail/control/defaultdelivery as an argument to qscheduler.
See INSTALL.mbox, INSTALL.maildir, and INSTALL.vsm for more information.
To select your default mailbox type, just enter the defaultdelivery value from the table into /etc/indimail/control/defaultdelivery.
e.g., to select the standard qmail Maildir delivery, do:

`echo ./Maildir/ >/etc/indimail/control/defaultdelivery`

## Addresses
Once you have decided the delivery mode above, one needs to have some mechanism to assign a local address for the delivery. qmail (which is what IndiMail uses) offers the following mechanism
locals
Any email addressed to user@domain listed in the file /etc/indimail/control/locals will be delivered to the local user user. If you have Maildir as the delivery mode and an email to user kanimoji@domain, with home directory /home/blackmoney, will be delivered to /home/blackmoney/Maildir/new
virtualdomains
The control file /etc/indimail/control/virtualdomains allows you to have multiple domains configured on a single server. Entries in virtualdomains are of the form:

`user@domain:prepend`

qmail converts user@domain to prepend-user@domain and treats the result as if domain was local. The user@ part is optional. If it's omitted, the entry matches all @domain addresses.
When you run the command

`$ sudo vadddomain example.com some_password`

It will add the following entry in virtualdomains control file

```
$ cat /etc/indimail/control/virtualdomains
example.com:example.com
```

What this means is that any email addressed to user@example.com will be delivered to the address example.com-user@example.com.
IndiMail further uses qmail-users mechanism to deliver the email for users in a virtual domain. This is explained below

## qmail-users

The file /etc/indimail/users/assign assigns addresses to users.
A simple assignment is a line of the form

`=local:user:uid:gid:homedir:dash:ext:`

Here local is an address; user, uid, and gid are the account name, uid, and gid of the user in charge of local; and messages to local will be controlled by

`homedir/.qmaildashext`

If there are several assignments for the same local address, <b>qmail-lspawn</b> will use the first one. local is interpreted without regard to case.

A wildcard assignment is a line of the form

`+loc:user:uid:gid:homedir:dash:pre:`

This assignment applies to any address beginning with loc, including loc itself.  
It means the same as

`=locext:user:uid:gid:homedir:dash:preext:`

for every string ext.
When you add a virtualdomain using vadddomain, you will have the following entry

```
+example.com-:example.com:555:555:/var/indimail/domains/example.com:-::
```

As stated earlier, any email addressed to user@example.com will be delivered to local user example.com-user@example.com because of virtualdomains control file. The above address can be looked as

```
=user@example.com:example.com:555:555:/var/indimail/domains/example.com:-:user:
```

So you can see that emails are controlled by .qmail-user in the directory /var/indimail/domains/example.com. if .qmail-user does not exist, then .qmail-default will be used
Adding the entry

```
+example.com-customer_care-:example.com:555:555:/var/indimail/domains/example.com/cc:-::
```

will cause emails to `customer_care-delhi@example.com`, `customer_care-mumba@example.com`, etc to be handled by /var/indimail/domains/cc/.qmail-default (if `.qmail-customer_care-delhi` does does not exist).

## Extension Addresses

In the qmail system, you control all local addresses of the form user-anything, as well as the address user itself, where user is your account name. Delivery to user-anything is controlled by the file homedir/.qmail-anything. (These rules may be changed by editing the assign file as given above in qmail-users.

The alias user controls all other addresses. Delivery to user is controlled by the file homedir/.qmail-user, where homedir is alias's home directory.
In the following description, <b>qmail-local</b> is handling a message addressed to local@domain, where local is controlled by .qmail-ext. Here is what it does.
If .qmail-ext is completely empty, <b>qmail-local</b> follows the defaultdelivery instructions set by your system administrator.
If .qmail-ext doesn't exist, <b>qmail-local</b> will try some default .qmail files. For example, if ext is foo-bar, <b>qmail-local</b> will try first .qmail-foo-bar, then .qmail-foo-default, and finally .qmail-default. If none of these exist, <b>qmail-local</b> will bounce the message. (Exception: for the basic user address, <b>qmail-local</b> treats a nonexistent .qmail the same as an empty .qmail.)
The vadddomain command creates the file .qmail-default in /var/domain/domains/domain\_name. Hence any email addressed to user@example.com gets controlled by /var/indimail/domains/example.com/.qmail-default.

WARNING: For security, <b>qmail-local</b> replaces any dots in ext with colons before checking .qmail-ext. For convenience, <b>qmail-local</b> converts any uppercase letters in ext to lowercase.

# Controlling Delivery Rates

<b>qmail-send</b> has the ability to control delivery rates of jobs scheduled for delivery. Every queue can have a directory named <b>ratelimit</b> where you can store rate control definition for a domain. These rate control definition files can be created using the <b>drate</b> command. All you need to provide is a mathematical expression that defines your rate. e.g. 100/3600 means 100 emails per hour. This rate control works at the queue level. indimail-mta uses multiple queues. Each queue has its own delivery handled by by its own set of <b>todo-proc</b>, <b>qmail-send</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, <b>qmail-clean</b> daemons. Enforcing rate control on such a queue has a practical problem of having to define rate control definition for each and every queue.

To avoid the practical problem of rate limiting every queue, we can use a dedicated queue to handle rate controllled delivery. The default indimail installation creates a special queue named as <b>slowq</b> and uses a daemon <b>slowq-send</b> instead of <b>qmail-send</b> to process the <u>slowq</u> queue. This is done by calling <b>slowq-start</b> instead of <b>qscheduler</b> / <b>qmail-start</b>. <b>slowq-send</b> doesn't have its own <b>todo-proc</b> process. We don't require it as we aren't looking at high delivery rates for this queue. <b>slowq-start</b> is invoked using a supervised service in <u>/service/slowq</u>.

To create this special slowq service for delivery rate control you can use svctool as below. This will also create the queue <u>/var/indimail/queue/slowq</u> and logs to be in <u>/var/log/svc/slowq/current</u>.

```
# Create supervised service to call slowq-start instead of qmail-start
# This will also create a special queue in /var/indimail/queue/slowq that
# has a subdir named ratelimit to store rate defintion files for any domain

$ sudo /usr/sbin/svctool --slowq --servicedir=/service \
  --qbase=/var/indimail/queue --cntrldir=control \
  --persistdb --starttls --fsync --syncdir \
  --dmemory=83886080 --min-free=52428800 \
  --dkverify=dkim --dksign=dkim \
  --private_key=/etc/indimail/control/domainkeys/%/default \
  --remote-authsmtp="plain" --localfilter --remotefilter \
  --deliverylimit-count="-1" --deliverylimit-size="-1"
Creating delivery only slowq
Creating Queue /var/indimail/queue/slowq
```

Having a dedicated service for rate controlled delivery also avoids having the main queue clogged up with email that need to be held back from delivery.

Below is an example of having emails to yahoo.com throttled to no more than 50 emails per hour

```
# create rate control definition file for yahoo.com which
# caps the delivery to a max of 50 emails per hour

$ sudo drate -d yahoo.com -r 50/3600

# Query the rate defition for yahoo.com

$ drate -d yahoo.com
Conf   Rate: 50/3600 (0.0138888888)
Email Count: 0
Start  Time: Fri, 28 May 2021 21:47:11 +0530
End    Time: Fri, 28 May 2021 21:47:11 +0530
CurrentTime: Fri, 28 May 2021 21:48:23 +0530
CurrentRate: 0.0000000000
```

During the course of delivery you can use the drate command to display the current delivery date.

```
# Query the rate defition for yahoo.com

$ drate -d yahoo.com
Conf   Rate: 5/3600 (0.0013888888)
Email Count: 70
Start  Time: Fri, 28 May 2021 21:47:11 +0530
LastUpdated: Sat, 29 May 2021 22:07:41 +0530
CurrentTime: Tue, 1 Jun 2021 10:23:01 +0530
CurrentRate: 0.0002298473 OK
```

Now that we have configured a separate queue <b>slowq</b> for rate controlled delivery, we need to queue emails for the configured domain in this queue rather than the any of the regular indimail-mta's multiple queues. To do that we must have any entry in the control file <b>domainqueue</b> to set the <b>QUEUEDIR</b> environment variable when using <b>qmail-inject</b> for sendmail. You can create, edit, delete entries from <b>domainqueue</b> using your favourite text editor.

```
$ cat /etc/indimail/control/domainqueue
# format of this file is
# domain:env variables to set or unset.
# e.g.
# domain:ENV1=val1,ENV2=,ENV3=val3
#
argos.indimail.org:QUEUEDIR=/var/indimail/queue/slowq
yahoo.com:QUEUEDIR=/var/indimail/queue/slowq
```

Once the delivery rate for a configured domain reaches the configured rate, emails will get queued but will not be picked up immediately for delivery. The <b>slowq-send</b> logs will display when this happens. As you can see, the delivery finally happens when the delivery rate becomes lesser than the configured rate.

```
2021-06-05 20:42:52.803287500 new msg 932523
2021-06-05 20:42:52.803328500 info msg 932523: bytes 792 from <mbhangui@argos.indimail.org> qp 116584 uid 1000 slowq
2021-06-05 20:42:52.803475500 local: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 932523 slowq
2021-06-05 20:42:52.803574500 info: slowq argos.indimail.org msg 932523: rate [1/0.0000000000/0.0013888888] ok since 6 secs
2021-06-05 20:42:52.803601500 starting delivery 4: msg 932523 to local mbhangui@argos.indimail.org slowq
2021-06-05 20:42:52.803603500 status: local 1/5 remote 0/5 slowq
2021-06-05 20:42:52.844974500 delivery 4: success: did_1+0+0/ slowq
2021-06-05 20:42:52.845134500 status: local 0/5 remote 0/5 slowq
2021-06-05 20:42:52.845236500 end msg 932523 slowq
2021-06-05 20:42:57.593358500 new msg 932523
2021-06-05 20:42:57.593401500 info msg 932523: bytes 792 from <mbhangui@argos.indimail.org> qp 116594 uid 1000 slowq
2021-06-05 20:42:57.593688500 local: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 932523 slowq
2021-06-05 20:42:57.593692500 warning: slowq argos.indimail.org msg 932523: rate exceeded [2/0.0909090936/0.0013888888] need 709 secs; will try again later
2021-06-05 20:42:57.593695500 status: local 0/5 remote 0/5 delayed jobs=1 slowq
2021-06-05 20:43:11.447111500 new msg 933364
2021-06-05 20:43:11.447160500 info msg 933364: bytes 792 from <mbhangui@argos.indimail.org> qp 116603 uid 1000 slowq
2021-06-05 20:43:11.447436500 local: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 933364 slowq
2021-06-05 20:43:11.447439500 warning: slowq argos.indimail.org msg 933364: rate exceeded [2/0.0399999991/0.0013888888] need 695 secs; will try again later
2021-06-05 20:43:11.447441500 status: local 0/5 remote 0/5 delayed jobs=2 slowq
2021-06-05 20:54:47.546702500 info: slowq argos.indimail.org msg 932523: rate [2/0.0013869625/0.0013888888] ok since 1 secs
2021-06-05 20:54:47.546711500 starting delivery 5: msg 932523 to local mbhangui@argos.indimail.org slowq
2021-06-05 20:54:47.546715500 status: local 1/5 remote 0/5 delayed jobs=1 slowq
2021-06-05 20:54:47.590367500 delivery 5: success: did_1+0+0/ slowq
2021-06-05 20:54:47.590563500 status: local 0/5 remote 0/5 delayed jobs=1 slowq
2021-06-05 20:54:47.590741500 warning: slowq argos.indimail.org msg 933364: rate exceeded [3/0.0027739251/0.0013888888] need 719 secs; will try again later
2021-06-05 20:54:47.590748500 status: local 0/5 remote 0/5 delayed jobs=1 slowq
2021-06-05 20:54:47.590829500 end msg 932523 slowq
2021-06-05 21:06:48.690469500 info: slowq argos.indimail.org msg 933364: rate [3/0.0013869625/0.0013888888] ok since 2 secs
2021-06-05 21:06:48.690478500 starting delivery 6: msg 933364 to local mbhangui@argos.indimail.org slowq
2021-06-05 21:06:48.690483500 status: local 1/5 remote 0/5 slowq
2021-06-05 21:06:48.734485500 delivery 6: success: did_1+0+0/ slowq
2021-06-05 21:06:48.734666500 status: local 0/5 remote 0/5 slowq
2021-06-05 21:06:48.734762500 end msg 933364 slowq
```

# Delivery Instructions for a virtual domain

IndiMail uses a modified version of qmail as the MTA. For local deliveries, <b>qmail-lspawn</b> reads a series of local delivery commands from descriptor 0, invokes <b>qmail-local</b> to perform the deliveries. <b>qmail-local</b> reads a mail message and delivers to to a user by the procedure described in dot-qmail(5). IndiMail uses <b>vdelivermail</b> as the local delivery agent.
A virtual domain is created by the command vadddomain(1).

`$ sudo vadddomain example.com some_password`

The above command creates a virtual domain with delivery instructions in /var/indimail/domains/example.com/.qmail-default file. A line in this file is of the form

```
/usr/sbin/vdelivermail '' delivery_instruction_for_non_existing_user
```

The `delivery_instruction_for_non_existing_user` can have one of the following 5 forms

 1. delete
 2. bounce-no-mailbox
 3. Maildir
 4. emailAddress
 5. IPaddress

* Using **delete** as the delivery instruction causes IndiMail to discard all mails addressed to non-existing users. The original sender does not get notified of the delivery. On a real messaging system serving real users, you will not want to do this.
* The instruction **bounce-no-mailbox** causes a bounce to be generated to the sender in case an email is addressed to a non-existing user. This is the most common usage in .qmail-default which most IndiMail installations will have
* The instruction **Maildir** causes emails to be addressed to non-existing users to be saved in a Maildir. Here Maildir should refer to a full path of an existing Maildir.
* The instruction **emailAddress** causes emails to be addressed to non-existing users to be forwarded to an email address emailAddress.
* The instruction **IPaddress** causes emails to be addressed to non-existing users to be redirected to a remote SMTP server at IP IPaddress. The format of IPaddress is domain:ip:port where domain is the domain name, ip is the IP address of the remote SMTP server and port is the SMTP port on the remote SMTP server. It is expected that the non-existing user is present on the remote system. This type of delivery is used by IndiMail on a clustered setup. In a clustered setup, users are distributed across multiple server. A particular user will be located only on one particular server. However, the same domain will be present on multiple servers.

In the delivery instruction in .**qmail-default**, you can replace **vdelivermail** with **vfilter** to perform in-line filtering use IndiMail's poweful vfilter. You can create filters using the program **vcfilter**.

# Setting up TLS for SMTP (qmail-smtpd) and Remote Delivery (qmail-remote)

indimail-mta supports multiple methods to achieve Transport Layer Security for both incoming and outgoing delivery through SMTP. It all depends on how you configure few variables. We will discuss below various methods along with examples. The example will demonstrate the versality of programs like [tcpserver(1)](https://github.com/mbhangui/indimail-mta/wiki/tcpserver.1), [tcpclient(1)](https://github.com/mbhangui/indimail-mta/wiki/tcpclient.1), [qmail-smtpd(8)](https://github.com/mbhangui/indimail-mta/wiki/qmail-smtpd.8) and [qmail-remote(8)](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8). You can also use [dotls(1)](https://github.com/mbhangui/indimail-mta/wiki/dotls.1) - A transparent TLS/SSL proxy that can help you to secure traffic for non-tls distributions like qmail, netqmail and others.

## Using tcpserver to provide encrypted SMTPS service

tcpserver which comes with indimail-mta, can by itself provides TLS. All you have to do is use -n option and provide a Privacy Enhanced Mail (PEM) file as the certificate. The below example shows how to do that. It also shows the flexibility of indimail-mta to queue the mail anywhere of your chosing and deliver the mail to its destination.

First let us create a queue and start an encrypted ucspi-tcp service using tcpserver on port 2025. You can use port 25 too, but then you will have to run tcpserver as root. We will use queue-fix to create a queue in the home directory. For me it is /home/manny. The [queue-fix](https://github.com/mbhangui/indimail-mta/wiki/queue-fix.8) command will create a queue named demo. We will use two terminals. One for running tcpserver to accept mail using SMTP protocol and another for running tcpclient to carry out a simple SMTP transaction.

**Terminal 1** run tcpserver

```
# create a queue named 'demo' in your home directory
$ cd /home/manny
$ sudo queue-fix demo

# Run tcpserver on port 2025
$ tcpserver -v -n /etc/indimail/certs/servercert.pem 0 2025 \
    env BIGTODO=0 CONFSPLIT=23 QUEUEDIR=/home/manny/demo qmail-smtpd
tcpserver: status: 0/40
```

**Terminal 2** run tcpclient

Here we demonstrate a SMTP transaction using tcpclient. You can also use the openssl command `openssl s_client -connect localhost:2025`

```
$ tcpclient -n "" 0 2025
220 argos.indimail.org (NO UCE) ESMTP IndiMail 1.283 Fri, 6 Jan 2023 19:51:47 +0530
mail from:<manvendra@indimail.org>
250 ok
rcpt to:<manny@argos.indimail.org>
250 ok
data
354 go ahead
To: Manny <manny@argos.indimail.org>
From: Manvendra <manvendra@indimail.org>
Subject: Test message for Demo purpose

This is a test message
.
250 ok 1673014999 qp 68729
quit
221 argos.indimail.org closing connection
```

On Terminal 2 you should see that the mail has been successfully queued (a message like `250 ok 1673014999 qp 68729`).

```
... contd. from above
tcpserver: status: 1/40
tcpserver: pid 68722 from ::1
tcpserver: ok 68722 [::1]2025 [::1]:49105:maxperip=20
qmail-smtpd: pid 68723 gsasl header version=2.2.0 library version=2.2.0
qmail-smtpd: pid 68723 from ::1 HELO <unknown> MAIL from <manvendra@indimail.org> RCPT <manny@argos.indimail.org> AUTH <local-rcpt> Size: 141 TLS=tcpserver
tcpserver: status: 0/40
```

To deliver the message from this custom queue all you have to do is run qmail-start

```
$ sudo env QUEUEDIR=/home/manny/demo CONFSPLIT=23 BIGTODO=0 qmail-start ./Maildir/
info: qmail-send: /home/mbhangui/demo: ratelimit=OFF, loglock=disabled, conf split=23
status: local 0/10 remote 0/20 demo
info: todo-proc: /home/mbhangui/demo: conf split=23, todo chunk size=1, bigtodo=no
new msg 33829336 demo
info msg 33829336: bytes 566 from <manvendra@indimail.org> qp 68729 uid 0 demo
local: manvendra@indimail.org manny@argos.indimail.org 33829336 566 bytes demo opendir mode
starting delivery 1: msg 33829336 to local manny@argos.indimail.org demo
status: local 1/10 remote 0/20 demo
delivery 1: success: did_0+1+0/qp_68839/ demo
status: local 0/10 remote 0/20 demo
end msg 33829336 qmail-send: demo
```

The above qmail-send log shows that the message with **qp 68729** was successfully delivered. The next interesting thing is that indimail-mta puts details of the encrypted channel in the `Received` header in the message. It also tells you that the tls provider was tcpserver.

```
Received: indimail-mta smtpd 68723 (::1)
  by argos.indimail.org with ESMTPS
  (tls provider tcpserver, TLS Server=TLSv1.3, Client=TLSv1.3, Ciphers=TLS_AES_256_GCM_SHA384); Fri, 6 Jan 2023 19:52:09 +0530
```

indimail-mta has five programs that are TLS providers - tcpserver, tcpclient, dotls, qmail-smtpd and qmail-remote. Out of these tcpclient, dotls support STARTTLS method to connect to any SMTP server. qmail-smtpd supports native SMTPS as well as STARTTLS. qmail-remote can connect to any SMTPS server as well as use STARTTLS to start delayed encryption.

## Using dotls provide encrypted SMTPS service or STARTTLS capability

The method we used above, where we used tcpserver, only those SMTP clients that have the ability to do TLS/SSL encryption can transact. To support clients that do not have this ability, a SMTP server needs to provide the STARTTLS capability. But many SMTP servers don't have TLS/SSL capability and neither do they have STARTTLS capability. A proxy that sits between a TLS client and a plain-text communication SMTP server can solve this problem. The program dotls is exacty that - a proxy that provides encrypted channel for programs that do not support TLS/SSL and STARTTLS capability. Using dotls is very simple. All you have to do is insert dotls in your tcpserver command line. Once you do that any TLS client will be able to communicate with your non-TLS application through dotls. The first example shows how to use dotls to provide SMTPS. The second example shows how to use dotls to provide STARTTLS capability. Here we demonstrate how to use dotls to provide SMTPS or STARTTLS for qmail, netqmail, notqmail which do not have native SMTPS/STARTTLS capability in their SMTP server. Just like the earlier example you can use tcpclient or openssl command to carry out a SMTP transaction. When you use dotls, the `Received` headers will show the TLS provider as dotls. The dotls program is as generic as it can be. It can be used for any program that can be run on the tcpserver command line. dotls also provides STLS capability for any non-tls pop3 daemon that can be run under tcpserver.

**Provide SMTPS cabability using dotls**

Here we use /var/qmail/bin/qmail-smtpd which comes from netqmail. This will work for qmail, notqmail and s/qmail too. Not a single line needs to be modified to provide SMTPS for these third party SMTP servers. Since telnet doesn't support TLS, we use tcpclient to connect to the SMTPS port. You can very well use `openssl s_client -connect localhost:2025`

```
$ tcpserver 0 2025 /usr/bin/dotls /var/qmail/bin/qmail-smtpd &
[1] 251063

$ tcpclient -n "" 0 2025
220 argos.indimail.org ESMTP
help
214 netqmail home page: http://qmail.org/netqmail
```

**Provide SMTPS STARTTLS cabability using dotls**

Here we use /var/qmail/bin/qmail-smtpd which comes from notqmail. This will work for qmail, netqmail and s/qmail too. Not a single line needs to be modified to provide STARTTLS on behalf these third party SMTP servers. Here we have used telnet to connect to port 2025. You could also have used `openssl s_client -starttls smtp -connect localhost:2025`. However, telnet cannot be used once you issue the starttls command. For that you can use tcpclient or the openssl command.

```
# Run tcpserver in the background

$ tcpserver 0 2025 /usr/bin/dotls -s smtp /var/qmail/bin/qmail-smtpd &
[1] 251597

# First example: using telnet to connect to port 2025

$ telnet 0 2025
Trying 0.0.0.0...
Connected to 0.
Escape character is '^]'.
220 argos.indimail.org ESMTP
ehlo
250-argos.indimail.org
250-PIPELINING
250-STARTTLS
250 8BITMIME
help
214 notqmail home page: https://notqmail.org

# Second example: using tcpclient to connect to port 2025
# and issue the STARTTLS command

$ tcpclient -s smtp -n "" 0 2025
220 argos.indimail.org ESMTP
ehlo
250-argos.indimail.org
250-PIPELINING
250 8BITMIME
help
214 notqmail home page: https://notqmail.org
```

## Using qmail-smtpd to provide SMTPS and STARTTLS

In the preceding examples we learnt how to use tcpserver and dotls to carry out a secure SMTP transaction. In fact, qmail-smtpd by itself has native SMTPS/STARTTLS capability. indimail-mta's qmail-smtpd can work without tcpserver providing TLS capabilities. To use SMTPS all you have to do is set the SMTPS environment variable. You can use TLS/SSL capability of tcpclient to connect to the SMTPS service

```
# Run tcpserver in the background

$ tcpserver 0 2025 env BIGTODO=0 SMTPS=1 CONFSPLIT=23 QUEUEDIR=/home/mbhangui/demo qmail-smtpd &
[1] 180420

$ tcpclient -n "" 0 2025
qmail-smtpd: pid 180431 gsasl header version=2.2.0 library version=2.2.0
qmail-smtpd: pid 180431 from ::1 ssl-version=TLSv1.3
220 argos.indimail.org (NO UCE) ESMTP IndiMail 1.284 Sat, 7 Jan 2023 10:43:53 +0530
quit
221 argos.indimail.org closing connection
```

Without the SMTPS environment variable qmail-smtpd supports STARTTLS capability if it finds the environment variable <b>STARTTLS</b> set. Here we show how tcpclient can be used even for non-encrypted connection. You can use the popular `telnet` command too. In fact the tcpclient binary that comes with indimail-mta can be used like telnet for both encrypted as well as non-encrypted connections. However, with telnet you cannot use STARTTLS. For that we can use tcpclient as shown in the second example below (you can also use `openssl s_client -starttls smtp localhost:2025`)

```
# Run tcpserver in the background

$ tcpserver 0 2025 env BIGTODO=0 STARTTLS=1 CONFSPLIT=23 QUEUEDIR=/home/mbhangui/demo qmail-smtpd &
[1] 69996

# First example: using telnet to connect to port 2025

$ tcpclient 0 2025
220 argos.indimail.org (NO UCE) ESMTP IndiMail 1.283 Fri, 6 Jan 2023 20:29:32 +0530
ehlo
250-argos.indimail.org [::1]
250-PIPELINING
250-8BITMIME
250-SIZE 10000000
250-ETRN
250-STARTTLS
250 HELP

# Second example: using tcpclient to connect to port 2025
# and issue the STARTTLS command. You will not see the STARTTLS
# capability because tcpclient has already issued the STARTTLS
# command

$ tcpclient -s smtp -n "" 0 2025
220 argos.indimail.org (NO UCE) ESMTP IndiMail 1.284 Sun, 8 Jan 2023 20:49:04 +0530
ehlo
250-argos.indimail.org [::1]
250-PIPELINING
250-8BITMIME
250-SIZE 10000000
250-ETRN
250 HELP
help
214-This is IndiMail SMTP Version 1.284
214-https://github.com/mbhangui/indimail-mta
214-This server supports the following commands:
214 HELO EHLO RSET NOOP MAIL RCPT DATA VRFY ETRN HELP QUIT
```

When you setup qmail-smtpd to do the encryption, you will find the following `Received` header

```
Received: indimail-mta smtpd 176367 from localhost (HELO argos.indimail.org) (::ffff:127.0.0.1)
  by argos.indimail.org with (TLSv1.3 TLS_AES_256_GCM_SHA384 encrypted) ESMTPS; Sat, 7 Jan 2023 08:14:56 +0530
```
The TLS/SSL connection can be customized by the following control files

*tlsservermethod* - The TLS protocol list. Accepted values are SSLv23, SSLv3, TLSv1, TLSv1\_1, TLSv1\_2, TLSv1\_3. The default is TLSv1.2 for OpenSSL Version \< 1.0.1. OpenSSL Version >= 1.0.1 uses TLS\_server\_method() where the actual protocol version used will be negotiated to the highest version mutually supported by the client and the server. The supported protocols are SSLv3, TLSv1, TLSv1.1, TLSv1.2 and TLSv1.3

*tlsserverciphers* - A set of OpenSSL cipher strings. Multiple ciphers contained in a string should be separated by a colon. If the environment variable **TLS\_CIPHER\_LIST** is set to such a string, it takes precedence.  *tlsclients*  A list of email addresses. When relay rules would reject an incoming message, **qmail-smtpd** can allow it if the client presents a certificate that can be verified against the CA list in *clientca.pem* and the certificate email address is in *tlsclients*.

*servercert.pem* - SSL certificate to be presented to clients in TLS-encrypted sessions.  Should contain both the certificate and the private key. Certifying Authority (CA) and intermediate certificates can be added at the end of the file. The filename can be overriden by the environment variable **SERVERCERT**. The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

*clientca.pem* - A list of Certifying Authority (CA) certificates that are used to verify the client-presented certificates during a TLS-encrypted session. The filename can be overriden by the environment variable **CLIENTCA**. The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

*clientcrl.pem* - A list of Certificate Revocation Lists (CRLs). If present it should contain the CRLs of the CAs in *clientca.pem* and client certs that will be checked for revocation. The filename can be overriden by the environment variable **CLIENTCRL**. The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

*rsa512.pem* - If this 512 bit RSA key is provided, **qmail-smtpd** will use it for TLS sessions instead of generating one on-the-fly. The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

*dh1024.pem* - If these 1024 bit DH parameters are provided, **qmail-smtpd** will use them for TLS sessions instead of generating one on-the-fly (which is very timeconsuming). The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

*dh512.pem* - 512 bit counterpart for **dh1024.pem.** The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR.

## Setting up qmail-remote to do STARTTLS

[qmail-remote](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8) will issue the STARTTLS command if it finds the remote server supports STARTTLS and it finds a valid certificate in <u>/etc/indimail/certs/clientcert.pem</u>. qmail-remote will revert to non-encrypted session if the TLS connection fails. Let's us invoke qmail-remote on the command line to simulate sending an email through a TLS session.

```
# create a sample rfc-822 email in /tmp/mail.txt

(
 echo "From: manny@example.com"
 echo "To: mbhangui@argos.indimail.org"
 echo "Subject: Test message"
 echo "Date: $(date --rfc-email)"
 echo
 echo "Test message"
) > /tmp/mail.txt

$ ls -l /tmp/mail.txt
-rw-r--r-- 1 mbhangui mbhangui 130 Jan  9 10:26 /tmp/mail.txt

# Run tcpserver in the background on terminal 1 which will run qmail-smtpd with STARTTLS capability enabled
# This capability will be seen by qmail-remote when it does EHLO

$ tcpserver 0 2025 env STARTTLS=1 BIGTODO=0 CONFSPLIT=23 QUEUE_START=1 QUEUE_COUNT=5 QUEUE_BASE=/var/indimail/queue qmail-smtpd &
[1] 7641

# Run qmail-remote by setting SMTPROUTE variable to your localhost. Run this on terminal 2

$ env SMTPROUTE='argos.indimail.org:127.0.0.1:2025' qmail-remote argos.indimail.org manny@example.com "" 130 mbhangui@argos.indimail.org </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <mbhangui@argos.indimail.org> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1673240254 qp 7784

# This is what qmail-smtpd will log on descriptor 2 on terminal 1 when qmail-remote above does a SMTP transaction

qmail-smtpd: pid 7783 gsasl header version=2.2.0 library version=2.2.0
qmail-smtpd: pid 7783 from ::ffff:127.0.0.1 ssl-version=TLSv1.3
qmail-smtpd: pid 7783 from ::ffff:127.0.0.1 HELO <argos.indimail.org> MAIL from <manny@example.com> RCPT <mbhangui@argos.indimail.org> AUTH <local-rcpt> Size: 130 TLS=TLSv1.3
```

The TLS/SSL connection can be customized by the following control files

*tlsclientmethod* - The TLS protocol list. Accepted values are SSLv23, SSLv3, TLSv1, TLSv1\_1, TLSv1\_2, TLSv1\_3. The default is TLSv1\_2 for OpenSSL Version \< 1.0.1. Without this control file OpenSSL Version >= 1.0.1 uses TLS\_client\_method(3ossl) where the actual protocol version used will be negotiated to the highest version mutually supported by the client and the server. The supported protocols are SSLv3, TLSv1, TLSv1.1, TLSv1.2 and TLSv1.3. The default location of /etc/indimail/control can be overriden by environment variable **CONTROLDIR**.

*tlsclientciphers* - A set of OpenSSL client cipher strings. Multiple ciphers contained in a string should be separated by a colon. The default location of /etc/indimail/control can be overriden by environment variable **CONTROLDIR**

*notlhosts* - domains for which **qmail-remote** will not initiate TLS sesson. This file in /etc/indimail/control or directory defined by the **CONTROLDIR** envirnoment variable, shouldn't be confused with /etc/indimail/certs/*notlshosts* directory in the directory or the directory defined by the **CERTDIR** environment variable.

For TLS sessions, the default location of /etc/indimail/certs can be overriden by environment variable CERTDIR. This affects the location of below files/directories *clientcert.pem*, *servercert.pem*, *tlshosts/\<FQDN>.pem*, *tlshosts/exhaustivelist*, *notlshosts/\<FQDN>*, *notlshosts/host*.

*clientcert.pem* - SSL certificate that is used to authenticate with the remote server during a TLS session. If clientcert.pem does not exist, **qmail-remote** will not negotiate TLS. The default location of /etc/indimail/certs can be overriden by environment variable CERTDIR. *clientcert.pem* can be overriden by environment variable **CLIENTCERT**.

*tlshosts/\<FQDN>.pem* - **qmail-remote** requires TLS authentication from servers for which this file exists (**\<FQDN>** is the fully-qualified domain name of the remote SMTP server). One of the *dNSName* or the *CommonName* attributes have to match. The file contains the trusted CA certificates. The default location of /etc/indimail/certs can be overriden by environment variable **CERTDIR**.

*tlshosts/exhaustivelist* - if this file exists no TLS will be tried on hosts other than those for which a file **tlshosts/\<FQDN>.pem** exists. The default location of /etc/indimail/certs can be overriden by environment variable **CERTDIR**.

**WARNING:** this option may cause mail to be delayed, bounced, doublebounced, or lost.

*notlshosts/\<FQDN>* - **qmail-remote** will not try TLS on servers for which this file exists (**\<FQDN>** is the fully-qualified domain name of the remote SMTP server). *(tlshosts/\<FQDN>.pem* takes precedence over this file however). The default location of /etc/indimail/certs can be overriden by environment variable **CERTDIR**.

*notlshosts/host* - **qmail-remote** will not try TLS on servers for which this file exists (**host** is the domain name of the recipient). *(tlshosts/\<FQDN>.pem* takes precedence over this file however). The default location of /etc/indimail/certs can be overriden by environment variable **CERTDIR**.

## Updating RSA and DH parameters

A script *update\_tmprsadh* in cron uses the following openssl commands to pre-generate 2048 bites RSA and DH parameters. You can pass --maxbits argument to *update\_tmprsadh* to generate these with higher bits. You can set the environment variable **SSL\_BITS** to make tcpclient choose specific bits for the RSA/DH parameters. These files are generated in /etc/indimail/certs. *update\_rmprsadh* is installed to be run by cron if you have installed indimail from Open Build Service.

```
cd /etc/indimail/certs
/usr/bin/openssl genrsa  -out    rsa2048.pem 2048
/usr/bin/openssl dhparam -out -2  dh2048.pem 2048
```

Some ciphersuites may use ephemeral Diffie-Hellman (DH) key exchange. In these cases, the session data is negotiated using the ephemeral/temporary DH key and the key supplied and certified by the certificate chain is only used for signing. Anonymous ciphers (without a permanent server key) also use ephemeral DH keys.

Using ephemeral DH key exchange yields forward secrecy as the connection can only be decrypted when the DH key is known. By generating a temporary DH key inside the server application that is lost when the application is left, it becomes impossible for an attacker to decrypt past sessions, even if they get hold of the normal (certified) key, as this key was only used for signing.

In order to perform a DH key exchange the server must use a DH group (DH parameters) and generate a DH key. The server will always generate a new DH key during the negotiation.

As generating DH parameters is extremely time consuming, an application should not generate the parameters on the fly. DH parameters can be reused, as the actual key is newly generated during the negotiation.

Typically applications should use well known DH parameters that have built-in support in OpenSSL. The macro SSL\_set\_dh\_auto(3ossl) configures OpenSSL to use the default built-in DH parameters for the SSL object.  Passing a value of 1 in the onoff parameter switches the feature on, and passing a value of 0 switches it off. The default setting is off.

If "auto" DH parameters are switched on then the parameters will be selected to be consistent with the size of the key associated with the server's certificate. If there is no certificate (e.g. for PSK ciphersuites), then it it will be consistent with the size of the negotiated symmetric cipher key.

Applications may supply their own DH parameters instead of using the built-in values. This approach is discouraged and applications should in preference use the built-in parameter support described above.  Applications wishing to supply their own DH parameters should call SSL\_set0\_tmp\_dh\_pkey(3ossl) to supply the parameters for SSL. The parameters should be supplied in the dhpkey argument as an EVP\_PKEY containg DH parameters. Ownership of the dhpkey value is passed to the SSL\_CTX or SSL object as a result of this call, and so the caller should not free it if the function call is succesful..

# Distributing your outgoing mails from Multiple IP addresses

Some mail providers like hotmail, yahoo restrict the number of connections from a single IP and the number of mails that can be delivered in an hour from a single IP. To increase your ability to deliver large number of genuine emails from your users to such sites, you may want to send out mails from multiple IP addresses.

IndiMail has the ability to call a custom program instead of <b>qmail-local</b>(8) or <b>qmail-remote</b>(8). This is done by defining the environment variable QMAILLOCAL or QMAILREMOTE. <b>qmail-remote</b>(8) can use the environment variable OUTGOINGIP to set the IP address of the local interface when making outgoing connections. By writing a simple script and setting QMAILREMOTE environment variable pointing to this script, one can randomly chose an IP address from the control file

`/etc/indimail/control/outgoingip`

The script below also allows you to define multiple outgoing IP addresses for a single host. e.g. you can create the control file to send out mails from multiple IPs only for the domain hotmail.com

`/etc/indimail/control/outgoingip.hotmail.com`

Let us name the below script balance\_outgoing

```
$ sudo /bin/bash
# svc -d /service/qmail-send.25
# echo "/usr/bin/balance_outgoing" > /service/qmail-send.25/variables/QMAILREMOTE
# svc -u /service/qmail-send.25
# exit
$

```

Finally the balance\_outgoing script can be placed with execute bit in /usr/bin

```
# This scripts expects qmail-remote arguments on command line
# argv0          - qmail-remote
# argv1          - host   (host)
# argv2          - sender (sender)
# argv3          - qqeh   (qmail queue extra header)
# argv4          - size
# argv5 .. argvn - recipients

host=$1
sender=$2
qqeh=$3
size=$4
shift 4
cd QMAIL
if [ " $CONTROLDIR" = " " ] ; then
    CONTROLDIR=@controldir@
fi
slash=`echo $CONTROLDIR | cut -c1`
if [ ! " $slash" = " /" ] ; then
    cd QMAIL
fi
FN=$CONTROLDIR/filterargs
if [ -n "$SPAMFILTER" -o -n "$FILTERARGS" -o -f $FN ] ; then
    # execute spawn-filter if you have filters defined for remote/local deliveries
    PROG="bin/spawn-filter"
else
    PROG="bin/qmail-remote"
fi
# Make an array of IP addresses in variable IP
if [ -f $CONTROLDIR/outgoingip.$host ] ; then
    IP=(`cat $CONTROLDIR/outgoingip.$host`)
elif [ -f $CONTROLDIR/outgoingip ] ; then
    IP=(`cat $CONTROLDIR/outgoingip`)
else
    exec -a qmail-remote $PROG "$host" "$sender" "$qqeh" $size $*
fi
IP_COUNT=${#IP[*]} # array size
if [ $IP_COUNT -gt 1 ] ; then
    i=`expr $RANDOM % $IP_COUNT` # chose an IP randomly
    export OUTGOINGIP=${IP[$i]}
fi
exec -a qmail-remote $PROG "$host" "$sender" "$qqeh" $size $*
```

# Handling Bounces

## Bounce Address Tag Validation (BATV)

The sender address can be forged by spammers to look like a legitimate email address. If that email address belongs to you, all bounce backs will be received by your server. Your server ends up handling the entire processing load for the spammer. BATV allows your SMTP server to determine if the original email was ever initiated from your server. This is done by <b>qmail-remote</b> marking each outbound email message with a key. When the bounce back arrives, <b>qmail-smtpd</b> will identify that the key is either present or not present. If the key is not present then <b>qmail-smtpd</b> knows it is not a valid bounce back and drops the email. This feature is known as [Bounce Address Tag Validation (BATV)](https://www.ietf.org/archive/id/draft-levine-smtp-batv-01.html).
BATV allows the qmail-smtpd to distinguish between legitimate and illegitimate bounce (NDR) messages. Once you configure BATV, all outbound messages sent by qmail-remote will be tagged using BATV. To enable BATV in IndiMail, all you need to do is create the <u>batvkey</u> control file. You can define <b>BATVKEY</b> environment variable to refer to another control file. You can also define BATVKEY environment variable for specific users or groups by using envrules. See [Envrules](#envrules).

IndiMail's BATV implementation is based on [draft-levine-mass-batv-02](https://datatracker.ietf.org/doc/html/draft-levine-mass-batv-02).

```
$ cd /etc/indimail/control
$ sudo bash
# tr -dc 'A-Za-z0-9' </dev/urandom | head -c 16 > batvkey
```

You can exclude signing for remote deliveries to hosts that don't support signing by having the host in the control file <u>batvnosignremote</u>. You can also exclude signing for mails for your own domains by having the domain in the control file <u>batvnosignlocals</u>.

You can also control the validity period for bounces by creating the control file <u>batvkeystale</u>. The default validity is 7 days.

We just learnt how to configure IndiMail to rejected forged bounces. For legitimate bounces/NDRs, IndiMail allows a mechanism by which you can use your own script/program to handle bounces. All bounces in IndiMail is generated by <b>qmail-send</b>. <b>qmail-send</b> generates a bounce when <b>qmail-lspawn</b> or <b>qmail-rspawn</b> reports a permanent failed delivery. A bounce is generated by <b>qmail-send</b> by injecting a new mail in the queue using <b>qmail-queue</b>. This bounce generation by <b>qmail-send</b> can be modified in three ways

## Using environment variable BOUNCEPROCESSOR

When you define the environment variable BOUNCEPROCESSOR as a valid path to a program or script, the program gets called whenever a delivery fails permanently. The program runs with the uid *qmails* and is passed the following five arguments

* bounce_file
* bounce_report
* bounce_sender
* original_recipient
* bounce_recipient

To set BOUNCEPROCESSOR, you would do the following

```
$ sudo /bin/bash
# echo "bounce_processor_path" > /service/qmail-send.25/variables/BOUNCEPROCESSOR
# svc -r /service/qmail-send.25
```

There are few email marketing companies who are using BOUNCEPROCESSOR to insert the status of all bounces in MySQL table for their email marketing campaigns.

## Using environment variable BOUNCERULES or control files bounce.envrules.

Using envrules, you can set specific environment variables only for bounced recipients. The format of this file is of the form

`pat:envar1=val,envar2=val,...]`

where pat is a regular expression which matches a bounce recipient.  envar1, envar2 are list of environment variables to be set. If var is omitted, the environment variable is unset.

e.g.

```
support@indimail.org:CONTROLDIR=control2,QMAILQUEUE=/usr/bin/qmail-nullqueue
```

causes all bounces generated for the sender support@indimail.org to be discarded.

## Using BOUNCEQUEUE environment variable to queue bounces

<b>qmail-send</b> uses <b>qmail-queue</b> to queue bounces and aliases/forwards. This can be changed by using QMAILQUEUE environment variable. If a different queue program is desired for bounces, it can be set by using BOUNCEQUEUE environment variable.
e.g

```
$ sudo /bin/bash
# echo /usr/bin/qmail-nullqueue > /service/qmail-send.25/variables/BOUNCEQUEUE
# svc -r /service/qmail-send.25
```

disables bounces system-wide. Though disabling bounces may not be the right thing to do but in some situations where bounces are not at all needed, disabling bounces will surely result in performance improvements of your system, especially so if your system does mass-mailing.

# Setting Disclaimers in your emails

In my earlier article, I showed how to set up automatic rule based archival. I had discussed email archival as one of the many compliance requirements you might have. Sometimes you may also require to configure disclaimers in your messaging system. e.g for UK Companies Act 2006, IRS Circular 230.
IndiMail provides a utility called altermime(1) to add your own disclaimers on each and every mail that goes out through your IndiMail messaging server. You can use any of the two options below to configure disclaimers

## Option 1 - using /etc/indimail/control/filterargs

The filterargs control file allows you to insert any filter before remote or local delivery. You can use altermime to insert a disclaimer as below

```
*:/usr/bin/altermime --input=- --disclaimer=/etc/indimail/control/disclaimer
```

If you want disclaimer to be used only for your outgoing mails then, you could do the following

```
*:remote:/usr/bin/altermime --input=- --disclaimer=/etc/indimail/control/disclaimer
```

In both the above examples the file /etc/indimail/control/disclaimer contains the text of your disclaimer

## Option 2 - Set the FILTERARGS environment variable

Just like filterargs control file, the environment variable FILTERARGS allows you to set any custom filter before your mail gets deposited into the queue by <b>qmail-queue</b>(8).

```
$ sudo /bin/bash
# echo /usr/bin/altermime --input=- --disclaimer=/etc/indimail/control/disclaimer \
  > /service/qmail-smtpd.587/variables/FILTERARGS
# svc -r /service/qmail-smtpd.587
```

Read **altermime**(1) man page for more details

# Email Archiving

IndiMail provides multiple options for those who want their emails archived automatically. For easy retrieval, you can use tools like google desktop, beagle, etc If you use IndiMail, you have two methods to achieve automatic archiving of emails

## using environment variable EXTRAQUEUE

If EXTRAQUEUE environment variable is set to any environment variable, <b>qmail-queue</b> will deposit an extra copy of the email which it receives for putting it in the queue. Normally you would set EXTRAQUEUE variable in any of the clients which use <b>qmail-queue</b>. e.g. <b>qmail-smtpd</b>, <b>qmail-inject</b>, sendmail, etc. If you have setup IndiMail as per the official instructions, you can set EXTRAQUEUE for incoming and outgoing mails as given below

```
$ sudo /bin/bash
# echo "archive@example.com" > /service/qmail-smtpd.25/variables/EXTRAQUEUE
# svc -r /service/qmail-smtpd.25 /service/qmail-smtpd.587
```

Now all your emails coming in and going out of the system, a copy will be sent to archive@example.com. If archive@example.com lies on IndiMail Messaging Platform, you can set filters (using vfilter(1)) to automatically deposit the mails in different folders. The folders can be decided on various criteria like date, sender, recipient, domain, etc.

## using control file mailarchive

This control file allows you to set up rule based archiving. For any specific sender or recipient, you can set a rule to select a destination email for archiving. This is much more flexible than using EXTRAQUEUE which allowed you to archive emails to a single email address. A line in the control file mailarchive can be of the form

`type:regexp:dest_address`

* Here *type* is 'T' to set a rule on recipients. You can set the type as 'F' to set a rule on the sender.
* *regexp* is any email address which matches the sender or recipient (depending on whether type is 'T' or 'F').
* *dest_address* should expand to a valid email address. You can have a valid email address. You can also have the '%' sign followed by the letters u, d or e in the address to have the following substitutions made

```
$u - gets replaced by the user component of email address (without the '@' sign)
$d - gets replaced by the domain component of email address
$e - gets replaced by the email address
```

The email address in the above substitution will be the recipient (if type is 'T') and the sender (if type is 'F').
Here is another example and a cool tip.

`T:*:%u@arch%d`

Will make a hot standby of your incoming mails for *yourdomain* on another server hosting arch*yourdomain*.

For some organizations, email archiving is a must due to compliance with regulatory standards like SOX, HIPAA, Basel II Accord (effective 2006), Canadian Privacy Act, Data Protection Act 1988, EU Data Protection Directive 95/46/FC, Federal Information Security Management Act (FISMA), Federal Rules of Civil Procedure (FRCP), Financial Services Act 198, regulated by FSA, Freedom of Information Act (FOIA), Freedom of Information Act (in force January 2005), The Gramm-Leach-Bliley Act (GLBA), MiFID (Markets in Financial Instruments Directives), PIPEDA (Personal Information Protection and Electronic Documents Act), SEC Rule 17a-4/ NASD 3010 (Securities Exchange Act 1934).

Apart from archiving, you would also want to set disclaimers. IndiMail allows you to set a disclaimer by setting the **FILTERARGS** environment variable and using altermime(1). The following acts/circular specifically require you to set disclaimers.
`UK Companies Act 2006, IRS Circular 230,`

```
Reference
    • Email Compliance - A simple 5 step guide
    • E-Mail archiving - Wikipedia
    • Compliance Requirements for email archiving
    • Email Legislation - Summary of UK, US, EU legislations
```

# Envrules

IndiMail allows you to configure most of its functionality through set of environment variables. In fact there more more than 200 features that can be controlled just by setting or un-setting environment variables. envrules is applicable to <b>qmail-smtpd</b>, <b>qmail-inject</b>, <b>qmail-local</b>, <b>qmail-remote</b> as well. It can also be used to control programs called by the above programs (e.g <b>qmail-queue</b>). IndiMail allows you to configure quite many things using environment variables. Just set the environment variable <b>CONTROLDIR=control2</b> and all qmail components of IndiMail start looking for control files in <u>/etc/indimail/control2</u>. You can set <b>CONTROLDIR=/etc/indimail</b> and all control files can be conveniently placed in <u>/etc/indimail</u>.
Some of these environment variables can be set during the startup of various services. IndiMail has all its services configured as directories in the /service directory. As an example, if you want to force authenticated SMTP on all your users, setting the environment variable <b>REQUIREAUTH</b> allows you to do so.

```
$ sudo /bin/bash
# echo 1 > /service/qmail-smtpd.587/variables/REQUIREAUTH
# svc -r /service/qmail-smtpd.587
```

sets the <b>qmail-smtpd</b> running on port 587 to force authentication.

Setting environment variables in your startup script, in your .profile or your shell forces you to permanently set the environment variable to a specific value. Using **envrules**, IndiMail allows you to set these environment variables specific to different *senders* or *recipients* envrules allows IndiMail platform to be tuned differently for different users. No other messaging platform, to the best of my knowledge, is capable of doing that. Another way of saying is that envrules allows your IndiMail platform to dynamically change its behavior for each and every user.

For the SMTP service, you can set different environment variables for different *senders*. All that is required is to define the following in the control file <u>/etc/indimail/control/from.envrules</u>. The format of this file is of the form

`pattern:envar1=val,envar2=val,...]`

where pattern is a regular expression which matches a sender. envar1, envar2 are list of environment variables to be set. If val is omitted, the environment variable is unset. The name of the control file can be overridden by the environment variable **FROMRULES**. e.g. having the following in from.envrules

`*consultant:REQUIREAUTH=1,NORELAY=1`

forces all users whose email ids end with 'consultant' to authenticate while sending mails. Also such users will be prevented from sending mails to outside your domain.

`ceo@example.com:DATASIZE=`

Removes all message size restrictions for the user whose email address is ceo@example.com, by unsetting the environment variable **DATASIZE**.

You can also set envrules on per recipient basis. This gets set for <b>qmail-local</b> & <b>qmail-remote</b>. The control file to be used in this case is /etc/indimail/control/rcpt.envrules. The filename can be overridden by **RCPTRULES** environment variable.

.e.g

`*.yahoo.com:OUTGOINGIP=192.168.2.100`

The **OUTGOINGIP** environment variable is used by <b>qmail-remote</b> to bind on a specific IP address when connecting to the remote SMTP server. The above envrule forces <b>qmail-remote</b> to use 192.168.2.100 as the outgoing IP address when sending mails to any recipient at yahoo.com.

For SMTP service the following the following list of environment variables can be modified using envrules

```
REQUIREAUTH, QREGEX, ENFORCE_FQDN_HELO, DATABYTES, BADHELOCHECK, BADHELO, BADHOST, BADHOSTCHECK, TCPPARANOID, NODNSCHECK, VIRUSCHECK, VIRUSFORWARD, REMOVEHEADERS, ENVHEADERS, LOGHEADERS, LOGHEADERFD, SIGNATURES, BODYCHECK, BADMAILFROM, BADMAILFROMPATTERNS, BOUNCEMAIL, CUGMAIL, MASQUERADE, BADRCPTTO, BADRCPTPATTERNS, GOODRCPTTO, GOODRCPTPATTERNS, GREYIP, GREETDELAY, CLIENTCA, TLSCIPHERS, SERVERCERT, BLACKHOLERCPT, BLACKHOLERCPTPATTERNS, SIGNKEY, SIGNKEYSTALE, SPFBEHAVIOR, SPFIPV6, SPFRULES, SPFGUESS, SPFEXP, TMPDIR, TARPITCOUNT, TARPITDELAY, MAXRECIPIENTS, MAX_RCPT_ERRCOUNT, AUTH_ALL, CHECKRELAY, CONTROLDIR, ANTISPOOFING, CHECKRECIPIENT, SPAMFILTER, LOGFILTER, SPAMFILTERARGS, SPAMEXITCODE, REJECTSPAM, SPAMREDIRECT, SPAMIGNORE, SPAMIGNOREPATTERNS, FILTERARGS, QUEUEDIR, QUEUE_BASE, QUEUE_START, QUEUE_COUNT, QMAILQUEUE, QUEUEPROG, RELAYCLIENT, QQEH, BADEXT, BADEXTPATTERNS, ACCESSLIST, EXTRAQUEUE, QUARANTINE, QHPSI, QHPSIMINSIZE, QHPSIMAXSIZE, QHPSIRC, QHPSIRN, USE_FSYNC, SCANCMD, PLUGINDIR, QUEUE_PLUGIN, PASSWORD_HASH, MAKESEEKABLE, MIN_FREE, ERROR_FD, DKSIGN, DKVERIFY, DKSIGNOPTIONS, DKQUEUE, DKEXCLUDEHEADERS, DKIMSIGN, DKIMVERIFY, DKIMPRACTICE, DKIMIDENTITY, DKIMEXPIRE, SIGN_PRACTICE DKIMQUEUE, BATVKEY, BATVKEYSTALE, BATVNOSIGNLOCALS, BATVNOSIGNREMOTE, SRS_DOMAIN, SRS_SECRETS, SRS_MAXAGE, SRS_HASHLENGTH, SRS_HASHMIN, SRS_ALWAYSREWRITE, SRS_SEPARATOR, SIGNATUREDOMAINS, and NOSIGNATUREDOMAINS
```

The following list of environment variables can be modified using envrules if QMAILLOCAL and QMAILREMOTE is set to /usr/sbin/spawn-filter.

```
QREGEX, SPAMFILTER, LOGFILTER, SPAMFILTERARGS, FILTERARGS, SPAMEXITCODE, HAMEXITCODE, UNSUREEXITCODE, REJECTSPAM, SPAMREDIRECT, SPAMIGNORE, SPAMIGNOREPATTERNS, DATABYTES, MDA, MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP, MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_READ_TIMEOUT, MYSQL_OPT_WRITE_TIMEOUT, QUEUEDIR, QUEUE_BASE, QUEUE_START, QUEUE_COUNT, and TMPDIR
```

The following list of environment variables which can be modified using envrules are specfic to <b>qmail-remote</b>.

```
CONTROLDIR, SMTPROUTE, SIGNKEY, OUTGOINGIP, DOMAINBINDINGS, AUTH_SMTP, MIN_PENALTY, and MAX_TOLERANCE
```

The following list of environment variables which can be modified using envrules are specfic to <b>qmail-local</b>.

`USE_SYNCDIR, USE_FSYNC, and LOCALDOMAINS`

Do man <b>qmail-smtpd</b>(8), <b>spawn-filter</b>(8) to know the full list of environment variables that can be controlled using envrules.

# Domain Specific Queues

This is actually an extension of [envrules](#envrules), but has its own control file named <b>domainqueue</b> for convenience. The control file <u>/etc/indimail/control/domainqueue</u> is of the form

`pattern:envar1=val,envar2=val,...]`

When the domain for an email being injected into the queue by <b>qmail-smtpd</b> or <b>qmail-inject</b>, matches <u>pattern</u>, the environment variable list gets set. In the example below, emails injected into the queue by <b>qmail-inject</b> go into the queue <u>/var/indimail/queue/slowq</u>

`yahoo.com:QUEUEDIR=/var/indimail/queue/slowq`

This feature becomes useful when setting domain specific delivery rate controls as mentioned in the chapter [Controlling Delivery Rates](#controlling-delivery-rates)

# indimail-mini / qmta Installation

indimail-mta has multiple daemons qscheduler/qmail-start, qmail-send, qmail-lspawn, qmail-rspawn and qmail-clean for processing a queue. The standard indimail-mta installation is meant for servers that can withstand high loads resulting from high inbound/outbound mail traffic. For small servers which have minimal or sporadic traffic, the full indimail-mta installation isn't required or necessary. In such cases you can either install indimail-mini to use QMQP protocol or qmta to use standard delivery mechanisms. indimail-mini comprises of just 10 binaries, whereas qmta comprises of 25 binaries. For most cases, you just require one binary <b>qmail-qmqpc</b> if you use indimail-mini. If you use qmta, for most cases you just require <b>qmta-send</b> and <b>qmail-queue</b> to process and send mails.

## indimail-mini - Using QMQP protocol provided by qmail-qmqpc / qmail-qmqpd

QMQP provides a centralized mail queue within a cluster of hosts. QMQP clients do not require local queue for queueing messages. QMQP is faster than SMTP. You can use QMQP to send mails from your relay servers (or any server) to servers running QMQP service. The QMQP server can deliver mails to your local mailboxes or/and relay mails to the outside world. In short, to use QMQP, you need a client that uses QMQP protocol and a server that provides the QMQP service. <b>qmail-qmqpc</b> which uses [QMQP](https://cr.yp.to/proto/qmqp.html) protocol, is a QMQP client. <b>qmail-qmqpd</b> is a server that you run to provide a central hub offering the QMQP service. <b>qmail-qmqpd</b> can be run under <b>tcpserver</b> under port <u>628</u> on the central hub. If you have qmail-send running on the central hub, you can deliver local as well as relay mails. The central hub can also be a collection of multiple servers to aid performance. You just need to enter the IP addresses of these servers in /etc/indimail/control/qmqpservers on the client(s). The QMQP protocol doesn't require clients to have a local queue. In fact you can use QMQP protocol on a truly diskless client which doesn't even have any remote filesystem mounted. If you run web servers which need to send out emails, you can use QMQP to send emails without impacting performance. This is because you don't have the overhead of running an MTA like sendmail, postfix or qmail/netqmail/notqmail/indimail-mta. If you already have an installation of indimail-mta on your network, you can quickly setup a indimail-mini installation on your web server. The sendmail client provided by indimail-mini is fully compatible with PHP, python and any software that uses sendmail interface to push out emails. On a server, QMQP service is provided by having indimail-mta installed and enabling the qmail-qmqpd service. On the client you just need to have indimail-mini / indimail-mta or qmta installed and configure sendmail/qmail-inject to use qmail-qmqpc. If you install indimail-mini, the installation will configure sendmail/qmail-inject to use qmail-qmqpc. If you have ezmlm, indimail-mta, qmta installed, you need to set QMAILQUEUE to use qmail-qmqpc. You can refer to [Setting Environment Variables](#setting-environment-variables) to set your QMAILQUEUE variable. The process for setting up QMQP service is outlined in the next chapter.

![image](indimail_mini.png)

If you use a source installation, you can copy few binaries manually and run few commands manually, to have a indimail-mini installation. There is also a RPM package indimail-mini which does both installation and setup. An indimail-mini installation comes up with a bare minimum list of programs to enable you to send out mails.

### How do I set up a QMQP service?

You need to have at least one host on your network offering QMQP service to your clients. indimail-mta runs a QMQP service which handles incoming QMQP connections on port 628 using tcpserver. It uses multilog to store log messages under /var/log/svc/qmqpd.628. The QMQP service is actually provided by <b>qmail-qmqpd</b>

If you have installed indimail-mta using the RPM, QMQP service is installed by default. However, you need to enable it.

```
$ sudo /bin/bash
# /bin/rm /service/qmail-qmqpd.628/down
# /usr/bin/svc -u /service/qmail-qmqpd.628
```

If you have installed indimail-mta using the source, you may create the QMQP service using the following command

```
Create the service using svctool

$ sudo /usr/sbin/svctool --qmqp=628 --servicedir=/service \
  --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
  --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --fsync --syncdir --memory=104857600 --min-free=52428800

Send a sighup to svscan to activate the new service immediately
rather than waiting for 5 minutes

$ sudo kill -1 `cat /run/svscan/.svscan.pid`
or
$ sudo svps -h
```

The above command will create a supervised service which runs qmail-qmqpd under tcpserver. In case you are setting up this service to relay mails to outside world, you might want to also specify --dkfilter, --qhpsi, --virus-filter, etc arguments to svctool(8) so that tasks like virus scanning, dk, domainkey signing, etc is done by the QMQP service.

Note: Some of the tasks like virus/spam filtering, dk, dkim signing, etc can be done either by the client (if `QMAILQUEUE=/usr/sbin/qmail-spamfilter`), or can be performed by QMQP service if **QMAILQUEUE** is defined as <b>qmail-qmqpc</b> in the service's variable directory.

A QMQP server shouldn't even have to glance at incoming messages; its only job is to queue them for <b>qmail-send</b>(8). Hence you should allow access to QMQP service only from your authorized clients. You can edit the file /etc/indimail/tcp/tcp.qmqp to grant specific access to clients. Here's how to set up QMQP service to authorized client hosts on your indimail-mta server.

first create /etc/indimail/tcp/tcp.qmqp in tcprules format to allow queueing from the authorized hosts. make sure to deny connections from unauthorized hosts. for example, if queueing is allowed from 1.2.3.\*:

```
1.2.3.:allow
:deny
```

Then create /etc/indimail/tcp/tcp.qmqp.cdb:

```
$ sudo qmailctl cdb
building /etc/indimail/tcp/tcp.qmqp.cdb:                   [  OK  ]
```

You can change /etc/indimail/tcp/tcp.qmqp and run tcprules again at any time.

### Client Setup - How do I install indimail-mini to use qmail-qmqpc

A indimail-mini installation is just like a indimail installation, except that it's much easier to set up:

* You don't need MySQL
* You don't need /var/indimail/alias. A indimail-mini installation doesn't do any local delivery.
* You don't require daemontools, ucspi-tcp. You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail.
* You don't need indimail entries in /etc/group or /etc/passwd. indimail-mini runs with the same privileges as the user sending mail; it doesn't have any of its own files.
* You don't need to start anything from your boot scripts. indimail-mini doesn't have a queue, so it doesn't need a long-running queue manager.

Installation and setup is trivial if you use the RPM package (See the chapter [Installing Indimail using DNF/YUM/APT Repository](#installing-indimail-using-dnfyumapt-repository)).

```
$ sudo dnf install indimail-mini
```

If you are doing a source installation then you need to manually copy few binaries and few shared librareis. Here's what you do need if you want to setup QMQP on the client.

* The following binaries are required in the path (for source installation only).
  sendmail
  qmail-inject
  irmail
  predate
  datemail
  mailsubj
  qmail-showctl
  srsfilter
  qmail-qmqpc
  qmail-direct
  mini-smtpd
* shared libs that the above binaries reference. You can use the ldd command (or otool -L command on OSX) (for source installation only).
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail (for source installation only);
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision. Everything can be shared across hosts except for /etc/indimail/control/idhost.
* Setup QMAILQUEUE environment variable to have qmail-qmqpc called instead of qmail-queue when any client injects mails in the queue (for source installation only).
  ```
  # mkdir /etc/indimail/control/defaultqueue
  # cd /etc/indimail/control/defaultqueue
  # echo /usr/sbin/qmail-qmqpc > QMAILQUEUE
  ```
* All manual pages for the above binaries (not a hard requirement but good for future reference) (for source installation only).

Remember that users won't be able to send mail if all the QMQP servers are down. Most sites have two or three independent QMQP servers.

Note that users can still use all the <b>qmail-inject</b> environment variables to control the appearance of their outgoing messages. This will include environment variables in $HOME/.defaultqueue directory.

If you want to setup a SMTP service, you can setup mini-smtpd service. In case you setup SMTP service, you may want to handle tasks is dkim, virus and spam filtering. You can use QHPSI along with a virus scanner like clamav. You can also choose not to have these tasks done at the client end, but rather have it carried out by the QMQP service. For virus scanning refer to chapter [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi). You can set QMAILQUEUE to qmail-multi, qmail-dkim, etc. However, you must remember to have qmail-qmqpc called at the end in case you change QMAILQUEUE to something other than qmail-qmqpc. If you need to setup mini-smtpd, here can be an option

```
sudo ./svctool --smtp=25 --servicedir=/service --skipsend --no-multi \
    --qmailqueue=/usr/sbin/qmail-qmqpc --qmailsmtpd=/usr/sbin/mini-smtpd \
    --cntrldir=control --localip=0 --maxdaemons=20 --maxperip=5 \
    --memory=104857600 --min-free=52428800 \
    --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net
```

## qmta - Using a minimal standalone qmta-send MTA

<b>qmta-send</b> can work like <b>qmail-send</b> to transfer local and remote mails, but unlike qmail-send, it doesn't require mutiple daemons (<b>todo-proc</b>, <b>qmail-lspawn</b>/<b>qmail-rspawn</b>, <b>qmail-clean</b>) to process the queue. In case you don't have a central host running QMQP (provided by <b>qmail-qmqpd</b>) or you have a small host that sends out insignificant number of emails in a day, then <b>qmta-send</b> is what you would want to setup. <b>qmta-send</b> is well suited for tiny computers like raspberry pi, banana pi and other [Single Board Computers](https://en.wikipedia.org/wiki/Single-board_computer).

<b>qmta-send</b> can process messages queued by clients with differing split directory value. The only required is that <b>qmta-send</b> should be running with a higher split dir value than that of the client. You can use the -<u>s</u> option to run with any split dir value.

### How do I set up a standalone MTA using qmta-send

A qmta installation using qmta-send is just like an indimail-mini installation, except that it has a single queue named <u>qmta</u>. This queue is processed by a single daemon <b>qmta-send</b>. <b>qmta-send</b> is like <b>qmail-send</b> provided by indimail-mta, except that it doesn't require multiple daemons - <b>todo-proc</b>, <b>qmail-lspawn</b>/<b>qmail-rspawn</b>, <b>qmail-clean</b>. Just like indimail-mini,

* You don't require MySQL
* You don't require daemontools, ucspi-tcp. You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail.

Installation and setup is trivial if you use the RPM package (See the chapter [Installing Indimail using DNF/YUM/APT Repository](#installing-indimail-using-dnfyumapt-repository)).

```
Install qmta (you can use yum / dnf / apt-get depending on your distribution)

$ apt-get update && apt-get install qmta
Reading package lists... Done
Building dependency tree
Reading state information... Done
The following additional packages will be installed:
  libqmail libsrs2
The following NEW packages will be installed:
  libqmail libsrs2 qmta
0 upgraded, 3 newly installed, 0 to remove and 0 not upgraded.
Need to get 439 kB of archives.
After this operation, 1605 kB of additional disk space will be used.
Do you want to continue? [Y/n] y
Get:1 http://download.opensuse.org/repositories/home:/mbhangui/Debian_10  libqmail 1.1-1.1+62.1 [118 kB]
Get:2 http://download.opensuse.org/repositories/home:/mbhangui/Debian_10  libsrs2 1.0.18-1.1+10.1 [26.2 kB]
Get:3 http://download.opensuse.org/repositories/home:/mbhangui/Debian_10  qmta 2.13-1.1+103.1 [295 kB]
Fetched 439 kB in 1s (309 kB/s)
Selecting previously unselected package libqmail.
(Reading database ... 12408 files and directories currently installed.)
Preparing to unpack .../libqmail_1.1-1.1+62.1_amd64.deb ...
Unpacking libqmail (1.1-1.1+62.1) ...
Selecting previously unselected package libsrs2.
Preparing to unpack .../libsrs2_1.0.18-1.1+10.1_amd64.deb ...
Unpacking libsrs2 (1.0.18-1.1+10.1) ...
Selecting previously unselected package qmta.
Preparing to unpack .../qmta_2.13-1.1+103.1_amd64.deb ...
Adding IndiMail users/groups
Unpacking qmta (2.13-1.1+103.1) ...
Setting up libsrs2 (1.0.18-1.1+10.1) ...
Setting up libqmail (1.1-1.1+62.1) ...
Setting up qmta (2.13-1.1+103.1) ...
Processing triggers for man-db (2.8.5-2) ...

enable qmta-send service

$ sudo systemctl enable qmta-send
Created symlink /etc/systemd/system/multi-user.target.wants/qmta-send.service → /usr/lib/systemd/system/qmta-send.service.

start qmta-send service

$ sudo systemctl start qmta-send

check processes using the humble ps command

$ ps -ef | grep qmta-send | grep -v grep
qmailq    258421       1  0 21:30 ?        00:00:00 /usr/sbin/qmta-send -d ./Maildir/ splogger qmta-send
qmaill    258422  258421  0 21:30 ?        00:00:00 splogger qmta-send
root      258423  258421  0 21:30 ?        00:00:00 /usr/sbin/MTAlspawn -d ./Maildir/ splogger qmta-send
qmailr    258424  258421  0 21:30 ?        00:00:00 /usr/sbin/MTArspawn -d ./Maildir/ splogger qmta-send

check processes using the mighty systemctl command

$ systemctl status qmta-send
● qmta-send.service - qmta Mail Transport Agent
     Loaded: loaded (/usr/lib/systemd/system/qmta-send.service; enabled; vendor preset: disabled)
     Active: active (running) since Fri 2021-07-16 21:30:35 IST; 57s ago
   Main PID: 258421 (qmta-send)
      Tasks: 4 (limit: 9421)
     Memory: 1.9M
     CGroup: /system.slice/qmta-send.service
             ├─258421 /usr/sbin/qmta-send -d ./Maildir/ splogger qmta-send
             ├─258422 splogger qmta-send
             ├─258423 /usr/sbin/MTAlspawn -d ./Maildir/ splogger qmta-send
             └─258424 /usr/sbin/MTArspawn -d ./Maildir/ splogger qmta-send

Jul 16 21:30:35 argos.indimail.org systemd[1]: Started qmta Mail Transport Agent.
Jul 16 21:30:35 argos.indimail.org qmta-send[258422]: 1626451235.303814 status: local 0/10 remote 0/20
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.566547 new msg 1049084
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.566607 info msg 1049084: bytes 366 from <root@argos.indimail.org> qp 258569 uid 0
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.566665 local: root@argos.indimail.org mbhangui@argos.indimail.org 1049084 366 bytes
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.567691 starting delivery 1: msg 1049084 to local mbhangui@argos.indimail.org
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.567740 status: local 1/10 remote 0/20
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.573755 delivery 1: success: did_1+0+0/
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.573809 status: local 0/10 remote 0/20
Jul 16 21:31:27 argos.indimail.org qmta-send[258422]: 1626451287.573876 end msg 1049084
$ You have mail in /home/mbhangui/Maildir
```

As you can see above that qmta-send is very easy to setup and consumes very little resource (1.9M in this case). In the above case, /usr/sbin/MTAlspawn and /usr/sbin/MTArspawn are in fact qmta-send processes with exactly the same code as qmail-lspawn and qmail-rspawn. The only external process in the above case is the splogger command running with pid 258422. In future versions, I might make splogger too an internal process and eliminate the requirement of a separate binary. My ultimate goal for qmta-send is to subsume qmail-local and qmail-remote too.

Just like for indimail-mini installation, If you want to setup a SMTP service, you can setup mini-smtpd service. In case you setup SMTP service, you may want to handle tasks is dkim, virus and spam filtering. You can use QHPSI along with a virus scanner like clamav. You can also choose not to have these tasks done at the client end, but rather have it carried out by the QMQP service. For virus scanning refer to chapter [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi). You can set QMAILQUEUE to qmail-multi, qmail-dkim, etc. However, you must remember to have qmail-qmqpc called at the end in case you change QMAILQUEUE to something other than qmail-qmqpc. If you need to setup mini-smtpd, here can be an option

```
sudo ./svctool --smtp=25 --servicedir=/service --skipsend --no-multi \
    --qmailqueue=/usr/sbin/qmail-qmqpc --qmailsmtpd=/usr/sbin/mini-smtpd \
    --cntrldir=control --localip=0 --maxdaemons=20 --maxperip=5 \
    --memory=104857600 --min-free=52428800 \
    --rbl=-rzen.spamhaus.org --rbl=-rdnsbl-1.uceprotect.net
```

If you are doing a source installation then you need to manually copy few binaries and few shared libraries. Here's what you do need if you want to setup from a source installation.

* The following binaries are required in the path
  sendmail
  qmail-inject
  irmail
  forward
  predate
  datemail
  mailsubj
  qmail-showctl
  qmaildirmake
  maildir2mbox
  maildirwatch
  srsfilter
  queue-fix
  qmail-qmqpc
  qmail-direct
  qmta-send
  qmail-lspawn
  qmail-local
  qmail-rspawn
  qmail-remote
  qmail-clean
  qmail-tcpok
  qmail-tcpto
  mini-smtpd
* shared libs that the above binaries reference (libsrs2, libqmail). You can use the ldd command (or otool -L command on OSX) (for source installation only)
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail; (for source installation only)
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision.
* Following groups in /etc/group (for source installation only). You can add them by running the command `groupadd group_name`. Here <u>group_name</u> are listed below.
  indimail
  nofiles
  qmail
  qscand
* Following users in /etc/passwd (for source installation only).
  indimail
  alias
  qmailq
  qmailr
  qmails
  qscand

  You can add them by running the following command
  ```
  # useradd -r -g indimail -d /var/indimail indimail
  # useradd -M -g nofiles  -d /var/indimail/alias  -s /sbin/nologin alias
  # useradd -M -g qmail    -d /var/indimail/       -s /sbin/nologin qmailq
  # useradd -M -g qmail    -d /var/indimail/       -s /sbin/nologin qmailr
  # useradd -M -g qmail    -d /var/indimail/       -s /sbin/nologin qmails
  # useradd -M -g qscand   -d /var/indimail/qscanq -G qmail,qscand -s /sbin/nologin qscand
  ```
* Setup QMAILQUEUE environment variable to have qmail-qmqpc called instead of qmail-queue when any client injects mails in the queue (for source installation only).
  ```
  # mkdir -p /etc/indimail/control/defaultqueue
  # cd /etc/indimail/control/defaultqueue
  # echo qmail-queue > QMAILQUEUE
  ```
* Create a single queue for qmta-send (for source installation only).
  ```
  # mkdir -p /var/indimail/queue
  # queue-fix /var/indimail/queue/qmta
  # cd /etc/indimail/control/defaultqueue
  # echo /var/indimail/queue/qmta > QUEUEDIR
  ```
* Run qmta-send
  1. To run as a daemon run `qmta-send -d`. You can put this command in your system rc script.
  2. To run as and when needed run `qmta-send`. You can call this command using cron or a script.
  3. If you have installed the qmta RPM/debian package, this will be already setup for you in systemd. Else you can copy qmta-send.service to systemd unit directory.
  ```
  # copy systemd unit file qmta-send.service
  $ sudo cp qmta-send /usr/lib/systemd/system

  # enable qmta-send service
  $ sudo systemctl enable qmta-send
  Created symlink /etc/systemd/system/multi-user.target.wants/qmta-send.service → /usr/lib/systemd/system/qmta-send.service.

  # start qmta-send service
  $ sudo systemctl start qmta-send

  # check processes using the humble ps command
  $ ps -ef | grep qmta-send | grep -v grep
  qmailq     39977       1  0 22:53 ?        00:00:00 /usr/sbin/qmta-send -d ./Maildir/ splogger qmta-send
  qmaill     39978   39977  0 22:53 ?        00:00:00 splogger qmta-send
  root       39979   39977  0 22:53 ?        00:00:00 /usr/sbin/MTAlspawn -d ./Maildir/ splogger qmta-send
  qmailr     39980   39977  0 22:53 ?        00:00:00 /usr/sbin/MTArspawn -d ./Maildir/ splogger qmta-send

  # check processes using the mighty systemctl command
  $ sysemctl status qmta-send
  ● qmta-send.service - qmta Mail Transport Agent
       Loaded: loaded (/usr/lib/systemd/system/qmta-send.service; enabled; vendor preset: disabled)
       Active: active (running) since Tue 2021-07-06 22:53:32 IST; 7s ago
     Main PID: 39977 (qmta-send)
        Tasks: 4 (limit: 9421)
       Memory: 1.9M
       CGroup: /system.slice/qmta-send.service
               ├─39977 /usr/sbin/qmta-send -d ./Maildir/ splogger qmta-send
               ├─39978 splogger qmta-send
               ├─39979 /usr/sbin/MTAlspawn -d ./Maildir/ splogger qmta-send
               └─39980 /usr/sbin/MTArspawn -d ./Maildir/ splogger qmta-send

  Jul 06 22:53:32 argos.indimail.org systemd[1]: Started qmta Mail Transport Agent.
  Jul 06 22:53:32 argos.indimail.org qmta-send[39978]: 1625592212.411851 status: local 0/10 remote 0/20
  ```
* All manual pages for the above binaries (not a hard requirement but good for future reference).

Note that users can still use all the <b>qmail-inject</b> environment variables to control the appearance of their outgoing messages. Also you can setup environment variables in $HOME/.defaultqueue apart from /etc/indimail/control/defaultqueue

# Using /usr/sbin/alternatives

Sometimes two or more package exist that serve the same purpose. The alternatives system provides a mechanism for selecting an active default application from several valid alternatives. You can use the alternatives system to configure as an alternative MTA for your system. Using alternatives, you don't have to create the links to /usr/bin/sendmail manually as instructed above.

```
$ sudo /usr/sbin/alternatives --install \
    /usr/sbin/sendmail mta /usr/bin/sendmail 120 \
    --slave /usr/share/man/man8/sendmail.8.gz mta-sendmailman \
    /usr/share/man/man8/qmail-inject.8.gz \
    --slave /usr/lib/sendmail mta-sendmail \
    /usr/bin/sendmail
    /usr/sbin/alternatives --set mta /usr/bin/sendmail
```

NOTE: archlinux doesn't have the alternatives command.

# Post Handle Scripts

IndiMail provides a handle post successful operation of few programs. A post execution handle is a program with the same name as that of the calling program but in the directory /usr/libexec/indimail. On successful completion, such programs will execute the handle program and return the status of the called handle program.

In my experience of setting up mail servers in the corporate world, often it is required that users be added to external databases which could be part of some strange enterprise applications. It could be as simple as adding users to your ldap server when creating a mailbox on IndiMail. Sometimes it could be as bad as adding users to ADS (ugh).

IndiMail (release 1.6.9 onwards) provides you a hook, to execute any program after successful completion of the programs, vadddomain, vaddaliasdomain, vdeldomain, vadduser and vdeluser, vrenamedomain, vrenameuser, vmovuser, vpasswd.

A hook can be defined by creating a script or an executable in /usr/libexec/indimail with the name of the program being executed. e.g. if you create a script named vadduser in the directory /usr/libexec/indimail, the script will get executed whenever the program vadduser is used to add a user to indimail. The execution happens only if the program completes successfully. Depending on what you need to do, you can customize the scripts in a jiffy.

The hook script name can be overridden by setting the **POST_HANDLE** environment variable. See the man pages of **vadddomain**(1), **vaddaliasdomain**(1), **vdeldomain**(1), **vadduser**(1), **vmoduser**(1), **vmoveuser**(1), **vdeluser**(1), **vrenamedomain**(1), **vrenameuser**(1), **vpasswd**(1) for more details.

Let me know if you create an interesting script.

Example of using a handle can be demonstrated when adding a user, vuserinfo is also run automatically

```
$ cat /usr/libexec/indimail/vadduser
exec /usr/bin/vuserinfo $1
```

because of the above, this is what happens when you add a user

```
$ sudo /usr/bin/vadduser test05@example.com
New IndiMail password for test05@example.com:
Retype new IndiMail password:
name : test05@example.com
passwd : $1$awb5a5oV$/3rsmlKSu.wzwIFhBzMf7/ (MD5)
uid : 1
gid : 0
-all services available
gecos : test05
dir : /home/mail/T2Zsym/example.com/test05 (missing)
quota : 5242880 [5.00 Mb]
curr quota : 0S,0C
Mail Store IP : 192.168.1.100 (Clustered - local)
Mail Store ID : 1000
Sql Database : 192.168.1.100:indimail:ssh-1.5-
Table Name : indimail
Relay Allowed : NO
Days inact : 0 days 00 Hrs 00 Mins 00 Secs
Added On : ( 127.0.0.1) Sat Apr 24 19:49:06 2010
last auth : Not yet logged in
last IMAP : Not yet logged in
last POP3 : Not yet logged in
PassChange : Not yet Changed
Inact Date : Not yet Inactivated
Activ Date : ( 127.0.0.1) Sat Apr 24 19:49:06 2010
Delivery Time : No Mails Delivered yet / Per Day Limit not configured
```

I personally use post execution handle for adding some mandatory users every time I add a new domain. So this is what my vadddomain handle looks like

```
$ cat /usr/libexec/indimail/vadddomain
/usr/bin/vdominfo $1
/usr/bin/valias -i '&register-spam' register-spam@$1
/usr/bin/valias -i '&register-ham' register-ham@$1
/usr/bin/valias -i '&spam' spam@$1
/usr/bin/valias -i '&ham' ham@$1
/usr/bin/vadduser -e prefilt@$1 xxxxxxxx
/usr/bin/vadduser -e postfilt@$1 xxxxxxxx
/usr/bin/vcfilter -i -t spamFilter -c 3 -k "Yes, spamicity=" -f Spam -b 0 -h 33 prefilt@$1
/bin/ls -dl /var/indimail/domains/$1
/bin/ls -al /var/indimail/domains/$1
exit 0
```

# Authenticated SMTP Mechanisms

IndiMail provides you authenticated SMTP providing **AUTH PLAIN**, **AUTH LOGIN**, **AUTH CRAM-MD5**, **CRAM-SHA1**, **CRAM-SHA224**, **CRAM-SHA256**, **CRAM-SHA384**, **CRAM-SHA512**, **CRAM-RIPEMD**, **DIGEST_MD5**, **SCRAM-SHA-1**, **SCRAM-SHA-256**, **SCRAM-SHA-1-PLUS**, **SCRAM-SHA-256-PLUS** methods.

## LOGIN and PLAIN authentication mechanism

The most common practice to offer authentication is where the passwords are one-way hash and stored in a database. The UNIX <b>passwd</b> program is one such program that encrypts the user supplied password and stores it in the system's database (e.g. /etc/shadow). The <b>passwd</b> uses the <b>crypt</b> library function to encrypt passwords and it is the same function that IndiMail uses. IndiMail has the programs <b>vpasswd</b> and <b>vmoduser</b> that sets the encrypted password for a user in IndiMail's MySQL database. The <b>vadduser</b> program too sets encrypted password using crypt(3). The original clear text passwords are not stored. If this database is stolen, the thief has possession of encrypted passwords which cannot be used for authentication. An exception is when you setup any of the CRAM authentication methods (explained later). Authentication of a password is verified by encrypting the password and matching it with the value stored in the database. But this requires the client to exchange the clear text password with the server. The LOGIN and PLAIN authentication methods require clients to exchange clear text passwords. This can be a concern on an un-encrypted connections. If you encrypt the session using TLS, this concern can be addressed. You can enable <b>SECURE\_AUTH</b> in qmail-smtpd and qmail-remote to disallow LOGIN and PLAIN authentication when using un-encrypted connections. For example, you can do this for the SMTP submission port as below

```
$ sudo sh -c "echo 1 > /service/qmail-smtpd.587/variables/SECURE_AUTH"
```

To set this one-way hash encrypted passwords is simple. For example, use the vpasswd program to set the password for the user <u>manny</u> as <u>supersecret</u>

```
$ sudo vpasswd manny@example.com supersecret
name          : manny@example.com
passwd        : $5$edYmuYDig/cqixyh$0HWa5eEOn2MH2TuLfy51YsRPkcuEFWbRPqGs6m3/uS0 (SHA256)
uid           : 1
gid           : 0
                -all services available
gecos         : manny
dir           : /home/mail/L2P/example.com/manny
quota         : 524288000 [500.00 MiB]
curr quota    : 0S,0C
Mail Store IP : 192.168.2.107 (Clustered - local
Mail Store ID : 100
Sql Database  : 192.168.2.107:indimail:ssh-1.5-:3306:ssl
TCP/IP Port   : 3306
Use SSL       : Yes
SSL Cipher    : TLS_AES_256_GCM_SHA384
Table Name    : indimail
Relay Allowed : NO
Days inact    : 0 days 10 Hrs 18 Mins 43 Secs
Added On      : (127.0.0.1) Tue Jul  2 09:33:55 2019
last  auth    : (127.0.0.1) Sat Aug 27 11:07:24 2022
last  POP3    : Not yet logged in
last  IMAP    : (127.0.0.1) Sat Aug 27 11:07:24 2022
PassChange    : (127.0.0.1) Sat Aug 27 21:26:05 2022
Inact Date    : Not yet Inactivated
Activ Date    : (127.0.0.1) Tue Jul  2 09:33:55 2019
Delivery Time : No Mails Delivered yet / Per Day Limit not configured
```

If you notice the passwd field above, you will see the string `$5$edYmuYDig/cqixyh$0HWa5eEOn2MH2TuLfy51YsRPkcuEFWbRPqGs6m3/uS0`. This is actually an encrypted password with the salt as `edYmuYDig/cqixyh` and the encrypted value as `0HWa5eEOn2MH2TuLfy51YsRPkcuEFWbRPqGs6m3/uS0`. You can use the system crypt(3) function to encrypt the passphrase `supersecret` using `edYmuYDig/cqixyh` as the salt, you will always get the result `$5$edYmuYDig/cqixyh$0HWa5eEOn2MH2TuLfy51YsRPkcuEFWbRPqGs6m3/uS0`. As an example, you can use the `incrypt` program to verify this

```
$ incrypt -S 'edYmuYDig/cqixyh' supersecret
"$5$edYmuYDig/cqixyh$0HWa5eEOn2MH2TuLfy51YsRPkcuEFWbRPqGs6m3/uS0"
```

The other thing to notice in the password field is that it starts with `$5$`. This is a SHA256 hashed password. If it is `$6$`, it would have been a SHA512 hashed password, `$1$` it would have been a MD5 hashed password. See crypt(5) for more details. You can use the -h argument to vpasswd to set a specific hash method (MD5, SHA256 or SHA512). With so much computing power available today, one shouldn't use any hash below SHA256.

In my opinion, using this scheme with TLS enabled connection is the most secure way to setup authentication. Your database is safe and by using TLS, you are safeguarded from [MITM](https://en.wikipedia.org/wiki/Man-in-the-middle_attack) and credentials getting leaked by [sniffing attacks](https://en.wikipedia.org/wiki/Sniffing_attack).

## Challenge Response Authentication Mechanisms

<b>qmail-smtpd</b> and <b>qmail-remote</b> also support various challange response (also known as CRAM) authenticaiton mechanisms. These methods have one side (the server) issue a challenge and the other side (client) respond with an answer. What is exchanged between the client and the server are the challenge and response. Passwords are not exchanged on the network, thus safeguarding you against sniffing and eavesdropping attacks. Though this method makes authentication safe even in a un-encrypted channel, the downside is that the both server and client need to know the clear text passwords. So for the CRAM authentication to work, the server requires the clear text passwords to be stored in the database. The huge security risk of using CRAM is safeguarding your database containing user's passwords. If the database is stolen, the thief walks away with all the passwords. If you are still ok after reading this, the <b>vpasswd</b> program can be used to store passwords in clear text by passing the -e arguments.

```
$ sudo vpasswd -e manny@example.com supersecret
name          : manny@example.com
passwd        : supersecret (DES)
uid           : 1
gid           : 0
                -all services available
gecos         : manny
dir           : /home/mail/L2P/example.com/manny
quota         : 524288000 [500.00 MiB]
curr quota    : 0S,0C
Mail Store IP : 192.168.2.107 (Clustered - local
Mail Store ID : 100
Sql Database  : 192.168.2.107:indimail:ssh-1.5-:3306:ssl
TCP/IP Port   : 3306
Use SSL       : Yes
SSL Cipher    : TLS_AES_256_GCM_SHA384
Table Name    : indimail
Relay Allowed : NO
Days inact    : 0 days 10 Hrs 35 Mins 46 Secs
Added On      : (127.0.0.1) Tue Jul  2 09:33:55 2019
last  auth    : (127.0.0.1) Sat Aug 27 11:07:24 2022
last  POP3    : Not yet logged in
last  IMAP    : (127.0.0.1) Sat Aug 27 11:07:24 2022
PassChange    : (127.0.0.1) Sat Aug 27 21:43:10 2022
Inact Date    : Not yet Inactivated
Activ Date    : (127.0.0.1) Tue Jul  2 09:33:55 2019
Delivery Time : No Mails Delivered yet / Per Day Limit not configured
```

There are many authentication methods in the CRAM family. IndiMail supports [CRAM-MD5](https://en.wikipedia.org/wiki/CRAM-MD5), CRAM-SHA1, CRAM-SHA224, CRAM-SHA256, CRAM-SHA384, CRAM-SHA512, [CRAM-RIPEMD](https://en.wikipedia.org/wiki/RIPEMD), [DIGEST-MD5](https://www.rfc-editor.org/rfc/rfc2831). CRAM-Md5 uses HMAC-MD5 hash based message authentication code. CRAM-SHA1 uses HMAC-SHA1. CRAM-SHA224 uses HMAC-SHA224. See [this](https://en.wikipedia.org/wiki/HMAC) for more details.

Apart from the security issue of having the plain text passwords in the database, LOGIN and PLAIN authentication will not work with clear text passwords in the database. A workaround for this issue is to store encrypted passwords in the database and set the environment variable <b>ENABLE\_CRAM</b> and have the clients supply the encrypted password to the server as the password. But if you enable this and your database with encrypted passwords gets stolen, the attacker can use the encrypted password to successfully authenticate using the CRAM authentication methods.

Please think twice about setting <b>ENABLE\_CRAM</b> environment variable or enabling clear text passwords using the -e option to <b>vpasswd</b> program. I prefer using the LOGIN or PLAIN methods over an encrypted channel and disabling the CRAM authentication methods. Another option is to use the Salted Challenge Response Authentication Mechanisms explained below.

## Salted Challenge Response Authentication Mechanism

From [RFC-5802](https://www.rfc-editor.org/rfc/rfc5802)

<i>The secure authentication mechanism most widely deployed and used by Internet application protocols is the transmission of clear-text passwords over a channel protected by Transport Layer Security (TLS). There are some significant security concerns with that mechanism, which could be addressed by the use of a challenge response authentication mechanism protected by TLS.  Unfortunately, the challenge response mechanisms presently on the standards track all fail to meet requirements necessary for widespread deployment, and have had success only in limited use.</i>

SCRAM provides the following protocol features

The authentication information stored in the authentication database is not sufficient by itself to impersonate the client. The information is salted to prevent a pre-stored dictionary attack if the database is stolen.

* The server does not gain the ability to impersonate the client to other servers (with an exception for server-authorized proxies).
* The mechanism permits the use of a server-authorized proxy without requiring that proxy to have super-user rights with the back-end server.
* Mutual authentication is supported, but only the client is named (i.e., the server has no name).
* When used as a SASL mechanism, SCRAM is capable of transporting authorization identities (see [RFC4422], Section 2) from the client to the server.

The SCRAM method also allows the server to advertise channel binding. Channel Binding allows the server to enforce TLS and safeguard the connection from main-in-the-middle-attack. IndiMail supports the [tls-unique](https://www.rfc-editor.org/rfc/rfc5929.html) channel binding for TLS1.2 and [tls-exporter](https://www.rfc-editor.org/rfc/rfc9266.html) channel binding for TLS1.3 and above. When the server supports channel binding, it advertises 

IndiMail supports SCRAM-SHA-1 and SCRAM-SHA-256 SCRAM authentication methods. qmail-smtpd advertises these methods to an EHLO response. qmail-remote issues the EHLO command to get the capability of the remote server. Both qmail-smtpd and qmail-remote support channel binding too. Hence qmail-smtpd advertises SCRAM-SHA-1-PLUS and SCRAM-SHA-256-PLUS authentication methods. Similarly qmail-remote uses tls-unique or the tls-exporter channel binding when the remote supports SCRAM-SHA\*-PLUS methods. You can use the <b>vpasswd</b> (or <b>vmoduser</b>) program to set passwords enabled for SCRAM authentication. For example, the below password will allow AUTH LOGIN, PLAIN and all SCRAM methods to work.

```
sudo vpasswd -m SCRAM-SHA-256 manny@example.com supersecret
name          : manny@example.com
passwd        : {SCRAM-SHA-256}4096,HWvKDMG1R9hSSt/e,thk3p9gV604V9A2chgmW/ovPgucUtFRhHJgfeWi2pJ4=,IvIc99nIBYdKFqW+ZTz1Ioz5nIxWXJDegzPlkq41tic=,$5$7.Jwtq6QiQaLsguq$RXUJkrI5PUhtqGa4jKGbLPZaLmJiGdpqXhe68qtqyE/ (SCRAM-SHA-256)
uid           : 1
gid           : 0
                -all services available
gecos         : manny
dir           : /home/mail/L2P/example.com/manny
quota         : 524288000 [500.00 MiB]
curr quota    : 0S,0C
Mail Store IP : 192.168.2.107 (Clustered - local
Mail Store ID : 100
Sql Database  : 192.168.2.107:indimail:ssh-1.5-:3306:ssl
TCP/IP Port   : 3306
Use SSL       : Yes
SSL Cipher    : TLS_AES_256_GCM_SHA384
Table Name    : indimail
Relay Allowed : NO
Days inact    : 0 days 10 Hrs 28 Mins 37 Secs
Added On      : (127.0.0.1) Tue Jul  2 09:33:55 2019
last  auth    : (127.0.0.1) Sat Aug 27 11:07:24 2022
last  POP3    : Not yet logged in
last  IMAP    : (127.0.0.1) Sat Aug 27 11:07:24 2022
PassChange    : (127.0.0.1) Sat Aug 27 21:36:01 2022
Inact Date    : Not yet Inactivated
Activ Date    : (127.0.0.1) Tue Jul  2 09:33:55 2019
Delivery Time : No Mails Delivered yet / Per Day Limit not configured
```

If you carefully notice the passwd field, you will see the password starting with {SCRAM-SHA-256}. The iteration count is 4096. This is the iteration used by the algorithm. The salt is `HWvKDMG1R9hSSt/e`, the stored key is `thk3p9gV604V9A2chgmW/ovPgucUtFRhHJgfeWi2pJ4=`, the server key is `IvIc99nIBYdKFqW+ZTz1Ioz5nIxWXJDegzPlkq41tic=` and finally the salted password that will be used for AUTH LOGIN and AUTH PLAIN is `$5$7.Jwtq6QiQaLsguq$RXUJkrI5PUhtqGa4jKGbLPZaLmJiGdpqXhe68qtqyE/`. When using SCRAM, the stored key and the server key is all that the server requires to verify if the client's response to the challenge is correct or not.

To additionally allow CRAM methods, use the -C argument to <b>vpasswd</b> (or <b>vmoduser</b>). Now look carefull at the password field by doing vuserinfo.

```
sudo vpasswd -C -m SCRAM-SHA-256 manny@example.com supersecret
name          : manny@example.com
passwd        : {SCRAM-SHA-256}4096,fIne4ExD9h3vufYZ,k9LD3a/7yk3u7o5tVPTRR4IYHqgd7khpwbKLzvNGnos=,LCuD6/yMc5cjmCMrBAj+es27icwu5eoI5Drqmh4P+mQ=:a8bac1a24c7aa2725630c9ab6813a631b366c79c6bc959bf344d3af8e685a375:supersecret,$5$j59A2PxyHYy5q5OE$wzif22gTmb3SPhbtQN6Z9jzvNB5JA54He8pN.midpu7 (SCRAM-SHA-256)
uid           : 1
gid           : 0
                -all services available
gecos         : manny
dir           : /home/mail/L2P/example.com/manny
quota         : 524288000 [500.00 MiB]
curr quota    : 0S,0C
Mail Store IP : 192.168.2.107 (Clustered - local
Mail Store ID : 100
Sql Database  : 192.168.2.107:indimail:ssh-1.5-:3306:ssl
TCP/IP Port   : 3306
Use SSL       : Yes
SSL Cipher    : TLS_AES_256_GCM_SHA384
Table Name    : indimail
Relay Allowed : NO
Days inact    : 0 days 11 Hrs 30 Mins 59 Secs
Added On      : (127.0.0.1) Tue Jul  2 09:33:55 2019
last  auth    : (127.0.0.1) Sat Aug 27 11:07:24 2022
last  POP3    : Not yet logged in
last  IMAP    : (127.0.0.1) Sat Aug 27 11:07:24 2022
PassChange    : (127.0.0.1) Sat Aug 27 22:38:23 2022
Inact Date    : Not yet Inactivated
Activ Date    : (127.0.0.1) Tue Jul  2 09:33:55 2019
Delivery Time : No Mails Delivered yet / Per Day Limit not configured
```
You will notice two additional fields - the hex salted password `a8bac1a24c7aa2725630c9ab6813a631b366c79c6bc959bf344d3af8e685a375` and the clear text password `supersecret` following it (separated by a colon). This kind of password will allow you to use AUTH PLAIN, LOGIN, all CRAM methods and all SCRAM / SCRAM-PLUS methods. When you set passwords using the -m and -C argument to <b>vadddomain</b>, <b>vadduser</b>, <b>vgroup</b>, <b>vpasswd</b>, <b>vmoduser</b>, CRAM authentication will not require setting of the environment variable <b>ENABLE\_CRAM</b>. But the problem with enabling CRAM auth methods is that attackers can get hold of the clear text passwords for all your users in case your database is stolen. Apart from the clear text password, the hex salted password too allows the attacker to impersonate both client and the server. If you use SCRAM authentication, you can disable all CRAM authentication methods and not use the -<b>C</b> argument to <b>vpasswd</b> (or <b>vmoduser</b>). This will give you the safety of of having all your passwords encrypted in the database.

## Authentication SMTP mechanism in qmail-smtpd

Now that we have discussed the various authentication mechanism, let's talk about how to enable it in qmail-smtpd. If qmail-smptd is called with the arguments <u>hostname</u> <u>checkprogram</u> <u>subprogram</u> <u>subprogram</u>, it will enable various authentication methods and pass authentication credentials to <u>checkprogram</u>, <u>subprogram</u> to validate authentication credentials. For example, the default installation invokes qmail-smtpd as <b>qmail-smtpd</b> <u>/usr/sbin/sys-checkpwd</u> <u>/usr/sbin/vchkpass</u>. The <b>sys-checkpwd</b> <u>checkprogram</u> validates the user credentials in the system database (/etc/shadow, pam, ldap, etc) and the <b>vchkpass</b> program validates user credentials against indimail's MySQL database. If the user is present in either of the two database and supplies the correct password, the authentication succeeds. Apart from using external programs called as command line arguments to qmail-smptd, there is an internal authentication mechanism to validate users using the SCRAM authentication methods. <u>checkprogram</u> are programs that use the [checkpassword](https://cr.yp.to/checkpwd/interface.html) interface designed by [djb](https://cr.yp.to/djb.html). indimail-mta uses a modifed checkpassword interface that supports [dovecot's checkpassword](https://doc.dovecot.org/configuration_manual/authentication/checkpassword/) authentication and exits when the final <u>subprogram</u> is <u>/bin/false</u>. To enable authentication when using dovecot imap, one needs to set the environment variable <b>NATIVE\_PASSWORD</b>.

IndiMail allows you to DISABLE individual authentication mechanism. e.g. the following will disable CRAM-MD5, CRAM-SHA1, CRAM-SHA224, CRAM-SHA256, CRAM-SHA384, CRAM-SHA512, CRAM-RIPEMD, DIGEST-MD5 authentication mechanisms.

```
$ sudo sh
cd /service/qmail-smtpd.587/variables
for i in MD5 SHA1 SHA224 SHA256 SHA384 SHA512
do
  echo 1 > DISABLE_AUTH_CRAM_$i
done
echo 1 > DISABLE_DIGEST_MD5
```

Once you have enabled authentication methods, you can use tools like swak(1), gsasl(1) to test the method. Few examples are given below.

```
# Using plaintext password
$ gsasl --no-cb -d --hostname=argos.indimail.org --x509-ca-file="" -a manny@example.com --password supersecret --mechanism SCRAM-SHA-256 --smtp --connect argos:587

# Using hex salted password
$ gsasl --no-cb -d --hostname=argos.indimail.org --x509-ca-file="" -a manny@example.com --password a8bac1a24c7aa2725630c9ab6813a631b366c79c6bc959bf344d3af8e685a375 --mechanism SCRAM-SHA-256 --smtp --connect argos:587

# Using channel binding
$ gsasl -d --hostname=argos.indimail.org --x509-ca-file="" -a manny@example.com --password supersecret --mechanism SCRAM-SHA-256-PLUS --smtp --connect argos:587

# Using a CRAM method
$ swaks --to testuser@example.com --server localhost -port 587 --a CRAM-SHA1 -au manny@example.com -ap supersecret
```

NOTE: SCRAM authentication has a design flaw. It doesn't have a mechanism to negotiate the SCRAM variant to use, the iteration count to use. The result of this is that one will be restricted to create users wth passwords supporing just one fixed variant and a fixed iteration count. You can use either SCRAM-SHA-1 or SCRAM-SHA-256 and not both. See https://blog.josefsson.org/2021/06/08/whats-wrong-with-scram/. The other way is to break the unique user+domain combination for <u>indimail</u> table in the MySQL database and store passwords for all variants when you add/modify users. That would make password management having gargantuan complications and prone to password mismatch within the different variants. And what do you do if these armchair RFC writers come up with a new SCRAM variant? Ask the users to change passwords to support the new SCRAM variant? And if you want to put yourself in a soup, imagine few users having a different iteration count!!!

## Enabling authentication methods for qmail-remote

qmail-remote supports the same authentication mechanism that qmail-smtpd supports. It requires the user credentials to be present in the file <u>/etc/indimail/control/smtproutes</u> or the environment variable <b>SMTPROUTE</b>. The format of this file can be any one of the below forms. You can read in more detail in the [qmail-remote](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8) man page.

```
domain:relay username password
domain:relay username password envstr
domain:relay:port username password
domain:relay:port username password envstr
domain:relay:port:penalty:max_tolerance username password
domain:relay:port:penalty:max_tolerance username password envstr

# wildcard entries
:relay:port
:relay:port username password
:relay:port username password envstr
:relay:port:penalty:max_tolerance
:relay:port:penalty:max_tolerance username password
:relay:port:penalty:max_tolerance username password envstr
```

As an example, let us use the same password for the user `manny` that we set above

```
(
 echo "From: manny@example.com"
 echo "To: testuser@example.com"
 echo "Subject: Test message"
 echo "Date: $(date --rfc-email)"
 echo
 echo "Test message"
) > /tmp/mail.txt

$ ls -l /tmp/mail.txt
-rw-r--r-- 1 mbhangui mbhangui 123 Oct 31 09:55 /tmp/mail.txt

# use plain text password and authenticate using CRAM-MD5
$ env SMTPROUTE='example.com:127.0.0.1:587 manny@example.com supersecret' AUTH_SMTP="CRAM-MD5" qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1661621762 qp 126027

# use plain text password and authenticate using CRAM-SHA-256 without channel binding
$ env SMTPROUTE='example.com:127.0.0.1:587 manny@example.com supersecret' AUTH_SMTP="SCRAM-SHA-256" qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1661621906 qp 126055

# use plain text password and authenticate using CRAM-SHA-256 with channel binding
$ env SMTPROUTE='example.com:127.0.0.1:587 manny@example.com supersecret' AUTH_SMTP="SCRAM-SHA-256-PLUS" qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1667103629 qp 20543

# use hex salted password and authenticate using CRAM-SHA-256 without channel binding
$ env SALTED_PASSWORD=1 SMTPROUTE='example.com:127.0.0.1:587 manny@example.com a8bac1a24c7aa2725630c9ab6813a631b366c79c6bc959bf344d3af8e685a375' AUTH_SMTP="SCRAM-SHA-256" qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1667103594 qp 20526
```

If you are going to use <u>smtproutes</u> file to store user credentials, have the file owned by <b>qmailr</b> and permissions <b>0600</b>. Instead of using <u>smtproutes</u> you can use the cdb file <u>remote_auth.cdb</u> to store credentials for authenticated smtp. This step involves using the <b>cdb-database</b> program to create <u>remote_auth.cdb</u> from the text file <u>remote_auth</u>.

```
# demonstrating use of remote_auth.cdb for storing credentials for remote authenticated SMTP

$ cd /etc/indimail/control
$ cat > remote_auth <<EOF
=manny@example.com:manny@example.com a8bac1a24c7aa2725630c9ab6813a631b366c79c6bc959bf344d3af8e685a375:
.
EOF
$ sudo cdb-database remote_auth
$ sudo chown qmailr:root remote_auth remote_auth.cdb
$ sudo chmod 600 remote_auth remote_auth.cdb

$ env SALTED_PASSWORD=1 SMTPROUTE='example.com:127.0.0.1:587 /s' AUTH_SMTP="SCRAM-SHA-256" qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1667151965 qp 136468
```

There is much more you can do with <u>remote_auth.cdb</u>. Please look at the [CONTROL FILES](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8#control-files) section in the man page for [qmail-remote](https://github.com/mbhangui/indimail-mta/wiki/qmail-remote.8).

Here is an another example of doing the same thing as above, but using <b>ctrlenv</b> program to set environment variables based on an address. Here we create a cdb file <u>ctrlenvdemo.cdb</u> for fast lookups using the <b>cdb-database</b> program.

```
# demonstrating ctrlenv
$ cd /etc/indimail/control
$ cat > ctrlenvdemo <<EOF
=manny@example.com:SALTED_PASSWORD=1,SMTPROUTE=example.com:127.0.0.1:587 /s,AUTH_SMTP=SCRAM-SHA-256:
.
EOF
$ sudo cdb-database ctrlenvdemo
$ sudo chown qmailr:root ctrlenvdemo ctrlenvdemo.cdb
$ sudo chmod 600 ctrlenvdemo ctrlenvdemo.cdb

$ ctrlenv -f ctrlenvdemo.cdb -a manny@example.com qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1667189684 qp 67360

# You need not use the file ctrlenvdemo.cdb. You can directly use the file ctrlenvdemo if you have very few records.
$ ctrlenv -f ctrlenvdemo -a manny@example.com qmail-remote example.com manny@example.com "" 123 testuser@example.com </tmp/mail.txt
rFrom: <manny@example.com> RCPT: <testuser@example.com> K127.0.0.1 accepted message - Protocol SMTP.
Remote host said: 250 ok 1667190974 qp 91502
```

# Relay Mechanism in IndiMail

A SMTP server is responsible for accepting mails from a sender and processing it for delivery to one or more recipients. In most situations, for domains which are under your administrative control (native addresses), the SMTP server should accept mails without authentication. However, when a mail is submitted for delivery to domains which are not under your administrative control, you should accept mails only after it satisfies security considerations like having the sender authenticate itself. This is to prevent abuse of external domains using your SMTP server. A SMTP server which accepts mails for external domains without any authentication is called an open relay. The act of accepting mails for external domains for delivery is called relaying.

The default configuration of IndiMail configures the SMTP as a closed system. Hence to be able to send mails to external domains, you need to setup mechanisms for relaying.

There are many methods. Choose any of the below after studying them. I prefer 3 or 4 for security reasons.

1. Have Sender's IP addresses in tcp.smtp file
2. Use control file relayclients for IP addresses of clients allowed to relay mail through this host.
3. Configure IndiMail to use MySQL relay table (good security). This is implemented as POP3/IMAP before SMTP
4. Use authenticated SMTP (good security)
5. For allowing relay to specific domains use control file relaydomains
6. For allowing specific users (native addresses) use control file relaymailfrom

NOTE: you should use 1 & 2 only if if the host having the sender's IP is under your control and you have good security policies for the host (however ‘what is a good security’ can be very subjective)

## Using tcp.smtp

Your startup script for the qmail smtp server must use the tcpserver -x file option, similar to this startup line.

```
env - PATH="/usr/bin" tcpserver -H -R -x /etc/indimail/tcp/tcp.smtp.cdb \
  -c 20 -u 555 -g 555 0 smtp /usr/sbin/qmail-smtpd 2>&1
```

IndiMail uses -x option to tcpserver and hence you need not bother about the above line. You however need to edit /etc/indimail/tcp.smtp and put in lines for all static IP's that you will always want to relay access to.

```
127.0.0.:allow,RELAYCLIENT=””
10.1.1.:allow,RELAYCLIENT=””
```

The above lines will cause **RELAYCLIENT** environment variable to be set for localhost and all machines on the 10.1.1 class and hence allow to relay through. Remember that any user on hosts on 10.1.1 class will be able to relay mails. You many not want this. The line having 127.0.0. will allow any client on the IndiMail host to use SMTP and relay mails.

If you add any IP to tcp.smtp, you have to rebuild a cdb database tcp.smtp.cdb. You can run the following command

`$ sudo /usr/bin/qmailctl cdb`

NOTE: Remember that you are exposed to unrestricted relaying from any of the IP addresses listed in tcp.smtp

## Using control file relayclients

IP addresses of clients allowed to relay mail through this host. Each address should be followed by a colon and an (optional) string that should be appended to each incoming recipient address, just as with the RELAYCLIENT environment variable. Nearly always, the optional string should be null. The filename can be overriden by the environment variable RELAYCLIENTS.
Addresses in relayclients may be wildcarded (2nd line in the example below):

```
192.168.0.1:
192.168.1.:
```

## Using MySQL relay table

Run the command /usr/bin/clearopensmtp in the cron every 30 Minutes

By default every time, if anyone uses IndiMail's POP3 or IMAP service and authenticates, the following happens:

1. On successful authentication, IMAP/POP3 daemon inserts entry into relay table, inserting email, IP address and timestamp
2. If **CHECKRELAY** environment variable is enabled, SMTP checks the relay table for a entry within minutes specified by the RELAY\_CLEAR\_MINUTES environment variable. If the entry is there, **RELAYCLIENT** environment variable is set, which allows relaying. At this point, the SMTP server will allow that IP to relay for 60 Mins (default)

clearopensmtp will clear all IP which have not authenticated in the past RELAY\_CLEAR\_MINUTES. clearopensmtp should be enabled in cron to run every 30 minutes.

## Set up Authenticated SMTP

Whenever a user successfully authenticates through SMTP, the **RELAYCLIENT** environment variable gets set. <b>qmail-smtpd</b> uses the **RELAYCLIENT** environment variable to allow relaying. Enabling authenticated smtp enables relaying when the user successfully authenticats.

Most of the email clients like thunderbird, evolution, outlook, outlook express have options to use authenticated SMTP.
For a tutorial on authenticated SMTP, you can refer to this [tutorial](http://indimail.blogspot.com/2010/03/authenticated-smtp-tutorial.html "Authenticated SMTP tutorial")


## Using control file relaydomains

Host and domain names allowed to relay mail through this host. Each address should be followed by a colon and an (optional) string that should be appended to each incoming recipient address, just as with the **RELAYCLIENT** environment variable. Nearly always, the optional string should be null. Addresses in relaydomains may be wildcarded:

```
heaven.af.mil:
.heaven.af.mil:
```

## Using control file relaymailfrom

envelope senders (MAIL FROM) listed in this file will be allowed to relay independently of the **RELAYCLIENT** environment variable. Entries in relaymailfrom can be E-Mail addresses, or just the domain (with the @ sign).

Unlike relaydomains native addresses should be entered. A line in relaymailfrom may be of the form @host, meaning every address at host. relaymailfrom can also be in cdb format. If relaymailfrom.cdb is present, it will be searched first.
Examples:

```
joeblow@domain1.com
@domain2.com
```

If you use the control file /etc/indimail/control/relaymailfrom, you should really know what you are doing. Any mail from having a domain component of the address matching any domain in this file, relaying will be allowed without any authentication. You can most probably use this only if you have a closed SMTP server to which access from outside is not possible.

# CHECKRECIPIENT - Check Recipients during SMTP

IndiMail has a feature called **CHECKRECIPIENT** which allows indimail to check at SMTP, if the recipient to whom the mail is being addressed exists. If an address doesn't exist, it is always better to reject such users at SMTP rather than later during the actual delivery to the mailbox. Due to spam, in most of the cases, the Return Path will be forged or undeliverable. Hence you will be left with a condition where plenty of bounces will be left on your system, impacting the performance of your messaging system.

**CHECKRECIPIENT** can also be used to reject mails for inactive users, overquota users and users who do not have the privilege to receive mails. **CHECKRECIPIENT** can be enabled by setting the environment variable **CHECKRECIPIENT** to one of the following values

1. Reject the user if not present in IndiMail's MySQL database
2. Reject the user if not present in IndiMail's MySQL database and not found through <u>recipients</u> extension
3. Reject user if not found through <u>recipients</u> extension

The recipients extension has been taken from [s/qmail](https://www.fehcom.de/sqmail/sqmail.html), an excellent modern replacement for qmail.

You can selectively turn on **CHECKRECIPIENT** for selective domains by including those domains (prefixing the domain with '@' sign) in the control file <u>/etc/indimail/control/chkrcptdomains</u>.

If the environment variable MAX\_RCPT\_ERRCOUNT is set <b>qmail-smtpd</b> will reject an email if in a SMTP session, the number of such recipients who do not exist, exceed MAX\_RCPT\_ERRCOUNT.

To make harvesting of email addresses difficult, **CHECKRECIPIENT** causes the RCPT TO command to be delayed by 5 seconds for every non-existent recipient.

If you do not have large number of users

```
$ sudo /bin/bash
# echo 1 > /service/qmail-smtpd.25/variables/CHECKRECIPIENT
# svc -r /service/qmail-smtpd.25
# exit
$
```

indimail-mta allows you many ways to create addresses that will be served by the MTA. In many cases addresses that you will configure which will not be present in the MySQL database. Such addresses can be aliases, autoresponders, forwards, addresses using .qmail files, local users in /etc/passwd. The recipients extension allows you to create rules or databases to accept such addresses and reject all other non-existent addresses. The recipient extension uses the cotrol file <u>/etc/indimail/control/recipients</u>. This control file contains list of external resources providing acceptable, full-qualified envelope addresses ('RCPT TO: <recip@domain>') to be used for recipient verification during the SMTP session. The external sources can be either <b>fastforward</b> compliant cdbs including the envelope addresses, where the path to a cdb has to be referenced relative to <u>/etc/indimail directory</u> - or - <b>checkpassword</b> compatible Plugable Authentication Modules (PAM), receiving the envelope address on FD 3 as 'recip@domain\0\0\0' and returning '0' in case of success and '1' in case of failure. The use of a PAM is indicated with a delimiting '|' and it will be called with up to five additional parameters; while a cdb follows a ':', which can be omitted. The list of external sources is consulted line-by-line for each recipient envelope address until the first positive answer, or a final negative response is encountered. Which external source to be queried, depends on the domain part of the recipient envelope address specified on the left side of the <u>recipients</u> file, while the external resource is provided right from the delimiter.

The address's domain part is evaluated in lower-case.

	* An entry '@example.com' serves as exact match for the domain address
	* An entry 'example.com' is used to match 'example.com' and in addition all subdomain addresses '*.example.com'; provided if present in control/rcpthosts.
	* The entry '*' will match all domains for the respective lookup.
	* Reversely, domains flagged as '!domain.com' are not queried and all recipients for this domain are accepted.
	* Additionally, a '!*' can be used as wildcard for all domains not encountered before in recipients (passthrough).

The <u>control/recipients</u> file is always constructed like `domain:cdb`, `domain|pam`, or simply `cdb`. Few examples are given below

```
!nocheck.com
@mydomain.com:users/recipients.cdb
example.com|ldap_pam ldapserver host port DN passwd
*:control/fastforward.cdb
*|ldap_pam otherserver
!*
```

Here is how the above will work

	* The first line allows all addresses for nocheck.com to be accepted without any query. 
	* The second example will query all address for mydomain.com to be searched in users/recipients.cdb.
	* The third example, ldap\_pam will be executed with the arguments following it and the email address written to descriptor 3. If ldap_pam exits 0, the address will be assumed to exist.
	* In the fourth example, all addresses that do not match the first 3 lines will be searched in control/fastforward.cdb.
	* In the fifth example, all addresses that don't match the first 4 lines, ldap_pam will be executed and the address sent on descriptor 3.
	* The last line allows all non-matching addresses to passthrough and get accepted.

A simple scheme would be to have your local domain as the line `@localdomain:users/recipients.cdb` in <u>control/recipients</u>. Then add any local users to <u>users/recipients</u> and rebuild the <u>users/recipients.cdb</u> database

```
qmail-cdb -r
```

You can use <b>test-recipients</b> to test your addresses in MySQL or through the <b>recipients</b> extension.

```
# test if manny@example.com is present in MySQL database
# This will query the inlookup daemon which requires user
# to have qmaild privileges. You can also use indimail user
# provided the FIFO gets create with 0666 file mode

$ sudo bash
# setuidgid -g qmail qmaild test-recipients -s testuser01@example.com
test-recipients: user found in MySQL
# setuidgid -g indimail indimail env FIFO_MODE=0666 test-recipients -s testuser01@example.com
test-recipients: user found in MySQL

# test if manny@nocheck.com is allowed by the recipients extension
$ test-recipient -r manny@nocheck.com
test-recipient: user found in cdb
```

Once can use the script <b>make-recipients</b> to create a database for <b>recipients</b> extension.

# SMTP Access List

One of the feature that IndiMail adds to <b>qmail-smtpd</b> is accesslist between senders and recipients. Accesslist can be enabled by creating a control file /etc/indimail/control/accesslist. A line in accesslist is of the form

`type:sender:recipient`

where *type* is either the word 'from' or 'rcpt'. *sender* and *recipient* can be the actual *sender*, *recipient*, a wildcard or a regular expression (uses regex(3))

The accesslist happens during SMTP session and mails which get restricted get rejected with permanent 5xx code.

To give some examples

```
rcpt:ceo@indimail.org:country_distribution_list@indimail.org
rcpt:md@indimail.org:country_distribution_list@indimail.org
from:recruiter@gmail.com:hr_manager@indimail.org
```

* The above accesslist implies that only the users with email `ceo@indimail.org` and `md@indimail.org` can send a mail to the email `country_distribution_list@indimail.org`
* The 3rd line implies that all outside mails from the sender `recruiter@gmail.com` will be rejected at SMTP unless the recipient is `hr_manager@indimail.org`

IndiMail also provides a program called uacl to test this accesslist. uacl is useful especially when you use wildcards or regular expressions.

An extreme example where you want to restrict the communication between two domains only

```
$ cat /etc/indimail/control/accesslist
rcpt:*example.com:*@example1.com
from:*@example1.com:*@example.com


$ uacl test@example.com test@example1.com
rule no 1: rcpt:*example.com:*@example1.com
matched recipient [test@example1.com] with [*@example1.com]
matched sender [test@example.com] with [*example.com] --&gt; access allowed
$
$ uacl test@indimail.org test@example1.com
rule no 1: rcpt:*example.com:*@example1.com
matched recipient [test@example1.com] with [*@example1.com]
sender not matched [test@indimail.org] --&gt; access denied
$
$ uacl test@example1.com test@example.com
rule no 2: from:*@example1.com:*@example.com
matched sender [test@example1.com] with [*@example1.com]
matched recipient [test@example.com] with [*@example.com] --&gt; access allowed
$
$ uacl test@example1.com manvendra@indimail.org
rule no 2: from:*@example1.com:*@example.com
matched sender [test@example1.com] with [*@example1.com]
recipient not matched [manvendra@indimail.org] --&gt; access denied
$
```

# IndiMail Control Files Formats

A little known feature of IndiMail allows some of your control files to be in plain text, cdb or in MySQL. These control files include authdomains, badhelo, badext, badmailfrom, badrcptto, blackholedsender, blackholedrcpt, chkrcptdomains, goodrcptto, relaymailfrom and spamignore. If you have quite a large number of entries in any of the above control files, you can expect a significant performance gains by having these control files in cdb or MySQL.

The mechanism is quite simple. For example, if you have the control file badmailfrom, <b>qmail-smtpd</b> will use badmailfrom. If you have the file badmailfrom.cdb, <b>qmail-smtpd</b> will first do cdb lookup in badmailfrom.cdb. To create badmailfrom.cdb, you need to run the command.

`$ sudo /usr/bin/qmail-cdb badmailfrom`

You can also have your entries in a MySQL table. Let say you have a MySQL server on the server localhost, a database named 'indimail' with user 'indimail' having password 'ssh-1.5-'. To enable the control file in MySQL you need to create the control file with a .sql extension. The following enables the badmailfrom in MySQL

```
# echo "localhost:indimail:ssh-1.5-:indimail:badmailfrom" > badmailfrom.sql
```

Once you have created a file badmailfrom.sql, <b>qmail-smtpd</b> will connect to the MySQL server on localhost and look for entry in the column 'email' in the table badmailfrom. If this table does not exist, <b>qmail-smtpd</b> will create an empty table using the following SQL create statement

```
create table badmailfrom (email char(64) NOT NULL, \
  timestamp timestamp NOT NULL, primary key (email), \
  index timestamp (timestamp))
```

You can use the MySQL client to insert entries. e.g.

```
MySQL> insert into badmailfrom (email) \
MYSQL> values ('testuser@example.com');
```

If you have all the 3 versions of control files existing, IndiMail will first do a cdb lookup, followed by MySQL lookup and finally look into the plain text control file.

Version 1.7.4 of indiMail and above comes with a utility named **qmail-sql** which allows you to create the MySQL table and also insert values from command line or convert an existing plain text version to MySQL. The below command will create a file <u>badmailfrom.sql</u> in <u>/etc/indimail/control</u> directory and table named <b>bmf</b> in MySQL

```
qmail-sql -s localhost -u indimail -p ssh-1.5- -d indimail -t bmf badmailfrom
```

Support for control files in MySQL support gets enabled by configuring the control file <u>mysql_lib</u> in <u>/etc/indimail/control</u> directory. This file should contain the full path to the MySQL shared library. e.g. `/usr/lib64/mysql/libmysqlclient.so.21.2.30`. The command `svctool --fixsharedlibs` updates this file. In case you are not using indimail-virtualdomains or do not have MySQL server installed or do not want MySQL to be installed, you can choose to not have the <u>mysql_lib</u> control file.

# Inlookup database connection pooling service

IndiMail uses MySQL for storing information of virtual domain users. The table 'indimail' stores important user information like password, access permissions, quota and the mailbox path. Most of user related queries have to lookup the 'indimail' table in MySQL.

Rather than making individual connections to MySQL for extracting information from the 'indimail' table, IndiMail programs use the service of the inlookup(8) server. Programs use an API function inquery() to request service. InLookup is a connection pooling server to serve requests for inquery() function. It is implemented over two fifos. One fixed fifo for reading the query and reading the path of a randomly generated fifo. The randomly generated fifo is used for writing the result of the query back. inlookup(8) creates a read FIFO determined by the environment variable **INFIFO**. If **INFIFO** is not defined, the default FIFO used is /var/indimail/inquery/infifo. inlookup(8) then goes into an infinite loop reading this FIFO. If **INFIFO** is not an absolute path, inlookup(8) uses environment variable FIFODIR to look for fifo named by **INFIFO** variable. Inlookup(8) can be configured by setting environment variables in /service/inlookup.info/variables

* inlookup helps in optimizing connection to MySQL(1), by keeping the connections persistent.
* It also maintains the query result in a double link list.
* It uses binary tree algorithm to search the cache before actually sending the query to the database.
* IndiMail clients send requests for MySQL(1) queries to inlookup through the function inquery() using a fifo.
* The inquery() API uses the InLookup service only if the environment variable QUERY_CACHE is set. If this environment variable is not set, the inquery() function makes a direct connecton to MySQL.
* Clients which are currently using inquery are <b>qmail-smtpd</b>(1), proxyimap(8), proxypop3(8), vchkpass(8) and authindi(8).
* inlookup(8) service is one of the reasons why IndiMail is able to serve million+ users using commodity hardware.

The program inquerytest simulates all the queries which inlookup supports and can be used as a test/diagnostic tool for submitting queries to inlookup. e.g

`sudo inquerytest -q 3 -i "" user@example.com`

# Setting limits for your domain

IndiMail comes with a program vlimit(1), which allows you to set global limits for your domain. Before using vlimit, you need to enable domain limits for a domain using vmoddomain(1).

`$ vmoddomain -l 1 example.com`

Once you have done the above, you can start using vlimit for the domain example.com

```
$ vlimit -s example.com
Domain Expiry Date : Never Expires
Password Expiry Date : Never Expires
Max Domain Quota : -1
Max Domain Messages : -1
Default User Quota : -1
Default User Messages: -1
Max Pop Accounts : -1
Max Aliases : -1
Max Forwards : -1
Max Autoresponders : -1
Max Mailinglists : -1
GID Flags:
Flags for non postmaster accounts:
pop account : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
alias : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
forward : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
autoresponder : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist users : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist moderators: ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
domain quota : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
default quota : ALLOW_CREATE ALLOW_MODIFY
```

Using vlimit you can set various limits or defaults for a domain. One of my favourite use of vlimit is setting default quota for users created in a domain. The default quota compiled in IndiMail is 5Mb which is not good enough for today's users. So if you want to have a default quota of 50 Mb for your users when you add them using the **vadduser**(1) command -

```
$ vlimit -q 52428800 example.com
$ vlimit -s example.com
Domain Expiry Date : Never Expires
Password Expiry Date : Never Expires
Max Domain Quota : -1
Max Domain Messages : -1
Default User Quota : 52428800
Default User Messages: -1
Max Pop Accounts : -1
Max Aliases : -1
Max Forwards : -1
Max Autoresponders : -1
Max Mailinglists : -1
GID Flags:
Flags for non postmaster accounts:
pop account : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
alias : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
forward : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
autoresponder : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist users : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist moderators: ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
domain quota : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
default quota : ALLOW_CREATE ALLOW_MODIFY
```

You can also implement domain level restrictions. To disable POP3 for all users in example.com

```
$ vlimit -g p example.com
$ vlimit -s example.com
Domain Expiry Date : Never Expires
Password Expiry Date : Never Expires
Max Domain Quota : -1
Max Domain Messages : -1
Default User Quota : 52428800
Default User Messages: -1
Max Pop Accounts : -1
Max Aliases : -1
Max Forwards : -1
Max Autoresponders : -1
Max Mailinglists : -1
GID Flags:
NO_POP
Flags for non postmaster accounts:
pop account : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
alias : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
forward : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
autoresponder : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist users : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
mailinglist moderators: ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
domain quota : ALLOW_CREATE ALLOW_MODIFY ALLOW_DELETE
default quota : ALLOW_CREATE ALLOW_MODIFY
```

# SPAM and Virus Filtering

IndiMail has multiple methods to insert your script anywhere before the queue, after the queue, before local delivery, before remote deliver or call a script to do local or remote delivery. Refer to the chapter [Writing Filters for IndiMail](#writing-filters-for-indimail) for more details.

# SPAM Control using bogofilter

If you have installed indimail-spamfilter package, you will have [bogofilter](https://bogofilter.sourceforge.io/ "bogofilter") providing a bayesian spam filter.

bogofilter requires training to work. The next section tells you how to carry out this training. You can also have a pre-trained database installed by installing the **bogofilter-wordlist** package, but it is possible that this may not work well in your setup.

On of the easiest method to enable bogofilter is to set few environment variable for indimail's [<b>qmail-spamfilter</b>(8)](https://github.com/mbhangui/indimail-mta/wiki/qmail-spamfilter.8) the frontend for <b>qmail-queue</b>(8) program. e.g. to enable spam filter on the incoming SMTP on port 25:

```
$ sudo /bin/bash

Move QMAILQUEUE to SPAMQUEUE only if QMAILQUEUE doesn't have
/usr/sbin/qmail-spamfilter

cd /service/qmail-smtpd.25/variables
# mv QMAILQUEUE SPAMQUEUE
# echo /usr/sbin/qmail-spamfilter > QMAILQUEUE
# echo "/usr/bin/bogofilter -p -d /etc/indimail" > SPAMFILTER
# echo "0" > SPAMEXITCODE
# echo "0" > REJECTSPAM
# echo "1" > MAKESEEKABLE
exit
$

The below command will create an indimail filter that will automatically move
during delivery, mails marked as spam by bogofilter 
$ sudo vcfilter -i -t spamFilter -c 3 -k "Yes, spamicity=" -f Spam -b 0 -h 33 prefilt@$1
```

Now <b>qmail-spamfilter</b>(8) will pass every mail through bogofilter before it gets passed to <b>qmail-queue</b>(8). You can refer to chapter [IndiMail Queue Mechanism](#indimail-queue-mechanism) and look at the picture to understand how it works. bogofilter(1) will add X-Bogosity in each and every mail. A spam mail will have the value `Yes` along with a probabality number (e.g. 0.999616 below). You can configure bogofilter in /etc/indimail/bogofilter.cf. The SMTP logs will also have lines having this X-Bogosity field. A detailed mechanism is depicted pictorially in the chapter [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi).

```
X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The method describe above is a global SPAM filter. It will happen for all users, unless you use something like **envrules** to unset **SPAMFILTER** environment variable. You can use **envrules** to set **SPAMFILTER** for few specific email addresses. You can refer to the chapter [Envrules](#envrules) for more details.

There is another way you can do spam filtering - during local delivery (you could do for remote delivery, but what would be the point?). IndiMail allows you to call an external program during local/remote delivery by settting **QMAILLOCAL** / **QMAILREMOTE** environment variable. You could use any method to call bogofilter (either directly in *filterargs* control file, or your own script). You can see a Pictorial representation of how this happens. ![LocalFilter](indimail_spamfilter_local.png)

You can also use <b>vcfilter</b>(1) to set up a filter that will place your spam emails in a designated folder for SPAM. Refer to the chapter [Writing Filters for IndiMail](#writing-filters-for-indiMail) for more details.

# Training bogofilter

Training bogofilter requires creating a bogofilter databases with corpus of HAM and SPAM emails with SPAM comprising roughly 30% of the total. A good way to start is download an initial corpus from the spamassasin database. Do not run the below curl command with root privileges.

```
$ mkdir /home/manny/bogofilter
$ cd /home/manny/bogofilter
$ curl -s https://raw.githubusercontent.com/mbhangui/indimail-virtualdomains/master/bogofilter-wordlist-x/download.sh | sh
$ curl -s https://raw.githubusercontent.com/mbhangui/indimail-virtualdomains/master/bogofilter-wordlist-x/training.sh | sh
$ sudo cp wordlist.db /etc/indimail
$ sudo chmod 644 /etc/indimail/wordlist.db
$ sudo chown indimail:indimail /etc/indimail/wordlist.db
```

The above steps will download the spamasassin corpus in the directory /home/manny/bogofilter/training and also create a bogofilter spam database wordlist.db trained from spamassasin corpus at http://spamassassin.apache.org/old/publiccorpus. The `vcfilter` command above creates a rule to automatically move emails marked as spam to Spam subfolder in your Maildir.

You can keep on updating the subfolders easy\_ham\_2 and spam\_2 subfolders in /home/manny/bogofilter/training directory and run the training.sh script. One possible method could be to have a script that uses the find command to copy ham emails from your Maildir subfolders, that you know for sure have non-spam emails, to the easy\_ham\_2 subfolder in the training directory. You can have folder named `Spam` in your Maildir where you move spam emails using your mail client. This will be spam emails not caught by bogofilter. You can have the same script copy spam mails from `Spam` folder to spam\_2 subfolder in the training directory.

# SPAM Control using badip control file

IndiMail has many methods to help deal with spam. For detecting spam, IndiMail uses bogofilter a fast bayesian spam filter. IndiMail's <b>qmail-smtpd</b> which provides SMTP protocol is neatly integrated with bogofilter. When bogofilter detects spam, <b>qmail-smtpd</b> prints the X-Bogosity header as part of SMTP transaction log

```
$ grep "X-Bogosity, Yes" /var/log/svc/smtpd.25/current
@400000004bc8183f01fcbc54 qmail-smtpd: pid 16158 from ::ffff:88.191.35.203 HELO X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The value "Yes" in X-Bogosity indicates spam. You can tell <b>qmail-smtpd</b> to reject such mails at SMTP just by doing

```
# echo 1 > /service/qmail-smtpd.25/variables/REJECTSPAM
# svc -r /service/qmail-smtpd.25
```

SMTP clients which tries to send a spam mail will get the following error at the end of the SMTP transaction
554 SPAM or junk mail threshold exceeded (#5.7.1)
The mail will get bounced. In some cases you would want to issue temporary error to such clients. In the above SMTP transaction log, the IP address of the client was 88.191.35.203. To put such client's into IndiMail's SPAM blacklist, you just need to put the IP address in the control file /etc/indimail/control/badip

`# echo 88.191.35.203 >> /etc/indimail/control/badip`

For turning on the BADIP functionality, you need to set the BADIPCHECK or the BADIP environment variable. i.e.

```
# echo badip > /service/qmail-smtpd.25/variables/BADIP
# svc -r /service/qmail-smtpd.25
```

Clients whose IP match an entry in badip will be greeted as below

```
421 indimail.org sorry, your IP (::ffff:88.191.35.203) is temporarily denied (#4.7.1)
```

Also the client will not be able to carry out any SMTP transactions like ehlo, MAIL FROM, RCPT TO, etc. A large ISP can run the following command every day once in cron

```
grep "X-Bogosity, Yes" /var/log/svc/qmail.smtpd.25/current > /etc/indimail/control/badip
```

If your badip files becomes very large, you can also take advantage of IndiMail's ability to use cdb (or you could use MySQL too)

`$ sudo /usr/bin/qmail-cdb badip`

# Virus Scanning using QHPSI

A large fraction of today’s emails is infected by a virus or a worm. It is necessary to recognize those malicious emails as soon as possible already in the DATA phase of the SMTP conversation and to reject them.

When you use IndiMail, it is ultimately <b>qmail-queue</b> which is responsible for queueing your messages. <b>qmail-queue</b> stores the message component of queued mails (captured duing DATA phase of the SMTP conversation) under the mess subdirectory.

Files under the mess subdirectory are named after their i-node number. Let us look at a typical log sequence for a message received on the local system.

```
@400000004b9da2f03b424bb4 new msg 660188
@400000004b9da2f03b426324 info msg 660188: bytes 2794 from fogcreek_xxx@response.whatcounts.com qp 3223 uid 555
@400000004b9da2f03b42c0e4 starting delivery 6: msg 660188 to local mailstore@indimail.org
@400000004b9da2f03b42dc3c status: local 1/10 remote 0/20
@400000004b9da2f106a1e234 delivery 6: success: did_1+0+0/
@400000004b9da2f1091e676c status: local 0/10 remote 0/20
@400000004b9da2f1091fa3d4 end msg 660188
```

The above lines indicates that indimail-mta has received a new message, and its queue ID is 660188. What this means is that is <b>qmail-queue</b> has created a file named /var/indimail/queue/mess/NN/660188. The i-node number of the file is 660188. This is the queue file that contains the message. The queue ID is guaranteed to be unique as long as the message remains in the queue (you can't have two files with the same i-node in a filesystem).

To perform virus scanning, it would be trivial to do virus scanning on the mess file above in <b>qmail-queue</b> itself. That is exactly what IndiMail does by using a feature called Qmail High Performance Virus Scanner (**QHPSI**). **QHPSI** was conceptualized by Erwin Hoffman. You can read here for more details.

IndiMail takes **QHPSI** forward by adding the ability to add plugins. The **QHPSI** extension for <b>qmail-queue</b> allows to call an arbitary virus scanner directly, scanning the incoming data-stream on STDIN. Alternatively, it allows plugins to be loaded from the /usr/lib/indimail/plugins directory. This directory can be changed by defining PLUGINDIR environment variable. **QHPSI** can be advised to pass multiple arguments to the virus scanner for customization. To run external scanner or load scanner plugins, <b>qmail-queue</b> calls <b>qhpsi</b>, a program setuid to qscand. By default, <b>qhpsi</b> looks for the symbol virusscan to invoke the scanner. The symbol can be changed by setting the environment variable QUEUE_PLUGIN to the desired symbol.

Today’s virus scanner -- in particluar Clam AV -- work in resource efficient client/server mode (clamd/clamdscan) and include the feature to detect virii/worms in the base64 encoded data stream. Thus, there is no necessity to call additional programs (like reformime or ripmime) except for the virus scanner itself.

You can see how the scanning works ![Pictorially](indimail_spamfilter_global.png)

To enable virus scanning in IndiMail during the SMTP data phase, you can implement either of the two methods below

## Using tcprules

Define QHPSI in tcp.smtp and rebuild tcp.smtp.cdb using tcprules.

`:allow,QHPSI=’/usr/bin/clamdscan %s --quiet --no-summary’`

## Using envdir for SMTP service under supervise(8)

Define QHPSI in SMTP service's variable directory

```
$ sudo /bin/bash
# echo "/usr/bin/clamdscan %s --quiet --no-summary" > /service/qmail-smtpd.25/variables/QHPSI
```

If you have installed IndiMail using RPM available here or here, QHPSI is enabled by default by defining it in the qmail-smtpd.25 variables directory. If you have clamd, clamav already installed on your server, the rpm installation also installs two services under supervise.

* freshclam - service to update the clamd virus databases
* clamd - service to run the clamd scanner

You may need to disable clamd, freshclam startup by your system boot process and enable the startup under indimail. Do have the clamd, freshclam service started up by indimail, remove the down file. i.e.

```
$ sudo /bin/rm /service/freshclam/down /service/clamd/down
$ sudo /usr/bin/svc -u /service/clamd /service/freshclam
```

```
$ tail -f /var/log/indimail/freshclam/current
@400000004b9da034170f6394 cdiff_apply: Parsed 17 lines and executed 17 commands
@400000004b9da03417103e54 Retrieving http://database.clamav.net/daily-10574.cdiff
@400000004b9da0342261b83c Trying to download http://database.clamav.net/daily-10574.cdiff (IP: 130.59.10.36)
Downloading daily-10574.cdiff [100%]g daily-10574.cdiff [ 13%]
@400000004b9da03509c39c64 cdiff_apply: Parsed 436 lines and executed 436 commands
@400000004b9da03510c3485c daily.cld updated (version: 10574, sigs: 24611, f-level: 44, builder: ccordes)
@400000004b9da03510c4d2e4 bytecode.cvd version from DNS: 2
@400000004b9da03510c4de9c bytecode.cvd is up to date (version: 2, sigs: 2, f-level: 44, builder: nervous)
@400000004b9da03510c82e44 Database updated (729340 signatures) from database.clamav.net (IP: 130.59.10.36)
```

```
$ cat /var/log/indimail/clamd/current
@400000004b9da0260d6c1a94 Limits: Global size limit set to 104857600 bytes.
@400000004b9da0260d6c264c Limits: File size limit set to 26214400 bytes.
@400000004b9da0260d6c3204 Limits: Recursion level limit set to 16.
@400000004b9da0260d6c3dbc Limits: Files limit set to 10000.
@400000004b9da0260d6c4974 Archive support enabled.
@400000004b9da0260d6c5144 Algorithmic detection enabled.
@400000004b9da0260d6c5cfc Portable Executable support enabled.
@400000004b9da0260d6c68b4 ELF support enabled.
@400000004b9da0260d6c7084 Detection of broken executables enabled.
@400000004b9da0260e7abfbc Mail files support enabled.
@400000004b9da0260e7acb74 OLE2 support enabled.
@400000004b9da0260e7ad344 PDF support enabled.
@400000004b9da0260e7adefc HTML support enabled.
@400000004b9da0260e7ae6cc Self checking every 600 seconds.
@400000004b9da2a3116a177c No stats for Database check - forcing reload
@400000004b9da2a3206deb04 Reading databases from /var/indimail/share/clamd
@400000004b9da2a70489facc Database correctly reloaded (728651 signatures)
@400000004b9da2a7061e372c /var/indimail/queue/queue2/mess/16/660188: OK
```

Once you have **QHPSI** enabled, <b>qmail-queue</b> will add the header **X-QHPSI** in the mail. You will have the following header

`X-QHPSI: clean`

in case the email is clean and the following header if a virus is found

`X-QHPSI: virus found`

The default configuration of IndiMail will allow these emails to be delivered to the inbox. This is because some sites have have legislations like SOX, etc to enforce archiving of all emails that come into the system. In case you want to reject the email at SMTP you can do the following

```
$ sudo /bin/bash
# echo 1 > /service/qmail.smtpd/variables/REJECTVIRUS
# svc -r /service/qmail-smtpd.25
```

One can also create a vfilter to deliver such email to the quarantine folder

```
/usr/bin/vcfilter -i -t virusFilter -c 0 -k "virus found" -f Quarantine -b 0 -h 28 prefilt@$1
```

If you implement different method, than explained above, let me know.

# Using spamassasin with IndiMail

Just few days back a user asked me whether spamassassin can be used with IndiMail.

IndiMail uses environment variables **SPAMFILTER**, **SPAMEXITCODE** to configure any spam filter to be used. All that is required for the spam filter is to read a mail message on stdin, output the message back on stdout and exit with a number which indicates whether the message is ham or spam.

The default installation of IndiMail creates a configuration where mails get scanned by bogofilter for spam filtering. bogofilter exits with value '0' in case the message is spam and with value '1' when message is ham. The settings for **SPAMFILTER**, **SPAMEXITCODE** is as below

```
SPAMFILTER="/usr/bin/bogofilter -p -u -d /etc/indimail"
SPAMEXITCODE=0 # what return value from spam filter should be treated as spam?
```

Assuming that you have installed, setup and trained spamassassin, you can follow the instructions below to have IndiMail use spamassassin.

spamassasin has a client spamc which exits 1 when message is spam and exits 0 if the message is ham. To use spamassassin, just use the following for SPAMFILTER, SPAMEXITCODE

```
SPAMFILTER="path_to_spamc_program -E-d host -p port -u user"
SPAMEXITCODE=1
```

(see the documentation on spamc for description of arguments to spamc program). You an also use -U socket_path, to use unix domain socket instead of -d host, which uses tcp/ip

Since IndiMail uses <b>envdir</b> program to set environment variable, a simple way would be to set SPAMFILTER, SPAMEXITCODE is to do the following

```
$ sudo /bin/bash
# echo "spamcPath -E -d host -p port -u user" > /service/qmail-smtpd.25/variables/SPAMFILTER
# echo 1 > /service/qmail-smtpd.25/variables/SPAMEXITCODE
```

What if you want to use both bogofilter and spamasssin. You can use a simple script like below as the SPAMFILTER program

```
#!/bin/bash
#
# you can -U option in spamc, pointing to a unix domain path instead of -d
#
DESTHOST=x.x.x.x

#
# pass the output of bogofilter to spamc and passthrough spamc output to stdout
# store the exit status of bogofilter in status1 and spamc in status2
#
/usr/bin/bogofilter -p -d /etc/indimail | /usr/bin/spamc -E -d $DESTHOST -p 783
STATUS=("${PIPESTATUS[@]}")
status1=${STATUS[0]}
status2=${STATUS[1]}

# bogofilter returned error
if [ $status1 -eq 2 ] ; then
  exit 2
fi
# spamc returned error see the man page for spamc
if [ $status2 -ge 64 -a $status2 -le 78 ] ; then
  exit 2
fi

#
# message is spam
# bogofilter returns 0 on spam, spamc returns 1 on spam
#
if [ $status1 -eq 0 -o $status2 -eq 1 ] ; then
  exit 0
fi
exit 1
```


Let us call the above script as bogospamc and let us place it in /usr/bin

```
$ sudo /bin/bash
# echo /usr/bin/bogospamc > /service/qmail-smtpd.25/variables/SPAMFILTER
# echo 0 > /service/qmail-smtpd.25/variables/SPAMEXITCODE
```

# SPF implementation in IndiMail

Sender Policy Framework (SPF) is an email authentication method designed to detect forging sender addresses during the delivery of the email. It is text record in DNS which allows to designate permitted senders for mails depending on the domain name. The goal is to disallow sender address forgery. qmail-smtpd checks incoming mails, adds Received-SPF lines and optionally blocks undesired SMTP transactions. The check is performed at the envelope level. SPF allows the receiving mail server to check during mail delivery that a mail claiming to come from a specific domain is submitted by an IP address authorized by that domain's administrators. The list of authorized sending hosts and IP addresses for a domain is published in the DNS records for that domain. You can use <b>dnstxt</b> program to get the published TXT records in DNS. You can use the <b>spfquery</b> program to check if an IP address is permitted to send mails on behalf of a domain. e.g.

```
$ dnstxt rr.com
v=spf1 ip4:24.30.203.0/24 ip4:24.28.200.0/24 ip4:24.28.204.0/24 ip4:24.30.218.0/24 ip4:75.180.132.0/24 ip4:71.74.56.0/24 ip4:107.14.166.0/24 ip4:107.14.73.0/24 +mx ~all
$ spfquery 24.30.203.1 rr.com postmaster@rr.com
result=pass
Received-SPF: pass (localhost: SPF record at rr.com designates 24.30.203.1 as permitted sender)
```

Indimail's SPF implementation has been adapted from [SPF implementation for qmail](https://www.saout.de/misc/spf/) written by Jana Saout and Christophe Saout. There are 5 configuration file in /etc/indimail/control directory that are used for configuring SPF.

* <b>spfbehavior</b> Use this to turn on SPF checking. The default value is 0 (off). You can specify a value between 0 and 6:
	- 0 Never do SPF lookups, don't create Received-SPF headers
	* 1 Only create Received-SPF headers, never block
	* 2 Use temporary errors when you have DNS lookup problems
	* 3 Reject mails when SPF resolves to fail (deny)
	* 4 Reject mails when SPF resolves to softfail
	* 5 Reject mails when SPF resolves to neutral
	* 6 Reject mails when SPF does not resolve to pass
	
	Values bigger than 3 are strongly discouraged, you probably want to go with 2 or 3.
	
	Important: This setting can be overridden using the environment variable <b>SPFBEHAVIOR</b>, e.g. from tcpserver rules or from the supervise variables directory.

	Note: If <b>RELAYCLIENT</b> is set, SPF checks won't run at all.

* <b>spfrules</b> You can specify a line with local rules. Local rules means: Rules that are executed before the real SPF rules for a domain would fail (fail, softfail, neutral). They are also executed for domains that don't publish SPF entries. It is suggested to add

		include:spf.trusted-forwarder.org
	
	You can also add mechanisms to trust known mail servers like backup MX servers, though I suggest that you should at least also use tcprules or envrules (to modify <b>SPFBEHAVIOR</b>).

* <b>spfguess</b> You can specify a line with guess rules. Guess rules means: Rules that are used if the domain doesn't publish SPF rules. The local spfrules are always executed afterwards. It is suggested to use <i>a/24 mx/24 ptr</i>. This isn't needed but generally gives good results (for spam filters scoring Received-SPF lines). The filename can be overriden by <b>SPFGUESS</b> environment variable.

* <b>spfexp</b> You can override the default SPF explanation if you want. The explanation is the line returned to the SMTP sender when a mail is rejected at the SMTP level. You can use macro expansion. If a domain specifies its own explanation it is going to be used instead.

	The SMTP answer when rejecting mails will look like:
	
		550 the expanded SPF explanation (#5.7.1)

	If you want the macro expansion explained look at the SPF draft. The filename can be overriden by <b>SPFEXP</b> environment variable.

* spfipv6 You can turn on spf for ipv6 connections by setting to value 1 or 0 to disable. You can override this control file with environment variable <b>USE\_SPFIPV6</b>.

## SPF Macro Support

SPF macros are supported by <b>qmail-smtpd</b> when doing SPF lookups. SPF macros are a feature of the SPF specification which allow for the creation of dynamic SPF policies. They allow for variables to be included within a policy, which are then evaluated by the receiving MTA and 'filled in' using data from the email being processed, such as the sender address or source IP address.

This enables various advanced SPF processing routines such as conditional lookups, and allows for additional email metadata to influence the decision.

SPF macros are present in the original SPF specification [RFC4408](https://www.rfc-editor.org/rfc/rfc4408#section-8), as well as the revised specification [RFC7208](https://tools.ietf.org/html/rfc7208#section-7).

## SPF Macro Syntax

SPF macros are represented by different single characters, surrounded by curly braces ({ }) and prepended by a percent (%) sign, e.g. %{i}. There are currently 8 'core' macros that are supported, as defined in section 7.2 of the RFC. These will be evaluated and expanded by <b>qmail-smtpd</b>.

    * %{s}: The sender of the received email, e.g. joe@example.com.
    * %{l}: The local part of the sender, e.g. joe.
    * %{o}: The domain of the sender, e.g. example.com.
    * %{d}: The authoritative domain of the current SPF policy --- usually the sender domain (%{o}), but may also represent other domains when evaluating include: statements.
    * %{i}: The source IP address of the message, e.g. 203.0.113.1.
    * %{p}: The validated reverse-DNS domain of the source IP, e.g. if example.com IN A is 203.0.113.1 and 1.113.0.203.in-addr.arpa IN PTR is example.com, the validated domain will be example.com.
    * %{v}: The static string in-addr or ip6 depending on the protocol version of the source IP --- used to construct macros utilising reverse DNS/PTR record syntax as seen in the .arpa zone.
    * %{h}: The domain used in the most recent SMTP HELO or EHLO command, e.g. mail.example.com.
    * %{t}: The current time
    * %{E}: Error message if any while doing spf lookup
    * %{R}: IP adddress of the local host.
    * %{S}: The SPF record for the domain of the sender


# SRS implementation in IndiMail

Hosts which adopt the [Sender Permitted From [SPF)](https://en.wikipedia.org/wiki/Sender_Policy_Framework) convention face a challenge when required to forward mail. If the forwarding host does not change the sender domain, it will fail the SPF test and may not be able to hand the message off to the recipient. 
The [Sender Rewriting Scheme (SRS)](https://en.wikipedia.org/wiki/Sender_Rewriting_Scheme) is a scheme for bypassing the Sender Policy Framework's (SPF) methods of preventing forged sender addresses. SPF "breaks" email forwarding. SRS is a way to fix it. SRS is a simple way for forwarding MTAs to rewrite the sender address. The original concept was published in [draft-mengwong-sender-rewrite](http://www.open-spf.org/svn/project/specs/drafts/draft-mengwong-sender-rewrite-01.txt) and further expanded on in a [paper by Shevek](http://www.open-spf.org/srs/srs.pdf). IndiMail's SRS implementation has been adapted from Marcelo Coelho's [qmail SRS patch](http://www.mco2.com.br/opensource/qmail/srs/), which uses a [forked](https://github.com/mbhangui/indimail-mta/tree/master/libsrs2-x) version of the [libsrs2](https://www.libsrs2.org/) library. The Sender Rewriting Scheme is depicted pictorially below, taken from [open-spf.org](http://www.open-spf.org/SRS/).

![Sender Rewriting Scheme](Sender-Rewriting-Scheme.png "SRS")

## Configuration Parameters

SRS is configured by setting libsrs2 parameters. To configure SRS in IndiMail you need to at the least configure the control files <u>srs_domain</u> and <u>srs_secrets</u>. The various SRS control files are given in the table below. Each control file controls SRS parameter of the same name as the control file. e.g. The control file <u>srs_domain</u> controls the libsrs2 parameter <b>srs_domain</b>. Each control filename can be overriden by corresponding environment variable with capital letters. e.g. The environment variable <b>SRS_DOMAIN</b> overrides the the control file <u>srs_domain</u>

Parameters|Description|Example
----------|-----------|-------
<b>srs_domain</b>|A domain to use in rewritten addresses. If not set, SRS is disabled.|srs.indimail.org
<b>srs_secrets</b>|A random string to generate and check SRS addresses. You can specify a list of secrets (one per line). The first secret in the list is used for generating new SRS addresses. All secrets on the list may be used to verify SRS addresses.|b1YI?,uL7f=oH
<b>srs_maxage</b>|The maximum permitted age of a rewritten address. SRS rewritten addresses expire after a specified number of days. libsrs2 default is 21, but I believe that a week is enougth to get all bounces, so I recommend you to use 7.|7
<b>srs_hashlength</b>|The hash length to generate in a rewritten address. The hash length is a measure of security in the SRS system; longer is more secure.|4
<b>srs_hashmin</b>|The hash length to require when checking an address. If the hash length is increased, there may be SRS addresses from your MTA in the wild which use a shorter hash length. This parameter may be set to permit checking of hashes shorter than srs\_hashlength. This parameter must be at most srs\_hashlength.|4
<b>srs_separator</b>|The separator to appear immediately after SRS[01] in rewritten addresses. This must be -, + or =. Default value is =.|=
<b>srs_alwaysrewrite</b>|Skip rcpthosts check and perform SRS rewriting for all forwarding, even when not required. This must be 0 (disabled) or 1 (enabled). Default value is 0 (disabled).|0

Now that we have described the SRS parameters, we can go ahead and configure SRS by following the below steps.

1. Configure <u>srs domain</u> control file

		$ cd /etc/indimail/control
		$ sudo bash
		# echo srs.domain.tld > srs_domain

2. Configure <u>srs_secrets</u> control file

		$ cd /etc/indimail/control
		$ sudo bash
		# tr -dc 'A-Za-z0-9!"#$%&'\''()*+,-./:;<=>?@[\]^_`{|}~' \
			</dev/urandom | head -c 24 > srs_secrets

3. Configure optional control files / parameters

		$ cd /etc/indimail/control
		$ sudo bash
		# echo 7 > srs_maxage
		# echo 4 > srs_hashlength
		# echo 4 > srs_hashmin
		# echo = > srs_separator
		# echo 0 > srs_alwaysrewrite

4. Configure indimail to receive mails for your SRS domain.

		$ cd /etc/indimail/control
		$ sudo bash
		# echo srs.domain.tld         > rcpthosts

		The below steps are not needed if you use qmail-smtpd. qmail-smtpd by
		itself has the ability to decode a SRS address. But you can do the below
		steps in case you are using mini-smtpd or other methods to receive mails
		from outside like ofmipd, qmail-smtpd from qmail, netqmail, notqmail, etc.
		In such cases you require srsfilter to decode SRS addresses.

		$ cd /etc/indimail/control
		$ sudo bash
		# echo srs.domain.tld:srs     > virtualdomains
		# cd /var/indimail/alias
		# echo "|/usr/bin/srsfilter" > .qmail-srs-default
		# svc -h /service/qmail-send.25

# Greylisting in IndiMail

Greylisting is a method of defending email users against spam, by temporarily rejecting any email from a IP/Sender which it does not recognize. As per SMTP, the originating server should retry after a delay. A server implementing greylisting should accept the mail if sufficient time has elapsed. If the mail is from a spammer it will probably not be retried since a spammer goes through thousands of email addresses and typically cannot afford the time delay to retry.

IndiMail 1.6 onwards implements greylisting using <b>qmail-greyd</b> daemon. You additionally need to set the environment variable GREYIP defined for the <b>qmail-smtpd</b> process. The environment variable GREYIP specifies on which IP and port, <b>qmail-greyd</b> is accepting greylisting requests. <b>qmail-smtpd</b> uses UDP to send a triplet (IP+RETURN\_PATH+RECIPIENT) to the greylisting server and waits for an answer which tells <b>qmail-smtpd</b> to proceed ahead or to temporarily reject the mail. <b>qmail-greyd</b> also accepts a list of whitelisted IP addresses for which greylisting should not be done.

## Enabling qmail-greyd greylisting server

```
$ sudo /ur/sbin/svctool --greylist=1999 --servicedir=/service --min-resend-min=2 \
   --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
   --save-interval=5 --whitelist=greylist.whitelist
```

NOTE: The above service has already been setup for you, if you have done a binary installation of IndiMail/indimail-mta

## Enabling greylisting in SMTP

Assuming you've setup your <b>qmail-smtpd</b> service with tcpserver with the -x option (as in LWQ), you just need to update the cdb file referenced by this -x option. The source for this file is typically /etc/indimail/tcp.smtp. For example,

```
127.:allow,RELAYCLIENT=""
192.168.:allow,RELAYCLIENT=""
:allow
```

could become,


```
127.:allow,RELAYCLIENT=""
192.168.:allow,RELAYCLIENT=""
:allow,GREYIP="127.0.0.1@1999"
```

If you've setup <b>qmail-greyd</b> on a non-default address (perhaps you're running <b>qmail-greyd</b> on a separate machine), you'll also need to specify the address it's listening on - adjust the above to include GREYIP="192.168.5.5@1999", for example.
Finally, don't forget to update the cdb file corresponding to the source file you've just edited. If you have a LWQ setup that's

```
$ sudo /bin/bash
# /usr/bin/qmailctl cdb
```

Alternatively (and particularly if you're not using the -x option to tcpserver) you can enable greylisting for all SMTP connections by setting GREYIP environment variable for <b>qmail-smtpd</b> service.

```
$ sudo /bin/bash
# echo GREYIP=\"127.0.0.1@1999\" > /service/qmail-smtpd.25/variables/GREYIP
```

# Configuring dovecot as the IMAP/POP3 server

IndiMail stores it's virtual user information in MySQL. However, IndiMail can work with virtually any IMAP/POP3 server which has a mechanism to authenticate using [checkpassword](https://cr.yp.to/checkpwd/interface.html) interface or has PAM, which can use the system's password database for user's home directory. This is because IndiMail provides checkpassword, PAM module and a NSS service, all independent of each other. The beauty of providing checkpassword, PAM module and NSS mechansism, is that you do not have to modify a single line of code anywhere. In this respect, IndiMail is probably the most flexible messaging server available at the moment.
 * checkpassword module is vchkpass
 * PAM module is [pam-multi - PAM modules for flexible, configurable authentication methods](https://github.com/mbhangui/indimail-virtualdomains/tree/master/pam-multi-x "pam-multi")
 * NSS is through [nssd - providing Name Service Switch](https://github.com/mbhangui/indimail-virtualdomains/tree/master/nssd-xi "nssd")


[Dovecot](https://www.dovecot.org/) is an open source IMAP and POP3 server for Linux/UNIX-like systems, written with security primarily in mind. Dovecot is an excellent choice for both small and large installations. It's fast, simple to set up, requires no special administration and it uses very little memory. Though I do not use dovecot, I have heard excellent reviews from users about dovecot. It took me less than 20 minutes to download dovecot today and have it working with IndiMail with all existing mails intact and accessible. So at the moment, my IndiMail installation is working with both courier-imap and dovecot simultaneously (with different IMAP/POP3 ports assigned to courier-imap and dovecot). dovecot isn't as modular and pluggable as courier-imap. Also it isn'textremely supervise friendly like courier-imap and you will need to install and configure it outside supervise.

## Installation

To install dovecot you just need to use dnf/yum/apt to install

### For RPM based systems

If you are doing to use dovecot, you need to disable courier-imap

```
$ sudo /bin/bash
# for i in /service/proxy-imapd.4143 /service/proxy-imapd-ssl.9143
   /service/qmail-imapd.143 /service/qmail-imapd-ssl.993
   /service/proxy-pop3d.4110 /service/proxy-pop3d-ssl.9110
   /service/qmail-pop3d.110 /service/qmail-pop3d-ssl.995
 do
   touch $i/down
   svc -dx $i $i/log
 done
```

```
# dnf -h install dovecot dovecot-mysql
# systemctl enable dovecot
```

NOTE: replace dnf with yum for distros like RHEL, CentOS, etc. For openSUSE, SUSE replace dnf with zypper

### For Debian based systems

```
# apt-get -y update
# apt-get -y install dovecot dovecot-mysql
# systemctl enable dovecot
```

Like most other IMAP/POP3 servers, dovecot is configurable and can use multiple methods to authenticate and as well get other information about the user such as home directory, user id, etc.

## Configuring Authentication Methods

### Basic Configuration

Dovecot uses /etc/dovecot/dovecot.conf as the main configuration file.

```
protocols = imap pop3
import_environment = QUERY_CACHE=1 DOMAIN_LIMITS=1 FIFODIR=/var/indimail/inquery INFIFO=infifo
```

NOTE: If you decide to use any of indimail's checkpassword authentication modules and if you need to enable QUERY\_CACHE above, make sure to comment out `#PrivateTmp=true` in the file `/usr/lib/systemd/system/dovecot.service. If you don't disable PrivateTmp and you are using any of the indimail's checkpassword modules for authentication, the modules will not be able to communicate with inlookup query daemon. This is because the /tmp that inlookup uses will be different from the /tmp directory that dovecot auth process will use. checkpassword modules create a fifo in the /tmp directory and exchange the name with inlookup through another fifo. If you do disable PrivateTmp, then do not forget to issue the command

```
# systemctl daemon-reload
```

You need to have auth-system.conf.ext for authenticating users in your system's password database.

file /etc/dovecot/conf.d/10-auth.conf

```
auth_mechanisms = plain login
!include auth-system.conf.ext
```

Indimail uses $HOME/Maildir for all virtual accounts

file /etc/dovecot/conf.d/10-mail.conf

```
mail_location = maildir:~/Maildir
```

It is important that the dovecot auth process runs as the user indimail so that it can read indimail control files and configuration.

file /etc/dovecot/conf.d/10-master.conf

```
service auth {
  # auth_socket_path points to this userdb socket by default. It's typically
  # used by dovecot-lda, doveadm, possibly imap process, etc. Users that have
  # full permissions to this socket are able to get a list of all usernames and
  # get the results of everyone's userdb lookups.
  #
  # The default 0666 mode allows anyone to connect to the socket, but the
  # userdb lookups will succeed only if the userdb returns an "uid" field that
  # matches the caller process's UID. Also if caller's uid or gid matches the
  # socket's uid or gid the lookup succeeds. Anything else causes a failure.
  #
  # To give the caller full permissions to lookup all users, set the mode to
  # something else than 0666 and Dovecot lets the kernel enforce the
  # permissions (e.g. 0777 allows everyone full permissions).
  unix_listener auth-userdb {
    #mode = 0666
    #user = 
    #group = 
  }

  # Auth process is run as this user.
  #user = $default_internal_user
  user = indimail
  group = qmail
}

service auth-worker {
  unix_listener auth-worker {
    mode = 0660
    group = qmail
  }
  # Auth worker process is run as root by default, so that it can access
  # /etc/shadow. If this isn't necessary, the user should be changed to
  # $default_internal_user.
  #user = root
  group=qmail
}
```

You also need to configure the ssl certs. The default certificate used by indimail is /etc/indimail/certs/servercert.pem.

File /etc/dovecot/conf.d/10-ssl.conf

```
ssl = required
ssl_cert = </etc/indimail/certs/servercert.pem
ssl_key = </etc/indimail/certs/servercert.pem
ssl_cipher_list = PROFILE=SYSTEM
```

There are four methods you can configure dovecot to work with indimail

### Use dovecot's SQL driver

file /etc/dovecot/conf.d/10-auth.conf

```
!include auth-sql.conf.ext
```

file /etc/dovecot/connf.d/auth-sql.conf.ext
```
passdb {
  driver = sql
  # Path for SQL configuration file, see example-config/dovecot-sql.conf.ext
  args = /etc/dovecot/dovecot-sql.conf
}

userdb {
  driver = prefetch
}

userdb {
  driver = sql
  args = /etc/dovecot/dovecot-sql.conf
}
```

```
file /etc/dovecot/dovecot-sql.conf
driver = mysql
default_pass_scheme = MD5
connect = host=/run/mysqld/mysqld.sock user=indimail password=ssh-1.5- dbname=indimail

#
user_query = SELECT pw_name,555 as uid, 555 as gid, pw_dir as home FROM indimail WHERE pw_name = '%n' AND pw_domain = '%d'

# The below passes all users and doesn't care for indimail vlimits (pw_gid column or vlimits table)
password_query = SELECT pw_passwd as password FROM indimail WHERE pw_name = '%n' AND pw_domain = '%d'

#
# A little bit more complicated query to support indimail pw_gid flags and vlimits for domain
# explanation:
# We're using bitwise operations on pw_gid.
# as defined in indimail.h:
# #define NO_POP                  0x02 - no pop3
# #define NO_WEBMAIL              0x04 - no imap
# #define NO_IMAP                 0x08 - no webmail
#
# !(pw_gid & 2) means - if 2nd bit of pw_gid is not set
# !(pw_gid & 4) means - if 4th bit of pw_gid is not set
# !(pw_gid & 8) means - if 8th bit of pw_gid is not set
# (pw_gid & 100) means - if 8 bits of pw_gid is set (ignore vlimits)
#
# additionally because we're using LEFT JOIN we have to take care of NULLs for rows
# that don't return any records from the right table hence the use of COALESCE() function
# !(pw_gid & 4) (disable webmail flag) is used in conjuntion with '%r'!="127.0.0.1"
# which means that it will only apply to connections originating from hosts other than localhost
#
# So the below query supports pw_gid and vlimits settings for user account and domains but no domain limit overrides
#
#password_query = select pw_passwd as password FROM indimail LEFT JOIN vlimits ON indimail.pw_domain=vlimits.domain WHERE pw_name='%n' and pw_domain='%d' and ( !(pw_gid & 8) and ('%r'!='127.0.0.1' or !(pw_gid & 4)) and ( '%r'!='127.0.0.1' or COALESCE(disable_webmail,0)!=1) and COALESCE(disable_imap,0)!=1);

#
# The below adds support for vlimits override on user account (vmoduser -o)
#
# logically this means: show password for user=%n at domain=%d when imap on the account
# is not disabled and connection is not comming from localhost when webmail access on
# the account is not disabled and if imap for the domain is not disabled and (connection
# is not comming from localhost when webmail access for the domain is not disabled) when
# vlimits are not overriden on the account
#
#password_query = select pw_passwd as password FROM indimail LEFT JOIN vlimits ON indimail.pw_domain=vlimits.domain WHERE pw_name='%n' and pw_domain='%d' and !(pw_gid & 8) and ('%r'!='127.0.0.1' or !(pw_gid & 4)) and ( ('%r'!='127.0.0.1' or COALESCE(disable_webmail,0)!=1) and COALESCE(disable_imap,0)!=1 or (pw_gid & 8192) );
```

### Using checkpassword

The checkpassword interface used by dovecot uses dovecot specific extensions. IndiMail provides `vchkpass`, a checkpassword implementation compatible with dovecot.

file /etc/dovecot/conf.d/10-auth.conf

```
!include auth-checkpassword.conf.ext
```

file /etc/dovecot/conf.d/auth-checkpassword.conf.ext

```
passdb {
  driver = checkpassword
  args = /usr/sbin/vchkpass
}

userdb {
  driver = prefetch
}
```

### Using pam-multi

Create the file /etc/dovecot/conf.d/auth-indimail-conf.ext

```
# Authentication for system users. Included from 10-auth.conf.
#

# PAM authentication. Preferred nowadays by most systems.
# PAM is typically used with either userdb passwd or userdb static.
# REMEMBER: You'll need /etc/pam.d/dovecot file created for PAM
# authentication to actually work. <doc/wiki/PasswordDatabase.PAM.txt>
passdb {
  driver = pam
  # [session=yes] [setcred=yes] [failure_show_msg=yes] [max_requests=<n>]
  # [cache_key=<key>] [<service name>]
  args = pam-multi
}

##
## User databases
##

# System users (NSS, /etc/passwd, or similar). In many systems nowadays this
# uses Name Service Switch, which is configured in /etc/nsswitch.conf.
userdb {
  driver = passwd
}
```

file /etc/dovecot/conf.d/10-auth.conf

```
!include auth-indimail.conf.ext
```

### Using indimail's Name Service Switch

file /etc/dovecot/conf.d/10-auth.conf

```
!include auth-system.conf.ext
```

In file /etc/nsswitch.conf

```
passwd: files nssd
shadow: files nssd
```

In file /etc/indimail/nssd.conf

```
getpwnam    SELECT pw_name,'x',555,555,pw_gecos,pw_dir,pw_shell \
            FROM indimail \
            WHERE pw_name='%1$s' and pw_domain='%2$s' \
            LIMIT 1
getspnam    SELECT pw_name,pw_passwd,'1','0','99999','0','0','-1','0' \
            FROM indimail \
            WHERE pw_name='%1$s'and pw_domain='%2$s' \
            LIMIT 1
getpwent    SELECT pw_name,'x',555,555,pw_gecos,pw_dir,pw_shell \
            FROM indimail LIMIT 100
getspent    SELECT pw_name,pw_passwd,'1','0','99999','0','0','-1','0' \
            FROM indimail

host        localhost
database    indimail
username    indimail
password    ssh-1.5-
socket      /var/run/mysqld/mysqld.sock
pidfile     /run/indimail/nssd.pid
threads     5
timeout     -1
facility    daemon
priority    err
```
## Start dovecot

You can start dovecot with a single command `sudo service dovecot start`

## Migration from courier-imap to dovecot

If you are already running courier-imap and want to migrate to dovecot, you need to convert your existing data or make dovecot read existing data.

### IMAP migration

When migrating mails from another IMAP server, you should make sure that these are preserved:

1. Message flags
   * Lost flags can be really annoying, you most likely want to avoid it.

2. Message UIDs and UIDVALIDITY value
   * If UIDs are lost, at the minimum clients' message cache gets cleaned and messages are re-downloaded as new.
   * Some IMAP clients store metadata by assigning it to specific UID, if UIDs are changed these will be lost.

3. Mailbox subscription list
   * Users would be able to manually subscribe them again if you don't want to mess with it.

### POP3 migration

When migrating mails from another POP3 server, you should try to preserve the old UIDLs. If POP3 client is configured to keep mails in the server and the messages' UIDLs change, all the messages are downloaded again as new messages.

Don't trust the migration scripts or anything you see in this wiki. Verify manually that the UIDLs are correct before exposing real clients to Dovecot. You can do this by logging in using your old POP3 server, issuing UIDL command and saving the output. Then log in using Dovecot and save its UIDL output as well. Use e.g. diff command to verify that the lists are identical. Note that:

  * If a client already saw changed UIDLs and decided to start re-downloading mails, it's unlikely there is anything you can do to stop it. Even going back to your old server is unlikely to help at that point.
  * Some (many?) POP3 clients also require that the message ordering is preserved.
  * Some clients re-download all mails if you change the hostname in the client configuration. Be aware of this when testing.

Some servers (UW, Cyrus) implementing both IMAP and POP3 protocols use the IMAP UID and UIDVALIDITY values for generating the POP3 UIDL values. To preserve the POP3 UIDL from such servers you'll need to preserve the IMAP UIDs and set pop3\_uidl\_format properly.

If the server doesn't use IMAP UIDs for the POP3 UIDL, you'll need to figure out another way to do it. One way is to put the UIDL value into X-UIDL: header in the mails and set pop3\_reuse\_xuidl=yes. Some POP3 servers (QPopper) write the X-UIDL: header themselves, making the migration easy.

Some POP3 servers using Maildir uses the maildir base filename as the UIDL. You can use pop3\_uidl\_format = %f to do this.

### Courier IMAP/POP3

[courier-dovecot-migrate.pl](https://dovecot.org/tools/courier-dovecot-migrate.pl) does a perfect migration from Courier IMAP and POP3, preserving IMAP UIDs and POP3 UIDLs. It reads Courier's courierimapuiddb and courierpop3dsizelist files and produces dovecot-uidlist file from it.

Before doing the actual conversion you can run the script and see if it complains about any errors and such, for example:

```
# ./courier-dovecot-migrate.pl --to-dovecot --recursive /home
Finding maildirs under /home
/home/user/Maildir/dovecot-uidlist already exists, not overwritten
/home/user/Maildir2: No imap/pop3 uidlist files
Total: 69 mailboxes / 6 users
       0 errors
No actual conversion done, use --convert parameter
```

The actual conversion can be done for all users at once by running the script with --convert --recursive parameters. Make sure the conversion worked by checking that dovecot-uidlist files were created to all maildirs (including to subfolders).

The --recursive option goes through only one level down in directory hierarchies. This means that if you have some kind of a directory hashing scheme (or even domain/username/), it won't convert all of the files.

You can also convert each user as they log in for the first time, using [PostLoginScripting](https://wiki.dovecot.org/PostLoginScripting) with a script something like:

```
#!/bin/sh
# WARNING: Be sure to use mail_drop_priv_before_exec=yes,
# otherwise the files are created as root!

courier-dovecot-migrate.pl --quiet --to-dovecot --convert Maildir
# This is for imap, create a similar script for pop3 too
exec /usr/local/libexec/dovecot/imap
```
FIXME: The script should rename also folder names that aren't valid mUTF-7. Dovecot can't otherwise access such folders.

### Dovecot configuration

Courier by default uses "INBOX." as the IMAP namespace for private mailboxes. If you want a transparent migration, you'll need to configure Dovecot to use a namespace with "INBOX." prefix as well. In /etc/dovecot/conf.d/10-mail.conf

```
mail_location = maildir:~/Maildir

namespace inbox {
  prefix = INBOX.
  separator = .
  inbox = yes
}
```

### Manual conversion

* Courier's courierimapsubscribed file is compatible with Dovecot's subscriptions file, but you need to remove the "INBOX." prefixes from the mailboxes. This is true even if you set namespace prefix to "INBOX." as described above.
* Courier's courierimapuiddb file is compatible with Dovecot's dovecot-uidlist file, just rename it.
* Courier's message flags are compatible with Dovecot (as they are specified by the Maildir specification)
* Courier's message keywords implementation isn't Dovecot compatible. There doesn't exist a simple way to convert the keywords manually.

# Configuring DKIM

**What is DKIM**

DomainKeys Identified Mail (DKIM) lets an organization take responsibility for a message while it is in transit. DKIM has been approved as a Proposed Standard by IETF and published it as RFC 4871, RFC 6376. There are number of vendors/software available which provide DKIM signing. IndiMail is one of them. You can see the full list [here](https://dkim.org/deploy). IndiMail supports RSA-SHA1, RSA-SHA256 and [Ed25519-SHA256](https://www.rfc-editor.org/rfc/rfc8463.html) Cryptographic signature methods for DKIM.

DKIM uses public-key cryptography to allow the sender to electronically sign legitimate emails in a way that can be verified by recipients. Prominent email service providers implementing DKIM (or its slightly different predecessor, DomainKeys) include Yahoo and Gmail. Any mail from these domains should carry a DKIM signature, and if the recipient knows this, they can discard mail that hasn't been signed, or has an invalid signature.

IndiMail from version 1.5 onwards, comes with a drop-in replacement for <b>qmail-queue</b> for DKIM signature signing and verification (see <b>qmail-dkim</b>(8) for more details). You need the following steps to enable DKIM. IndiMail from version 1.5.1 onwards comes with a filter dk-filter, which can be enabled before mail is handed over to <b>qmail-local</b> or <b>qmail-remote</b> (see <b>spawn-filter</b>(8) for more details).

You may want to look at an excellent [setup instructions](http://notes.sagredo.eu/node/92 "Roberto's Notes") by Roberto Puzzanghera for configuring dkim for qmail

## Create your DKIM signature

```
$ sudo /bin/bash
# mkdir -p /etcindimail/control/domainkeys
# chown root:qcerts /etc/indimail/control/domainkeys
# cd /etc/indimail/control/domainkeys
# dknewkey -b 2048 /etc/indimail/control/default
```

If you see above, the private key <b>default</b> has can be read only by the <b>root</b> UNIX user or a user who is part of the <b>qcerts</b> UNIX group. This means that only the <b>root</b> user or users who are part of the <b>qcerts</b> group can sign messages to have a DKIM signature.

## Create your DNS records

```
$ cd /etc/indimail/control/domainkeys
$ pubkey=$(openssl rsa -in default -pubout -outform PEM | grep -v '^--' | tr -d '\n')

The next command will print the text that you need to put in your txt record for
default._domainkey.indimail.org
$ printf "default._domainkey.indimail.org\tIN\tTXT\t\"v=DKIM1; k=rsa; p=%s\"\n" "$pubkey"
default._domainkey.indimail.org IN      TXT     "v=DKIM1; k=rsa; p=MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC2dvktnCXRavyuuoy2NUcHWpMp/Ia7Y5Y9tTwjjby7hS9wIvgecBz6UEMunOJdAZ2RVvSXKxPlxO4/rUgW6ow7vlEPY3IKagy+VFW1oHmvj4WU+BxZTJA2d8VrW9S9O1JMuPGGwdeYOC/Gcle/EviQtGYsz3jL/HrJb9rXXl4/gwIDAQAB"
```

choose the selector (some\_name) and publish this into DNS TXT record for:

`selector._domainkey.indimail.org` (e.g. selector can be named 'default')

Wait until it's on all DNS servers and that's it.

## Set SMTP to sign with DKIM signatures

<b>qmail-dkim</b> uses openssl libraries and there is some amount of memory allocation that happens. You may want to increase your softlimit (if any) in your <b>qmail-smtpd</b> run script.

```
$ sudo /bin/bash
# cd /service/qmail-smtpd.25/variables
# echo "/usr/bin/qmail-dkim" > QMAILQUEUE
# echo "/etc/indimail/control/domainkeys/default" > DKIMSIGN
# svc -r /service/qmail-smtpd.25
```

## Set SMTP to verify DKIM signatures

You can setup <b>qmail-smtpd</b> for verification by setting DKIMIVERIFY environment variable instead of DKIMSIGN environment variable.

```
$ sudo /bin/bash
# cd /service/qmail-smtpd.25/variables
# echo "/usr/bin/qmail-dkim" > QMAILQUEUE
# echo FGHIKLMNOQRTUVWjp > DKIMVERIFY
# svc -r /service/qmail-smtpd.25
```

## DKIM sign during remote delivery

```
$ sudo /bin/bash
# cd /service/qmail-send.25/variables
# echo "/usr/bin/spawn-filter" > QMAILREMOTE
# echo "/usr/bin/dk-filter" > FILTERARGS
# echo "/etc/indimail/control/domainkeys/default" > DKIMSIGN
# echo "-h" > DKSIGNOPTIONS
# svc -r /service/qmail-send.25
```

## DKIM sign during local delivery

On your host which serves as your incoming gateway for your local domains, it only makes sense to do DKIM verification with <b>qmail-local</b>

```
$ sudo /bin/bash
# cd /service/qmail-send.25/variables
# echo "/usr/bin/spawn-filter" > QMAILLOCAL
# echo "/usr/bin/dk-filter" > FILTERARGS
# echo FGHIKLMNOQRTUVWjp > DKIMVERIFY
# svc -r /service/qmail-send.25
```

## DKIM sign during mail injection

Any UNIX user on the system can sign their outbound emails with DKIM signature during mail injection itself. All that is required is to have a DKIM public/private key created using dknewkey as given in [Create your DKIM signature](#create-your-dkim-signature). Once you have done that, the user injecting the mail, will require read access to the private key. We have two unique cases. One for system users and another for users having shell accounts.

<b>System Users</b>

If this is a system user (users without shell access) that will be sending out the email then you just need to add this user to the <b>qcerts</b> group. e.g. for php scripts sending out emails, the following command will add the user <b>apache</b> to have <b>qcerts</b> as a supplementary group.

```
/usr/sbin/usermod -aG qcerts apache
```

<b>Non-System users</b>

If you want actual UNIX users who have shell access to the system, have the DKIM private key have the read permission for the primary group of the user. e.g. A UNIX user with <b>user1</b> as the username and <b>group1</b> as the primary group, do the following

```
$ sudo /bin/bash
# mkdir ~/domainkeys
# chmod 700 ~/domainkeys
# dknewkey -b 2048 ~/domainkeys/default
# cd ~/domainkeys
# chown user1:group1 default (name of our selector)
# chown user1:group1 default.pub
# chmod 640 default
# chmod 640 default.pub
```

Now to use the above DKIM key, we need to set few environment variables specific to <b>user1</b>. To do that we just use the envdir property of indimail-mta where any file in ~/.defaultqueue becomes an environment variable set by programs like <b>qmail-inject</b>. Refer to point 3 in [Setting Environment Variables](#setting-environment-variables) for reference. Assuming <u>/home/user1</u> is the home directory for <b>user1</b>, we can create DKIMSIGN environment variable as below.

```
$ mkdir ~/.defaultqueue
$ cd ~/.defaultqueue
$ echo "/home/user1/domainkeys/default" > DKIMSIGN
```

## DKIM Author Domain Signing Practices

IndiMail supports ADSP. A DKIM Author Signing Practice lookup is done by the verifier to determine whether it should expect email with the From: address to be signed.

The Sender Signing Practice is published with a DNS TXT record as follows:

`_adsp._domainkey.indimail.org. IN TXT "dkim=unknown"`

The dkim tag denotes the outbound signing Practice. unknown means that the indimail.org domain may sign some emails. You can have the values "discardable" or "all" as other values for dkim tag. discardable means that any unsigned email from indimail.org is recommended for rejection. all means that indimail.org signs all emails with dkim.

You may decide to consider ADSP as optional until the specifications are formalised. To set ADSP you need to set the environment variable SIGNPRACTICE=adsp. i.e

`# echo adsp > /service/smtpd.25/variables/SIGN_PRACTICE`

You may not want to do DKIM signing/verificaton by SMTP. In that case, you have the choice of using the QMAILREMOTE, QMAILLOCAL environment variables which allows IndiMail to run any script before it gets passed to <b>qmail-remote</b>, <b>qmail-local</b> respectively.
Setting <b>qmail-remote</b> to sign with DKIM signatures On your host which sends out outgoing mails, it only make sense to do DKIM signing and not verification.

## Testing outbound signatures

Once you have installed your private key file and added your public key to your DNS data, you should test the server and make sure that your outbound message are having the proper signatures added to them. You can test it by sending an email to sa-test (at) sendmail dot net. This reflector will reply (within seconds) to the envelope sender with a status of the DomainKeys and DKIM signatures.

If you experience problems, consult the <b>qmail-dkim</b> man page or post a comment below and I’ll try to help.
You can also use the following for testing.

* dktest@temporary.com, is Yahoo!'s testing server. When you send a message to this address, it will send you back a message telling you whether or not the domainkeys signature was valid.
* sa-test@sendmail.net is a free service from the sendmail people. It's very similar to the Yahoo! address, but it also shows you the results of an SPF check as well.

All the above was quite easy. If you don't think so, you can always use the magic options --dkverify (for verification) or `--dksign --private_key=domain_key_private_key_file` to svctool (svctool --help for all options) to create supervice run script for <b>qmail-smtpd</b>, <b>qmail-send</b>.

References

1. http://www.brandonturner.net/blog/2009/03/dkim-and-domainkeys-for-qmail/
2. http://qmail.jms1.net/patches/domainkeys.shtml
3. http://notes.sagredo.eu/node/82

# iwebadmin – Web Administration of IndiMail

I always find using the web ugly. It is a pain using the mouse almost all the time to do anything. One of the reasons I have never focussed on building a web administration tool for Indimail.

Lately my users have been pestering me if something can be done about it. I have no knowledge of web scripting, etc. But using some bit of common sense, I have managed to make qmailadmin work with IndiMail by modifying the source code (lucky for me, they are written in C).

For the admin user it provides

1.  user addition
2.  user deletion
3.  password change
4.  adding autoresponders
5.  deleting autoresponders
6.  modifying autoresponders
7.  adding forwarding addresses
8.  deleting forwarding addresses
9.  modifying forwarding addreses
10. quota modification

For users other than the postmaster account it provides

1. Password change
2. add/modify/delete forwarding addresses
3. add/modify/delete autoresponder


The RPM / Yum / APT Repo file can be installed using instructions at
http://software.opensuse.org/download.html?project=home:indimail&amp;package=iwebadmin

After installation, you just need to go to http://127.0.0.1/cgi-bin/iwebadmin

* The image assets get installed in /var/www/html/images/iwebadmin
* The html assets get installed in /usr/share/iwebadmin/html
* The language files get installed in /usr/share/iwebadmin/lang

The screen shots are below

![iwebadmin login](iwebadmin_login.jpg)
![iwebadmin menu](iwebadmin_menu.jpg)
![list accounts](iwebadmin_accounts.jpg)
![modify accounts](modify_account.jpg)
![iwebadmin forwards](iwebadmin_forwards.jpg)
![autoresponder list](mail_autoresponder1.jpg)
![autoresponder modify](mail_autoresponder2.jpg)
![mailinglist list](mailing_list1.jpg)
![mailinglist modify](mailing_list2.jpg)
![mailinglist new](mailing_list_new.jpg)


# Publishing statistics for IndiMail Server

You can now configure MRTG Graphs to show statistics for IndiMail. You need to have mrtg installed on your system. If you do not have mrtg, you can execute yum/dnf

`$ sudo yum install mrtg`

Bulk of the statistics is generated by a utility [qmailmrtg](https://github.com/mbhangui/indimail-mta/wiki/qmailmrtg.1) that reads log files created by <b>multilog</b> in <u>/var/log/svc</u> directory. The statistics generated by qmailmrtg includes service uptimes. At the moment the graphs display status of SMTP, IMAP, POP3 (plain and encrypted) services, inlookup cache service, MySQL service, tinydns and dnscache services. Few of the graphs (load, disk, network and memory utilization) come from snmpd. If you don't have snmpd installed, these graphs won't be shown

You need to execute the following steps (assuming your web server document root is /var/www/html)

`$ sudo /usr/sbin/svctool --mrtg=/var/www/html/mailmrtg --servicedir=/service`

The above command will also create a base configuration for snmpd in /etc/indimail/snmpd.conf.

After the above command is run, you can edit /service/mrtg/etc/indimail.mrtg.cfg to add, modify or remove entries as per your requirements.

After carrying out the above step using `svctool`, check the status of mrtg service.

```
$ sudo svstat /service/mrtg
/service/mrtg/: up 35254 seconds pid 2443
```

Point your browser to [http://localhost/mailmrtg/](http://localhost/mailmrtg/) and you should see graphs like below.

![MRTG Graphs for IndiMail](mrtg1.png)
![MRTG Graphs cont..](mrtg2.png)
![MRTG Graphs cont..](mrtg3.png)

# RoundCube Installation for IndiMail

These instructions will work on CentOS, RHEL, Fedora. For Debian/Ubuntu and other distros, please use your knowledge to make changes accordingly. In this guide, replace indimail.org with your own hostname.
Non SSL Version Install/Configuration
(look below for SSL config)

1. Install RoundCube. On older systems, use the yum command
   `$ sudo dnf -y install roundcubemail php-mysqlnd`

2. Connect to MySQL using a privileged user. IndiMail installation creates a privileged mysql user 'mysql'. It does not have the user 'root'. Look at the variable PRIV\_PASS in /usr/sbin/svctool to know the password.

   ```
   $ /usr/bin/mysql -u mysql -p mysql
   MySQL> create database RoundCube_db;
   MySQL> create user roundcube identified by 'subscribed';
   MySQL> GRANT ALL PRIVILEGES on RoundCube_db.* to roundcube;
   MySQL> FLUSH PRIVILEGES;
   MySQL> QUIT;
   $ /usr/bin/mysql -u mysql -p RoundCube_db < /usr/share/roundcubemail/SQL/mysql.initial.sql
   ```

3. Copy /etc/roundcube/config.inc.php.sample to /etc/roundcube.inc.php
   `$ sudo cp /etc/roundcube/config.inc.php.sample /etc/roundcubemail/config.inc.php`

    Edit the lines in /etc/roundcube/config.inc.php

    ```
    $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
    $config['smtp_server'] = 'localhost';
    $config['smtp_port'] = 587;
    $config['smtp_user'] = '%u';
    $config['smtp_pass'] = '%p';
    $config['support_url'] = 'http://indimail.sourceforge.net';
    $config['product_name'] = 'IndiMail Webmail';
    $config['plugins'] = array(
           'archive',
           'sauserprefs',
           'markasjunk2',
           'iwebadmin',
    );
    ```

    NOTE: the iwebadmin plugin will not work for postmaster account or IndiMail users having QA_ADMIN privileges. man vmoduser(1)
    This file should have read permission for apache group

    ```
    $ sudo chown root:apache /etc/roundcube/config.inc.php
    $ sudo chmod 640 /etc/roundcube/config.inc.php
    ```

    For markasjunk2 to work you need to set permission for apache to write /etc/indimail/spamignore

    ```
    $ sudo chown apache:indimail /etc/indimail/spamignore
    $ sudo chmod 644 /etc/indimail/spamignore
    ```

4. Edit the lines in /etc/roundcube/defaults.inc.php

   ```
   $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';$config['imap_auth_type'] = 'LOGIN';
   $config['smtp_auth_type'] = 'LOGIN';
   ```

   This file should have read permission for apache group

   ```
   $ sudo chown root:apache /etc/roundcube/defaults.inc.php
   $ sudo chmod 640 /etc/roundcube/defaults.inc.php
   ```

5. Change iwebadmin path in /usr/share/roundcubemail/iwebadmin/config.inc.php
   `$rcmail_config['iwebadmin_path'] = 'http://127.0.0.1/cgi-bin/iwebadmin';`

6. Change sauserprefs\_db\_dsnw in /usr/share/roundcubemail/sauserprefs/config.inc.php
   `$rcmail_config['sauserprefs_db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';`

7. Restore indimail plugins for roundcube

   ```
   $ sudo yum install ircube
   or
   $ cd /tmp
   $ wget http://downloads.sourceforge.net/indimail/indimail-roundcube-1.0.tar.gz # This file
   $ cd /
   $ sudo tar xvfz /tmp/indimail-roundcube-1.0.tar.gz usr/share/roundcubemail/plugins
   $ /usr/bin/mysql -u mysql -p RoundCube_db < /usr/share/roundcubemail/sauserprefs/sauserprefs.sql
   ```

8. change pdo\_mysql.default\_socket /etc/php.ini
   For some reason `pdo_mysql` uses wrong mysql socket on some systems. Uses `/var/lib/mysql/mysql.sock` instead of `/var/run/mysqld/mysqld.sock`. You need to edit the file `/etc/php.ini` and define `pdo_mysql.default_socket`

   `pdo_mysql.default_socket= /var/run/mysqld/mysqld.sock`

   You can verify if the path has been correctly entered by executing the below command. The command should return without any error

   ```
   $ php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"
   ```

9. HTTPD config
   i. Edit file /etc/httpd/conf.d/roundcubemail.conf and edit the following lines

   ```
   #
   # Round Cube Webmail is a browser-based multilingual IMAP client
   #Alias /indimail /usr/share/roundcubemail
   # Define who can access the Webmail
   # You can enlarge permissions once configured
   <Directory /usr/share/roundcubemail/>
       <IfModule mod_authz_core.c>
           # Apache 2.4
           Require ip 127.0.0.1
           Require all granted
           Require local
       </IfModule>
       <IfModule !mod_authz_core.c>
           # Apache 2.2
           Order Deny,Allow
           Deny from all
           Allow from 127.0.0.1
           Allow from ::1
       </IfModule>
   </Directory>
   ```

   This file should be owned by root

   ```
   $ sudo chown root:root /etc/httpd/conf.d/roundcubemail.conf
   $ sudo chmod 644 /etc/httpd/conf.d/roundcubemail.conf
   ```

   ii. Restart httpd

   `$ sudo service httpd restart`

10. Login to webmail at http://localhost/indimail
    SSL / TLS Version

    1. Install RoundCube. On older systems, use the yum command
       `$ sudo dnf -y install roundcubemail php-mysqlnd`

    2. Connect to MySQL using a privileged user. IndiMail installation creates a privileged mysql user 'mysql'. It does not have the user 'root'. Look at the variable PRIV\_PASS in /usr/sbin/svctool to know the password.

       ```
       $ /usr/bin/mysql -u mysql -p mysql
       MySQL> create database RoundCube_db;
       MySQL> create user roundcube identified by 'subscribed';
       MySQL> GRANT ALL PRIVILEGES on RoundCube_db.* to roundcube;
       MySQL> FLUSH PRIVILEGES;
       MySQL> QUIT;
       $ /usr/bin/mysql -u mysql -p RoundCube_db < /usr/share/roundcubemail/SQL/mysql.initial.sql
       ```

    3. Copy /etc/roundcube/config.inc.php.sample to /etc/roundcube.inc.php

       `$ sudo cp /etc/roundcube/config.inc.php.sample /etc/roundcubemail/config.inc.php`

       Edit the lines in /etc/roundcube/config.inc.php

       ```
       $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
       $config['default_host'] = 'ssl://indimail.org';
       $config['smtp_server'] = 'localhost';
       $config['smtp_port'] = 587;
       $config['smtp_user'] = '%u';
       $config['smtp_pass'] = '%p';
       $config['support_url'] = 'http://indimail.sourceforge.net';
       $config['product_name'] = 'IndiMail Webmail';
       $config['plugins'] = array(
              'archive',
              'sauserprefs',
              'markasjunk2',
              'iwebadmin',
       );
       ```
       NOTE: the iwebadmin plugin will not work for postmaster account or IndiMail users having QA_ADMIN privileges. man vmoduser(1)
       This file should have read permissions for apache group

       ```
       $ sudo chown root:apache /etc/roundcube/config.inc.php
       $ sudo chmod 640 /etc/roundcube/config.inc.php
       ```

    4. Edit the lines in /etc/roundcube/defaults.inc.php i.e.

        ```
       $config['db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
       $config['default_host'] = 'ssl://indimail.org';
       $config['default_port'] = 993;
       $config['imap_conn_options'] = array(
         'ssl'         => array(
           'verify_peer'       => false,
           'verify_peer_name'  => false,
         ),
       );
       $config['imap_auth_type'] = 'LOGIN';
       $config['smtp_auth_type'] = 'LOGIN';
       $config['force_https'] = true;
       $config['product_name'] = 'IndiMail Webmail';
       $config['useragent'] = 'IndiMail Webmail/'.RCMAIL_VERSION;
       ```

       This file should have read permission for apache group$config['force_https'] = true;

       ```
       $ sudo chown root:apache /etc/roundcube/defaults.inc.php
       $ sudo chmod 640 /etc/roundcube/defaults.inc.php
       ```

    5. Change iwebadmin path in /usr/share/roundcubemail/iwebadmin/config.inc.php
       `$rcmail_config['iwebadmin_path'] = 'https://127.0.0.1/cgi-bin/iwebadmin';`

    6. Change `sauserprefs_db_dsnw` in /usr/share/roundcubemail/sauserprefs/config.inc.php

       ```
       $rcmail_config['sauserprefs_db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';
       ```

    7. Restore indimail plugins for roundcube

       ```
       $ cd /tmp
       $ wget http://downloads.sourceforge.net/indimail/indimail-roundcube-ssl-1.0.tar.gz # This file
       $ cd /
       $ sudo tar xvfz /tmp/indimail-roundcube-ssl-1.0.tar.gz usr/share/roundcubemail/plugins
       $ /usr/bin/mysql -u mysql -p RoundCube_db < /usr/share/roundcubemail/sauserprefs/sauserprefs.sql
       ```

    8. Change pdo\_mysql.default\_socket /etc/php.ini
       For some reason pdo_mysql uses wrong mysql socket on some systems. Uses /var/lib/mysql/mysql.sock instead of /var/run/mysqld/mysqld.sock. You need to edit the file /etc/php.ini and define pdo_mysql.default_socket

       `pdo_mysql.default_socket= /var/run/mysqld/mysqld.sock`

       You can verifiy if the path has been correctly entered by executing the below command. The command should return without any error

       ```
       php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"
       ```

    9. HTTPD config
       i. Edit file /etc/httpd/conf.d/roundcubemail.conf and edit the following lines

       ```
       #
       # Round Cube Webmail is a browser-based multilingual IMAP client
       #Alias /indimail /usr/share/roundcubemail
       # Define who can access the Webmail
       # You can enlarge permissions once configured
       <Directory /usr/share/roundcubemail/>
           <IfModule mod_authz_core.c>
               # Apache 2.4
               Require ip 127.0.0.1
               Require all granted
               Require local
           </IfModule>
           <IfModule !mod_authz_core.c>
               # Apache 2.2
               Order Deny,Allow
               Deny from all
               Allow from 127.0.0.1
               Allow from ::1
           </IfModule>
       </Directory>
       ```

       This file should be owned by root

       ```
       $ sudo chown root:root /etc/httpd/conf.d/roundcubemail.conf
       $ sudo chmod 644 /etc/httpd/conf.d/roundcubemail.conf
       ```

      ii. This is assuming you have already generated indimail cert after indimail installation. If not execute the following command. We will assume that your host is indimail.org

       ```
       $ sudo /usr/sbin/svctool --postmaster=postmaster@indimail.org –config=cert" --common_name=indimail.org
       ```

       Edit the file /etc/httpd/conf.d/ssl.conf i.e.

       ```
       ServerName indimail.org:443
       SSLCertificateFile /etc/indimail/certs/servercert.pem
       ```

       Now apache server needs access to servercert.pem. Add apache user to the qmail group. You can chose either of the below two options (Options 2 is less secure, as it gives httpd access to qmail files).

       * Option 1

       ```
       $ sudo chown indimail:apache /etc/indimail/certs/servercert.pem
       $ sudo chmod 640 /etc/indimail/certs/servercert.pem
       ```

       * Option 2

       `$ sudo usermod -aG qmail apache`

       Now you should see apache getting qmail group access

       ```
       $ grep "qmail:x:" /etc/group
       qmail:x:1002:qscand,apache
       ```

    iii. Edit file /etc/php.ini. For some funny reason, the cert needs to be mentioned. i.e.

       ```
       openssl.cafile=/etc/indimail/certs/servercert.pem
       openssl.capath=/etc/pki/tls/certs
       ```

       Run the following command to get the cert locations. [ini_cafile] should point to servercert.pem location.

       ```
       $ php -r "print_r(openssl_get_cert_locations());"
       Array
       (
         [default_cert_file] => /etc/pki/tls/cert.pem
         [default_cert_file_env] => SSL_CERT_FILE
         [default_cert_dir] => /etc/pki/tls/certs
         [default_cert_dir_env] => SSL_CERT_DIR
         [default_private_dir] => /etc/pki/tls/private
         [default_default_cert_area] => /etc/pki/tls
         [ini_cafile] => /etc/indimail/certs/servercert.pem
         [ini_capath] => /etc/pki/tls/certs
       )
       ```

    iv. Follow instructions to setup https

        https://wiki.centos.org/HowTos/Https

    v. Restart httpd

       `$ sudo service httpd restart`

    vi. It appears that in PHP 5.6.0, functions are now validating SSL certificates(in a variety of ways). First, it appears to fail for untrusted certificates (i.e. no matching CA trusted locally), and secondly, it appears to fail for mismatched hostnames in the request and certificate. Verify that php is using the correct certificate with proper CN. Use the program testssl.php download from the location you downloaded this README/INSTALL file. In Step 9ii you created a certificate with common_name as indimail.org. Use the same host that you gave when creating the certificate.

       ```
       $ php ./testssl.php indimail.org
       Success
       ```

10. Login to webmail
    * edit /etc/hosts and edit the line for localhost i.e.
    127.0.0.1 localhost indimail.org
    * Restart httpd
    $ sudo service httpd restart
    * Login to webmail at https://indimail.org/indimail

NOTE: Replace indimail.org with domain that you have configured

# Setting up MySQL

This section assumes that you have already installed MySQL or MariaDB. There are few difference between them. Also remember that the MySQL developers have a habit of changing & breaking things. Anything written here could stop working few versions down the road. This document has been written with MySQL Version 5.7 and MariaDB 5.5

There are few things that IndiMail requires

1. A Database on which IndiMail has certain privileges. We will discuss this under the section ‘Database Initialization’
2. A consistent method of starting MySQL daemon mysqld. This can easily be achieved by having mysqld under supervise. We will discuss this under the section ‘MySQL Service’
3. Setting up MySQL users named mysql, indimail, admin and repl.
4. A MySQL config file which sets variables like the socket path. This should be named indimail.cnf. This can be created by running the svctool command. We will discuss this under the section ‘MySQL Configuration’
5. A control file which is used by all IndiMail client to access MySQL. This control file will have the MySQL host, username, password, port or socket path. This can be created using any text editor or using the echo command on the shell. We will discuss this under the section ‘MySQL Control File’

The role of the users discussed in point 3. above are

1. mysql to allow all administration access to MySQL database. This is equivalent to the MySQL root user created by default MySQL binary installation.
2. indimail to allow IndiMail programs to access MySQL. This user does not have access to any other database.
3. admin with reload and shutdown privileges on the MySQL database. This user can shutdown the MySQL database.
4. repl to with replication privileges. This allows a slave MySQL setup to replicate database from the MySQL master. This is needed only when you want to have a Master Slave setup.

## Storing Passwords

It is advisable not to store the MySQL passwords in scripts. You can use mysql\_config\_editor to store encrypted credentials in .mylogin.cnf file. This file is read by MySQL clients during startup, hence avoiding the need of passing passwords on the command line. Remember passing passwords on the command line is insecure. With the --password option, mysql\_config\_editor will prompt you for the password to store

```
$ mysql_config_editor set --login-path=admin --socket=/var/run/mysqld/mysqld.sock --user=mysql --password
Enter password:
```

Now to login to MySQL using mysql client you just need to executed

```
$ mysql –login-path=mysql
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 50
Server version: 5.7.20-log MySQL Community Server (GPL)

Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql>
```

You can create a similar login path for indimail user

```
$ mysql_config_editor set --login-path=indimail --socket=/var/run/mysqld/mysqld.sock –user=indimail --password
Enter password:
```

Now for carrying out routine queries for the MySQL indimail database you can pass the option –login-path=indimail to /usr/bin/mysql client.

## Database Initialization
**Using svctool**

svctool automates the process of MySQL db creation as required for IndiMail. You just need to run the following command

```
$ sudo /usr/sbin/svctool --config=mysqldb --mysqlPrefix=/usr \
     --databasedir=/var/indimail/mysqldb --default-domain=`uname -n` \
     --base_path=/home/mail
```

The above command will create and initialize a MySQL database for first time use by IndiMail. If the directory /var/indimail/mysqldb exists, svctool will do nothing. Else, svctool will also create the four users mysql, indimail, admin and repl. If you want to setup MySQL manually or want to use an existing MySQL instance/setup or want to do a manual initialization of MySQL database, you will have to create these users manually. The creation for these users will be discussed in Section 3.

**Manual Initialization**

In the examples shown here, the server is going to run under the user ID of the mysql login account. This assumes that such an account exists. Either create the account if it does not exist, or substitute the name of a different existing login account that you plan to use for running the server.

Create a directory whose location can be provided to the secure\_file\_priv system variable, which limits import/export operations to that specific directory:

```
$ mkdir -p /var/indimail/mysqldb/data
$ mkdir -p var/indimail/mysqldb/logs
$ chown -R mysql:mysql /var/indimail/mysqldb/data
$ chmod 750 /var/indimail/mysqldb/data
```

Initialize the data directory, including the mysql database containing the initial MySQL grant tables that determine how users are permitted to connect to the server.

Typically, data directory initialization need be done only after you first installed MySQL. If you are upgrading an existing installation, you should run mysql\_upgrade instead (see mysql\_upgrade — Check and Upgrade MySQL Tables). However, the command that initializes the data directory does not overwrite any existing privilege tables, so it should be safe to run in any circumstances. Use the server to initialize the data directory; for example:

**For MySQL from Oracle**

```
$ sudo mysqld --initialize --datadir=/var/indimail/mysqldb/data –user=mysql \
  --log-error=/var/indimail/mysqldb/logs/mysqld.log
```

**For MySQL from MariaDB**

```
$ sudo mysql_install_db –datadir=/var/indimail/mysqldb/data –user=mysql
  --log-error=/var/indimail/mysqldb/logs/mysqld.log
```

You can check if the database has been created by using the ls command. You should be able to see two directories mysql and indimail under /var/indimail/mysqldb/data, indicating creation of two databases named mysql and indimail.

```
$ ls -l /var/indimail/mysqldb/data
total 176228
-rw-rw----. 1 mysql mysql    16384 Dec 20 10:00 aria_log.00000001
-rw-rw----. 1 mysql mysql       52 Dec 20 10:00 aria_log_control
-rw-r-----. 1 mysql mysql       56 Mar 24  2017 auto.cnf
-rw-------. 1 mysql mysql     1675 Mar 24  2017 ca-key.pem
-rw-r--r--. 1 mysql mysql     1074 Mar 24  2017 ca.pem
-rw-r--r--. 1 mysql mysql     1078 Mar 24  2017 client-cert.pem
-rw-------. 1 mysql mysql     1675 Mar 24  2017 client-key.pem
drwxr-x---. 2 mysql mysql     4096 Mar 24  2017 ezmlm
-rw-r-----. 1 mysql mysql      726 Feb  8 07:59 ib_buffer_pool
-rw-r-----. 1 mysql mysql 79691776 Feb  8 07:59 ibdata1
-rw-r-----. 1 mysql mysql 50331648 Feb  8 07:59 ib_logfile0
-rw-r-----. 1 mysql mysql 50331648 Mar 24  2017 ib_logfile1
drwxr-x---. 2 mysql mysql     4096 Feb  3 18:02 indimail
-rw-rw----. 1 mysql mysql        0 Dec 17 18:52 multi-master.info
drwxr-x---. 2 mysql mysql     4096 Feb  3 18:02 mysql
drwxr-x---. 2 mysql mysql     4096 Mar 24  2017 performance_schema
-rw-------. 1 mysql mysql     1675 Mar 24  2017 private_key.pem
-rw-r--r--. 1 mysql mysql      451 Mar 24  2017 public_key.pem
-rw-r--r--. 1 mysql mysql     1078 Mar 24  2017 server-cert.pem
-rw-------. 1 mysql mysql     1675 Mar 24  2017 server-key.pem
drwxr-x---. 2 mysql mysql    12288 Mar 24  2017 sys
```

You now need to check the log /var/indimail/mysqldb/logs/mysqld.log. The last line in this log will give you the password for the root user. This user will have all privileges and we will use this to create the user indimail and grant it privileges to access the indimail database. Note down this password.

```
$ cat /var/indimail/mysqldb/logs/mysqld.log
2018-02-07T03:34:41.509241Z 0 [Warning] Changed limits: max_open_files: 1024 (requested 5000)
2018-02-07T03:34:41.509393Z 0 [Warning] Changed limits: table_open_cache: 431 (requested 2000)
2018-02-07T03:34:43.457211Z 0 [Warning] InnoDB: New log files created, LSN=45790
2018-02-07T03:34:43.712425Z 0 [Warning] InnoDB: Creating foreign key constraint system tables.
2018-02-07T03:34:43.801374Z 0 [Warning] No existing UUID has been found, so we assume that this is the first time that this server has been started. Generating a new UUID: d65b11f7-0bb7-11e8-b137-b8763fc3c7f1.
2018-02-07T03:34:43.811013Z 0 [Warning] Gtid table is not ready to be used. Table 'mysql.gtid_executed' cannot be opened.
2018-02-07T03:34:43.811968Z 1 [Note] A temporary password is generated for root@localhost: po!aj=Zi(8+b
```

If you want the server to be able to deploy with automatic support for secure connections, use the mysql_ssl_rsa_setup utility to create default SSL and RSA files:

`$ sudo bin/mysql_ssl_rsa_setup -uid=mysql --datadir=/var/ndimail/mysqldb/data`

For more information, see mysql\_ssl\_rsa\_setup — Create SSL/RSA Files.
Now using the password obtained from var/log/mysqld.log, we will connect to MySQL and create users

1. If the plugin directory (the directory named by the plugin\_dir system variable) is writable by the server, it may be possible for a user to write executable code to a file in the directory using SELECT ... INTO DUMPFILE. This can be prevented by making the plugin directory read only to the server or by setting the secure\_file\_priv system variable at server startup to a directory where SELECT writes can be performed safely. (For example, set it to the mysql-files directory created earlier.)
2. To specify options that the MySQL server should use at startup, put them in a /etc/my.cnf or /etc/mysql/my.cnf file. You can use such a file to set, for example, the secure\_file\_priv system variable. See Server Configuration Defaults. If you do not do this, the server starts with its default settings. You should set the datadir variable to /var/indimail/mysqldb/data in my.cnf.
3. If you want MySQL to start automatically when you boot your machine, see Section 9.5, “Starting and Stopping MySQL Automatically”.

Data directory initialization creates time zone tables in the mysql database but does not populate them. To do so, use the instructions in MySQL Server Time Zone Support.

## MySQL Startup

If you have done a binary installation of MySQL (yum/dnf/apt-get or RPM/DEB installation), the post install scripts should have installed MySQL to be started during boot. The preferred method for IndiMail is using supervise, though not mandatory.

**System Default**

If you decide to use have MySQL daemon started as setup by the MySQL package installation, you need to modify /etc/my.cnf or /etc/mysql/mysql.cnf and change datadir to /var/indimail/mysqldb/data.
You can start mysqld my issuing the command

`$ sudo service mysqld start`

NOTE: On some system you might have to replace mysqld with mysql while issuing the above command. After you do this, you will see mysqld_safe in the process list.
If you have installed MariaDB there is no confusion to startup MySQL. The command will be

`$ sudo service mariadb start`

**MySQL startup under supervise**

This is the preferred method. This allows for MySQL to be started automatically when IndiMail starts up and shutdown when IndiMail shuts down. The supervise service can be created by running the svctool command.

```
$ sudo /usr/sbin/svctool --mysql=3306 --servicedir=/service \
    --mysqlPrefix=/usr --databasedir=/var/indimail/mysqldb \
    --config=/etc/indimail/indimail.cnf --default-domain=`uname -n`
```

Once you do this, you will see the directory /service/mysql.3306

```
$ /bin/ls -lR /service/mysql.3306
/service/mysql.3306:
total 20
drwxr-xr-x. 3 root root 4096 Feb  3 17:46 log
-rwxr-xr-x. 1 root root 1345 Feb  5 12:57 run
-r-x------. 1 root root  423 Feb  5 12:57 shutdown
dr-x------. 2 root root 4096 Feb  3 17:43 variables

/service/mysql.3306/log:
total 8
-rwxr-xr-x. 1 root root  423 Feb  5 12:57 run
```

NOTE: You don’t need the mysql supervise service if you use the system installation default. If you have created this service, you can have supervise not start up mysqld by issuing the following command

`$ sudo touch /service/mysql.3306/down`

If you are doing to have supervise startup MySQL, then you need to disable the service from getting started up at boot. On modern systems, the command will be

```
$ sudo systemctl disable mysqld.service # MySQL from Oracle (it could also be mysql.service
$ sudo systemctl disable mariadb.service # MariaDB
```

## Creating MySQL Users
If you have created the MySQL database manually, you will need to create the uses using the MySQL client /usr/bin/mysql. In Section 1, we noted down the MySQL password for the root user. You would have started MySQL service in Section 2. We will now connect to the database and create users using the following set of commands.

First step is to connect using the password noted in Section 1 and change the password

```
$ mysqladmin -u root -p password
Enter password:
New password:
Confirm new password:
```
Warning: Since password will be sent to server in plain text, use ssl connection to ensure password safety.

NOTE: In case of MariaDB, the root password is not set and you will have to just press the Return key for the “Enter password:” prompt.
Now you can connect to MySQL using the new password set for user root above.

```
$ mysql -u root -p
Enter password:
Welcome to the MySQL monitor.  Commands end with ; or \g.
Your MySQL connection id is 10
Server version: 5.7.20-log MySQL Community Server (GPL)

Copyright (c) 2000, 2017, Oracle and/or its affiliates. All rights reserved.

Oracle is a registered trademark of Oracle Corporation and/or its
affiliates. Other names may be trademarks of their respective
owners.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

mysql> CREATE USER indimail identified by 'ssh-1.5-';
mysql> CREATE USER mysql    identified by '4-57343-';
mysql> CREATE USER admin    identified by 'benhur20';
mysql> CREATE USER repl     identified by 'slaveserver';
mysql> GRANT ALL on *.* to 'mysql';
mysql> GRANT SELECT,CREATE,ALTER,INDEX,INSERT,UPDATE,DELETE, \
        ->  CREATE TEMPORARY TABLES, \
         -> LOCK TABLES ON indimail.* to 'indimail';
mysql> GRANT RELOAD,SHUTDOWN,PROCESS on *.* to admin;
mysql> GRANT REPLICATION SLAVE on *.* to repl;
mysql> CREATE DATABASE indimail;
mysql> use mysql;
mysql> DELETE from user  where host=’localhost’ and user=’root’;
mysql> FLUSH PRIVILEGES;
```

NOTE: for MariaDB, you will have to execute few additional MySQL statements. The statements below also achieves what the script /bin/mysql\_secure\_installation does for MariaDB installations.

```
$ mysql -u root -p
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 6
Server version: 5.5.59-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> use mysql;
MariaDB [(none)]> DELETE from user where host = ‘localhost’ and user = 'mysql';
MariaDB [(none)]> DROP DATABASE test;
MariaDB [(none)]> FLUSH PRIVILEGES;
```

NOTE: After using the above commands, you will no longer have the MySQL user root. Instead you will use the user mysql for all privileged operations.

## Creating MySQL Configuration

This involves creating indimail.cnf in /etc/indimail. This file looks like this

```
$ cat /etc/indimail/indimail.cnf
[client]
port      = 3306
socket    = /var/run/mysqld/mysqld.sock

[mysqld]
#
# * Basic Settings
#

#
# * IMPORTANT
#   If you make changes to these settings and your system uses apparmor, you may
#   also need to also adjust /etc/apparmor.d/usr.sbin.mysqld.
#

sql_mode="NO_ENGINE_SUBSTITUTION,NO_ZERO_DATE,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_AUTO_CREATE_USER,STRICT_ALL_TABLES"
require_secure_transport = ON
explicit_defaults_for_timestamp=TRUE
user     = mysql
socket   = /var/run/mysqld/mysqld.sock
port     = 3306
basedir  = /usr
datadir  = /var/indimail/mysqldb

[inlookup]
#The number of seconds the server waits for activity on an
#interactive connection before closing it. An interactive client is
#defined as a client that uses the 'CLIENT_INTERACTIVE' option to connect
interactive_timeout=28880

#The number of seconds to wait for more data from a connection
#before aborting the read. This timeout applies only to TCP/IP
#connections, not to connections made via Unix socket files, named
#pipes, or shared memory.
net_read_timeout=5

#The number of seconds to wait for a block to be written to a
#connection before aborting the write. This timeout applies only to
#TCP/IP connections, not to connections made via Unix socket files,
#named pipes, or shared memory.
net_write_timeout=5

#The number of seconds the server waits for activity on a
#non-interactive connection before closing it. This timeout applies
#only to TCP/IP and Unix socket file connections, not to
#connections made via named pipes, or shared memory.
wait_timeout=28800
```

You can use your favourite editor to create the above file or create it using the svctool commands. Also you need to have a link to this file in /etc/mysql

```
$ sudo /usr/sbin/svctool --config=mysql --mysqlPrefix=/usr \
    --mysqlport=3306 –mysqlsocket=/var/run/mysqld/mysqld.sock
$ ln -s /etc/indmail/indimail.cnf /etc/mysql/indimail.cnf
```

For MariaDB you might find a directory etc/my.cnf.d. In that case create a link to /etc/indimail/indimail.cnf in that directory

`$ ln -s /etc/indmail/indimail.cnf /etc/my.cnf.d/indimail.cnf`

## MySQL Control File

Once you have MySQL up and running, you need to tell IndiMail how to use it. This is done by having the config file /etc/indimail/control/host.mysql. The format for this file is

```
mysql_host:mysql_user:mysql_pass:mysql_socket
or
mysql_host:mysql_user:mysql_pass:mysql_port
```

You will use the first syntax when you have MySQL and IndiMail installed on the same host. You will use the second form when you have MySQL installed on a host different from the host on which you have installed IndiMail. The user mysql_user needs to have certain privileges, which we will discuss under the section MySQL Privileges.

```
$ sudo /bin/sh -c “localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock >
    /etc/indimail/control/host.mysql
```

Now we have everything ready. We can test the connection and also create all default IndiMail tables by running the install_tables command.

```
$ /usr/sbin/install_tables
created table indimail on local
created table indibak on local
created table relay on local
created table atrn_map on local
created table bulkmail on local
created table fstab on local
created table ip_alias_map on local
created table lastauth on local
created table userquota on local
created table valias on local
created table vfilter on local
created table mailing_list on local
created table vlimits on local
created table vlog on local
skipped table aliasdomain on master
skipped table dbinfo on master
skipped table fstab on master
skipped table host_table on master
skipped table mgmtaccess on master
skipped table smtp_port on master
skipped table vpriv on master
skipped table spam on master
skipped table badmailfrom on master
skipped table badrcptto on master
skipped table spamdb on master
```

## Configuring MySQL/MariaDB to use SSL/TLS

MySQL/MariaDB can encrypt the connections between itself and its clients. To do that you need to create certificates. There are differences betweeen how you configure MySQL server and MariaDB server for SSL/TLS

**MySQL community server**

`$ sudo mysql_ssl_rsa_setup –user=mysql –datadir=/var/indimail/mysqldb/data`

You will now find the following files in /var/indimail/mysqldb/data.

ca.pem, ca-key.pem, client-cert.pem, client-key.pem, server-cert.pem, server-key.pem, public\_key.pem, private\_key.pem. If you are going to have multiple clients who need to connect to this server, you need to copy ca.pem and ca-key.pem to /var/indimail/mysqldb/data directory on the client host.

Setting up the mysql community server for encrypting communication is simple. If you want to enforce the connection you need to have the following lines in my.cnf under the [mysqld] section

```
[mysqld]
ssl-ca=ca.pem
ssl-cert=server-cert.pem
ssl-key=server-key.pem
require-secure-transport=ON
```

**MariaDB server**

```
$ sudo /usr/sbin/svctool --config=ssl_rsa –capath=/var/indimail/mysqldb/ssl
    --certdir=/var/indimail/mysqldb/ssl
```

You will now find the following files in /var/indimail/mysqldb/ssl.
ca.pem, ca-key.pem, client-cert.pem, client-key.pem, server-cert.pem, server-key.pem, public\_key.pem, private\_key.pem. If you are going to have multiple clients who need to connect to this server, you need to copy ca.pem and ca-key.pem to /var/indimail/mysqldb/ssl directory on the client host.

Unlike mysql-community server, for MariaDB server, you need to mention the certificates in my.cnf like this

```
[mysqld]
#
# * Basic Settings
#

#
# * IMPORTANT
#   If you make changes to these settings and your system uses apparmor, you may
#   also need to also adjust /etc/apparmor.d/usr.sbin.mysqld.
#

sql_mode="NO_ENGINE_SUBSTITUTION,STRICT_ALL_TABLES"

# MySQL Server SSL configuration
# Securing the Database with ssl option and certificates
# There is no control over the protocol level used.
# mariadb will use TLSv1.0 or better.
ssl
ssl-ca=/var/indimail/mysqldb/ssl/ca.pem
ssl-cert=/var/indimail/mysqldb/ssl/server-cert.pem
ssl-key=/var/indimail/mysqldb/ssl/server-key.pem

explicit-defaults-for-timestamp=TRUE
user     = mysql
socket   = /var/run/mysqld/mysqld.sock
port     = 3306
basedir  = /usr
datadir  = /var/indimail/mysqldb/data
character-set-client-handshake = FALSE
character-set-server = utf8mb4
collation-server = utf8mb4_unicode_ci

#Description: If set to 1, LOCAL is supported for LOAD DATA INFILE statements.
#If set to 0 (default), usually for security reasons, attempts to perform a
#LOAD DATA LOCAL will fail with an error message.
# local-infile = 1
```

MySQL community Client
You first need to copy ca.pem and ca-key.pem to the directory /var/indimail/mysqldb/data. Then run the following commands

```
$ sudo /usr/sbin/svctool --config=ssl_rsa –capath=/var/indimail/mysqldb/data
    --certdir=/var/indimail/mysqldb/data
```

MariaDB Client
You first need to copy ca.pem and ca-key.pem to the directory /var/indimail/mysqldb/ssl. Then run the following commands

```
$ sudo /usr/sbin/svctool --config=ssl_rsa –capath=/var/indimail/mysqldb/ssl
    --certdir=/var/indimail/mysqldb/ssl
```

You also need to specify the client certificates in my.cnf like this

```
[client]
port      = 3306
socket    = /var/run/mysqld/mysqld.sock

# MySQL Client SSL configuration
ssl-ca=/var/indimail/mysqldb/ssl/ca.pem
ssl-cert=/var/indimail/mysqldb/ssl/client-cert.pem
ssl-key=/var/indimail/mysqldb/ssl/client-key.pem
# This option is disabled by default
#ssl-verify-server-cert
```

## Configure MySQL/MariaDB access for svctool

We learned how to setup and use MySQL/Mariadb database with indimail. The general purpose configuration tool, <b>svctool</b>, which requires the following variables to access the MySQL database

Variable | Purpose
-------- | --------------------
MYSQL_PASS|Password for accessing <b>indimail</b> database in MySQL
PRIV_PASS|Password for accessing <b>mysql</b> database in MySQL. This is the MySQL superuser password.
ADMIN_PASS|Password required by <b>adminclient</b> program to execute remote commands
TMPDIR|Temporary directory used by <b>svctool</b>

You can have these stored as /etc/indimail/svctool.cnf owned by root and with no read/write access to group and others. e.g.

```
$ ls -l /etc/indimail/svctool.cnf 
-r-------- 1 root root 88 Nov  2  2018 /etc/indimail/svctool.cnf
$ sudo cat /etc/indimail/svctool.cnf
MYSQL_PASS="ssh-1.5-"
PRIV_PASS="4-57343-"
ADMIN_PASS="benhur20"
TMPDIR="/tmp/indimail"
```

# Using Docker Engine to Run IndiMail / IndiMail-MTA

IndiMail now has docker images. You can read about installing Docker [here](https://docs.docker.com/engine/install/ "Docker Installatoin"). Once you have installed docker-engine, you need to start it. Typically it would be

`$ sudo service docker start`

To avoid having to use sudo when you use the docker command, create a Unix group called docker and add users to it. When the docker daemon starts, it makes the ownership of the Unix socket read/writable by the docker group.
NOTE: Warning: The docker group is equivalent to the root user; For details on how this impacts security in your system, see [Docker Daemon attack surface](https://docs.docker.com/engine/security/security/#docker-daemon-attack-surface)

```
$ sudo groupadd docker
$ sudo usermod -aG docker your_username
```
Log out and login again to ensure your user is running with the correct permissions. You can run the unix id command to confirm that you have the docker group privileges. e.g.

```
$ id -a
uid=1000(mbhangui) gid=1000(mbhangui) groups=1000(mbhangui),10(wheel),545(docker) context=unconfined\_u:unconfined\_r:unconfined\_t:s0-s0:c0.c1023
```

Now we need to pull the docker image for IndiMail. use the **docker pull** command. The values for tag can be fedora-23, centos7, centos8, debian9, debian10, xenial, bionic, focal, fc32, Leap15.2, Tumbleweed, etc. If your favourite OS is missing, let me know. You can find the list of all images here.

`$ docker pull cprogrammer/indimail:tag`

(for indimail-mta image, execute docker pull cprogrammer/indimail-mta:tag
You can now list the docker image by executing the **docker images** command.

```
$ docker images
REPOSITORY                 TAG                 IMAGE ID            CREATED             SIZE
cprogrammer/indimail       fedora-23           a02e6014a67b        53 minutes ago      1.774 GB
```

Now let us run a container with this image using the image id a02e6014a67b listed above by running the **docker run** command. The **--privileged** flag gives all capabilities to the container, and it also lifts all the limitations enforced by the device cgroup controller. In other words, the container can then do almost everything that the host can do. This flag exists to allow special use-cases, like running Docker within Docker. In our case, I want the systemctl command to work and the container run like a normal host.


I have now figured out the you don't require the --privileged flag. This flag gives the container access to the host's systemd. A better way is to add SYS\_ADMIN capability

```
$ docker run -ti --cap-add=SYS\_ADMIN -e "container-docker" -v /sys/fs/cgroup:/sys/fs/cgroup:ro a02e6014a67b /sbin/init
```

The above will start a fully functional Fedora 23 OS with IndiMail, MySQL, sshd, httpd services up and running. We can list the running container by running the docker ps command

```
$ docker ps
CONTAINER ID        IMAGE               COMMAND             CREATED             STATUS              PORTS               NAMES
fd09c7ca75be        a02e6014a67b        "/sbin/init"        38 seconds ago      Up 37 seconds                           desperate_jones
```

We now have a running container and can attach to it and use it like any functional host. Run the docker exec command. The **-ti** option attaches a pseudo terminal and makes the session interactive.

```
$ docker exec -ti fd09c7ca75be /bin/bash --login
#
# /usr/bin/svstat /service/\*
/service/fetchmail: down 10036 seconds spid 45058 
/service/greylist.1999: up 10036 seconds pid 45102 
/service/indisrvr.4000: up 10036 seconds pid 45090 
/service/inlookup.infifo: up 10006 seconds pid 46146 
/service/mrtg: up 10036 seconds pid 45125 
/service/mysql.3306: up 10036 seconds pid 45148 
/service/proxy-imapd.4143: up 10036 seconds pid 45128 
/service/proxy-imapd-ssl.9143: up 10036 seconds pid 45141 
/service/proxy-pop3d.4110: up 10036 seconds pid 45147 
/service/proxy-pop3d-ssl.9110: up 10036 seconds pid 45120 
/service/pwdlookup: up 10006 seconds pid 46148 
/service/qmail-daned.1998: up 10036 seconds pid 45067 
/service/qmail-imapd.143: up 10036 seconds pid 45064 
/service/qmail-imapd-ssl.993: up 10036 seconds pid 45072 
/service/qmail-logfifo: up 10036 seconds pid 45091 
/service/qmail-pop3d.110: up 10036 seconds pid 45104 
/service/qmail-pop3d-ssl.995: up 10036 seconds pid 45114 
/service/qmail-poppass.106: up 10036 seconds pid 45081 
/service/qmail-qmqpd.628: down 10036 seconds spid 45007 
/service/qmail-qmtpd.209: up 10036 seconds pid 45107 
/service/qmail-send.25: up 10036 seconds pid 45131 
/service/qmail-smtpd.25: up 10036 seconds pid 45066 
/service/qmail-smtpd.366: up 10036 seconds pid 45065 
/service/qmail-smtpd.465: up 10036 seconds pid 45124 
/service/qmail-smtpd.587: up 10036 seconds pid 45136 
/service/qscanq: up 10036 seconds pid 45096 
/service/udplogger.3000: up 10036 seconds pid 45150 
```

You now have a fully functional mail server with a pre-configured virtual domain indimail.org and a pre-configured virtual user testuser01@indimail.org. You can use IMAP/POP3/SMTP to your heart's content. If not satisfied, try out the ssl enabled services IMAPS/POP3S/SMTPS or STARTTLS command. If still not satisfied, read the man pages. You can stop the container by executing the docker stop command.

`$ docker stop fd09c7ca75be`

You can make your changes to the container and commit changes by using the docker commit command. Learning how to use docker is not difficult. Just follow the Docker Documentation. If you are lazy like me, just read the Getting Started guide.
I am also a newbie as far as docker is concerned. Do let me know your experience with network settings and other advanced docker topics, that you may be familiar with. Do send few bottles of beer my way if you can.

NOTE:
Currently IndiMail supports both docker and podman. Both commands are interchangeble. There is much that has happened since this chapter was written in Apr 2016. You should be actually reading [this](https://github.com/mbhangui/indimail-docker/blob/master/README.md "Docker README ").

# Installation & Repositories

## Installing Indimail using DNF/YUM/APT Repository

You can get binary RPM / Debian packages at

* [Stable](http://download.opensuse.org/repositories/home:/indimail/ "Stable Build Repository")
* [Experimental](http://download.opensuse.org/repositories/home:/mbhangui/ "Experimental Build Repository")
* [copr Releases](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable](https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail "Stable Build Repository")
* [Experimental](https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail "Experimental Build Repository")
* [copr Releases](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail). The copr repository can be enabled by running the command 
  `$ sudo dnf copr enable cprogrammer/indimail`

```
Currently, the list of supported distributions for IndiMail is

    * Arch Linux

    * SUSE
          o openSUSE_Leap_15.3
          o openSUSE_Leap_15.4
          o openSUSE_Tumbleweed
          o SUSE Linux Enterprise 12
          o SUSE Linux Enterprise 12 SP1
          o SUSE Linux Enterprise 12 SP2
          o SUSE Linux Enterprise 12 SP3
          o SUSE Linux Enterprise 12 SP4
          o SUSE Linux Enterprise 12 SP5
          o SUSE Linux Enterprise 15
          o SUSE Linux Enterprise 15 SP1
          o SUSE Linux Enterprise 15 SP2
          o SUSE Linux Enterprise 15 SP3
          o SUSE Linux Enterprise 15 SP4

    * Red Hat
          o Fedora 37
          o Fedora 38
          o Red Hat Enterprise Linux 7
          o Red Hat Enterprise Linux 8 +
          o Red Hat Enterprise Linux 9 +
          o EPEL 8 +
          o EPEL 9 +
          o Scientific Linux 7
          o CentOS 7
          o CentOS 8
          o CentOS 8 Stream
          o CentOS 9 Stream
          o RockyLinux 8
          o RockyLinux 9
          o OracleLinux 8
          o OracleLinux 9
          o AlmaLinux 8
          o AlmaLinux 9

          +: Some of the above Red Hat flavoured distributions are available
             only on copr (RHEL 8, RHEL9, EPEL9, EPEL9)

    * Debian
          o Debian 10.0
          o Debian 11.0

    * Ubuntu
          o Ubuntu 18.04
          o Ubuntu 20.04
          o Ubuntu 22.04
```

## Binary Builds on openSUSE Build Service and Copr

**[Build Status on](https://build.opensuse.org/project/monitor/home:mbhangui) [Open Build Service](https://build.opensuse.org/project/show/home:mbhangui)**

[![indimail-mta](https://build.opensuse.org/projects/home:mbhangui/packages/indimail-mta/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail-mta)
[![daemontools](https://build.opensuse.org/projects/home:mbhangui/packages/daemontools/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/daemontools)
[![ucspi-tcp](https://build.opensuse.org/projects/home:mbhangui/packages/ucspi-tcp/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/ucspi-tcp)
[![ezmlm-idx](https://build.opensuse.org/projects/home:mbhangui/packages/ezmlm-idx/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/ezmlm-idx)
[![tinydnssec](https://build.opensuse.org/projects/home:mbhangui/packages/tinydnssec/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/tinydnssec)
[![indimail](https://build.opensuse.org/projects/home:mbhangui/packages/indimail/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail)
[![indimail-access](https://build.opensuse.org/projects/home:mbhangui/packages/indimail-access/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail-access)
[![indimail-auth](https://build.opensuse.org/projects/home:mbhangui/packages/indimail-auth/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail-auth)
[![indimail-spamfilter](https://build.opensuse.org/projects/home:mbhangui/packages/indimail-spamfilter/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail-spamfilter)
[![bogofilter-wordlist](https://build.opensuse.org/projects/home:mbhangui/packages/bogofilter-wordlist/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/bogofilter-wordlist)
[![indimail-utils](https://build.opensuse.org/projects/home:mbhangui/packages/indimail-utils/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indimail-utils)
[![iwebadmin](https://build.opensuse.org/projects/home:mbhangui/packages/iwebadmin/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/iwebadmin)
[![ircube](https://build.opensuse.org/projects/home:mbhangui/packages/ircube/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/ircube)
[![logalert](https://build.opensuse.org/projects/home:mbhangui/packages/logalert/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/logalert)
[![procmail](https://build.opensuse.org/projects/home:mbhangui/packages/procmail/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/procmail)
[![indium](https://build.opensuse.org/projects/home:mbhangui/packages/indium/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/indium)
[![libqmail](https://build.opensuse.org/projects/home:mbhangui/packages/libqmail/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/libqmail)
[![libdkim](https://build.opensuse.org/projects/home:mbhangui/packages/libdkim/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/libdkim)
[![libsrs2](https://build.opensuse.org/projects/home:mbhangui/packages/libsrs2/badge.svg?type=percent)](https://build.opensuse.org/package/show/home:mbhangui/libsrs2)

**[Build Status on](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/monitor/) [copr](https://copr.fedorainfracloud.org/coprs/)**

[![indimail-mta](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-mta/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-mta/)
[![daemontools](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/daemontools/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/daemontools/)
[![ucspi-tcp](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ucspi-tcp/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ucspi-tcp/)
[![ezmlm-idx](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ezmlm-idx/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ezmlm-idx/)
[![tinydnssec](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/tinydnssec/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/tinydnssec/)
[![indimail](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail/)
[![indimail-access](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-access/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-access/)
[![indimail-auth](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-auth/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-auth/)
[![indimail-spamfilter](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-spamfilter/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-spamfilter/)
[![bogofilter-wordlist](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/bogofilter-wordlist/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/bogofilter-wordlist/)
[![indimail-utils](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-utils/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indimail-utils/)
[![indium](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indium/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/indium/)
[![ircube](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ircube/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/ircube/)
[![iwebadmin](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/iwebadmin/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/iwebadmin/)
[![logalert](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/logalert/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/logalert/)
[![libqmail](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libqmail/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libqmail/)
[![libdkim](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libdkim/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libdkim/)
[![libsrs2](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libsrs2/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/cprogrammer/indimail/package/libsrs2/)

## Docker / Podman Repository

The [docker repository](https://github.com/mbhangui?tab=packages) can be used to pull docker/podman images for indimail/indimail-mta

For latest details refer to [README](https://github.com/mbhangui/indimail-docker/blob/master/README.md "Docker README ")

Earlier I used to provide docker / podman container images on [docker hub](https://hub.docker.com/u/cprogrammer) alone. But recently (Jul 25, 2021) docker, without any warning or communication, on the sly, has removed the ability to connect with a github account unless you pay them dollars every month - a questionable business practice. Lucky for me we have github. I now mirror the containers to [Github Container Registry](https://github.blog/2020-09-01-introducing-github-container-registry/). The containers are built and pushed to both docker and github container registry automatically using action scripts. You can also build your own images by referring to [README](https://github.com/mbhangui/indimail-docker/blob/master/README.md).

## GIT Repository

IndiMail has a git repository at [here](https://github.com/mbhangui/indimail-virtualdomains "IndiMail")

# Support Information

## IRC / Matrix

* [Matrix Invite Link #indimail:matrix.org](https://matrix.to/#/#indimail:matrix.org)
* IndiMail has an [IRC channel on libera](https://libera.chat/) #indimail-mta

The matrix room and libera.chat channel have been bridged so joining either one should be sufficient.

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support "Support"). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel "Devel Support"). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here]
(https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail "Announce")
4. Archive at [Google Groups](http://groups.google.com/group/indimail "Archive"). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686 "Project Trackerr") for IndiMail (Bugs, Feature Requests, Patches, Support Requests)

# Features

## Speed

IndiMail uses a modified [qmail](http://cr.yp.to/qmail.html "qmail") as the MTA. qmail's modular, lightweight design and sensible queue management provides IndiMail the speed to make it one of the fastest available message transfer agent.
IndiMail provides a multi-queue version of the original ''qmail-queue'' program. IndiMail uses multiple queues with each queue running its own <b>qmail-send</b>/todo-proc, <b>qmail-lspawn</b>, <b>qmail-rspawn</b> processes. This allows IndiMail to process mails faster than what can be provided by qmail. A process named qscheduler monitors these processes and restarts them if they go down, which hasn't yet been observed to go down.

Note: todo-proc is a dedicated todo processor whose basic working comes from the ext-todo patch for qmail by Claudio Jeker <jeker@n-r-g.com> and Andre Oppermann <opi@nrg4u.com>, (c) 1998,1999,2000,2001,2002 Internet Business Solutions Ltd.

## Setup

 * automatic adaptation to your UNIX variant
 * Linux, FreeBSD, OSX, SunOS, Solaris, and more
 * automatic per-host configuration - gnu autoconf
 * High degree of automation of configuration through ''svctool''
 * RPM/Debian packages for multiple Linux Distros. Binary packages follows Dave Sill's [LWQ](http://www.lifewithqmail.org/ "LWQ").

## Security

 * clear separation between addresses, files, and programs
 * minimization of setuid code (<b>qmail-queue</b>, qhspi, qscanq, systpass)
 * minimization of root code (qmail-start, <b>qmail-lspawn</b>)
 * five-way trust partitioning---security in depth
 * optional logging of one-way hashes, entire contents, etc. (EXTRAQUEUE, mailarchive control file)
 * virus scanning through clamav/qscanq/avg
 * Inbuilt virus scanner
 * sender/recipient accesslist, hostaccess using ''tcprules''.

## Message construction

 * RFC 822, RFC 1123
 * full support for address groups
 * automatic conversion of old-style address lists to RFC 822 format
 * sendmail hook for compatibility with current user agents
 * header line length limited only by memory
 * host masquerading (control/defaulthost)
 * user masquerading ($MAILUSER, $MAILHOST)
 * automatic Mail-Followup-To creation ($QMAILMFTFILE)
 * ability to add signature/content to messages using ''altermime''.
 * ARF Generator (Abuse Bounce Report Generator) for setting up Email Feedback Loop

## SMTP service

 * RFC 2821, RFC 1123, RFC 1651, RFC 1652, RFC 1854, RFC 1870, RFC 1893
 * 8-bit clean
 * 931/1413/ident/TAP callback
 * relay control---stop unauthorized relaying by outsiders (control/rcpthosts)
 * no interference between relay control and aliases
 * automatic recognition of local IP addresses
 * per-buffer timeouts
 * hop counting
 * parallelism limit (tcpserver)
 * per host limit (tcpserver - MAXPERIP)
 * refusal of connections from known abusers(tcpserver, badmailfrom, badmailpatterns,<br />  badhelo, blackholedsender, blackholedpatterns, badhost, badip)
 * goodrcptto, goodrcptpatterns which override the above
 * blackholercpt, blackholercptpatterns for blackholing mails to specific senders.
 * Control files spamignore, blackholedsender, badmailfrom, relaymailfrom, badrcptto,<br />  chkrcptdomains, goodrcptto, blackholercpt, badip can be specified in plain text, cdb format or in MySQL tables.
 * relaying and message rewriting for authorized clients
 * authenticated SMTP PLAIN, LOGIN, CRAM-MD5 (HMAC), CRAM-SHA1, CRAM-SHA224, CRAM-SHA256, CRAM-SHA384, CRAM-SHA512, CRAM-RIPEMD, DIGEST-MD5, SCRAM-SHA-1, SCRAM-SHA-256, SCRAM-SHA-1-PLUS, SCRAM-SHA-256-PLUS (RFC 1321, RFC 2104, RFC 2554, RFC 2617, RFC 5802, RFC 7677, RFC 5056, RFC 5929, RFC 9l66). Supports tls-unique and tls-exporter channel bindings.
 * STARTTLS extension, TLS
 * Support for SMTPS
 * POP/IMAP before SMTP
 * ETRN (RFC 1985)
 * ODMR (RFC 2645)
 * RBL/ORBS support (rblsmtpd)
 * DNSBL (DNS Blacklist) Support
 * SURBL (SURBL Blacklist) support using surblfilter
 * SPAM Control (Reject/Tag/Accept) using Bayesian techniques
 * High Performance MS Virus Control via control file viruscheck and<br />  control file signatures
 * Content Filtering and blocking of prohibited attachments via control file bodycheck
 * Ability to reject/bounce mails for unknown/inactive users (CHECKRECIPIENT)
 * ability to have the RECIPIENT check for selective domains using control file chkrcptdomains
 * Antispoofing mode (turned on by environment variable ANTISPOOFING)
 * Masquerading ability.
 * Multiline greetings via control file smtpgreeting
 * Message Submission Agent – MSA (RFC 2476)
 * Domain IP address pair access control via control file hostaccess
 * Per User accesslist via control file accesslist
 * [SPF](http://www.openspf.org/ "openspf") – Sender Permitted From. Adapted from [SPF implementation for qmail](https://www.saout.de/misc/spf/)
 * [SRS](http://www.libsrs2.org/ "libsrs2") - Sender Rewriting Scheme
 * [Bounce Address Tag Validation (BATV)](http://www.linux-magazine.com/Online/News/IndiMail-1.6-Includes-Greylisting-and-BATV?category=13404 "batv")
 * Per User control of environment variable by envrules(rules file set by environment variable FROMRULES)
 * [Greylisting](http://www.gossamer-threads.com/lists/qmail/users/136761 "greylisting") capability using <b>qmail-greyd</b> or greydaemon
 * SMTP Plugins - External plugins using shared objects in /usr/lib/indimail/plugins to enhance functionality of MAIL, RCPT & DATA session.
 * Notify recipients when message size exceeds databyte limits (by setting environment variable DATABYTES\_NOTIFY)
 * Enforce STARTTLS before AUTH
 * RFC 6530-32 Email Address Internationalization

## Queue management

 * instant handling of messages added to queue
 * parallelism limit (control/concurrencyremote, control/concurrencylocal)
 * queue specific parallelism limit (control/concurrencyr.queueX, control/concurrencyl.queueX)
 * split queue directory---no slowdown when queue gets big
 * quadratic retry schedule---old messages tried less often
 * independent message retry schedules
 * automatic safe queueing---no loss of mail if system crashes
 * automatic per-recipient checkpointing
 * automatic queue cleanups (qmail-clean)
 * queue viewing (qmail-qread)
 * detailed delivery statistics (qmailanalog)
 * Configurable number of queues for load balancing. A queue in indimail is configurable by three environment variables QUEUE\_BASE, QUEUE\_COUNT, and QUEUE\_START. A queue in IndiMail is a collection of queues. Each queue in the collection can have one or more SMTP listener but a single or no delivery (<b>qmail-send</b>) process. It is possible to have the entire queue collection without a delivery process (e.g. SMTP on port 366 – ODMR). The QUEUE\_COUNT can be defined based on how powerful your host is (IO bandwidth, etc). NOTE: This configurable number of queues is possibe with a single installation and does not require you to install multiple instances of qmail.
 * dynamic number, based on queue load, using qscheduler.
 * Ability to hold local, remote or both deliveries (holdlocal, holdremote control file)
 * Qmail Queue Extra Header – Ability to pass extra headers to local and remote deliveries via <b>qmail-queue</b> (Environment variable QQEH).
 * External Virus scanning via QHPSI – Qmail High Performance Scanner Interface
 * Ability to extend QHPSI interface through plugins. The keyword plugin:shared\_lib defined in the environment variable QHPSI denotes 'shared\_lib' to be loaded.
 * Virus scanner qscanq. Ability to detect virus via a third party scanner defined by SCANCMD environment variable (clamscan, clamdscan, etc)
 * Blocking of prohibited filename extensions via qscanq program
 * [Domainkeys](https://en.wikipedia.org/wiki/DomainKeys "Yahoo Domainkeys") (<b>qmail-dk</b>) RFC 4870
 * [DomainKeys Identified Mail](http://marc.info/?l=qmail&m=123817543532399&w=2 "DKIM") with ADSP/SSP (<b>qmail-dkim</b>) RFC 4871, RFC 6376, RFC 8463
 * Set all header values listed in envheader control file as environment variables.
 * Log all headers listed in control file logheaders to stderr.
 * Remove all headers listed in control file removeheaders from email.
 * Ability to do line processing instead of block processing.
 * qmail-nullqueue – blackhole the mail silently.
 * rule based mail archival using control file mailarchive ([Sarbanes–Oxley_Act](https://en.wikipedia.org/wiki/Sarbanes%E2%80%93Oxley_Act "Sarbanes Oxley Act"), [Health_Insurance_Portability_and_Accountability_Act](https://en.wikipedia.org/wiki/Health_Insurance_Portability_and_Accountability_Act "HIPAA Compliance") compliance)
 * Added additional recipients for a message using extraqueue or mailarchive control file.
 * X-Originating-IP header to record the original IP from which the mail originates

## Bounces

 * [QSBMF](https://cr.yp.to/proto/qsbmf.txt "QSBMF") bounce messages---both machine-readable and human-readable
 * [HCMSSC](https://cr.yp.to/proto/hcmssc.txt "HCMSSC") support---language-independent RFC 1893 error codes
 * double bounces sent to postmaster
 * Ability to discard double bounces
 * Ability to preserve [MIME](https://en.wikipedia.org/wiki/MIME "MIME") format when bouncing.
 * Control of bounce process via envrules (rules file controlled by environment variable BOUNCERULES or control files bounce.envrules)
 * limit size of bounce using control file bouncemaxbytes
 * Ability to process bounces using external bounce processor (environment variable BOUNCEPROCESSOR)
 * X-Bounced-Address header to indicate the bounced address.

## Routing by domain

 * any number of names for local host (control/locals)
 * any number of virtual domains (control/virtualdomains)
 * domain wildcards (control/virtualdomains)
 * configurable percent hack support (control/percenthack)
 * Clustered Domain. Same virtual domain can exist on multiple hosts, each having its own set of users. Provides Load Balancing and infinite scalability.

## Remote SMTP delivery

 * RFC 2821, RFC 974, RFC 1123, RFC 1870
 * 8-bit clean
 * automatic downed host backoffs
 * Configurable tcp timeouts for downed host backoffs.
 * automatic switchover to next best MX
 * artificial routing---smarthost, localnet, mailertable (control/smtproutes)
 * Support for jumbo ISP (control/smtproutes.cdb)
 * per-buffer timeouts
 * passive SMTP queue---perfect for SLIP/PPP (<b>serialmail</b>)
 * AutoTURN support (<b>serialmail</b>)
 * Authenticated SMTP (username/password in control/smtproutes) - PLAIN, LOGIN, CRAM-MD5 (HMAC), CRAM-SHA1, CRAM-SHA224, CRAM-SHA256, CRAM-SHA384, CRAM-SHA512, CRAM-RIPEMD, DIGEST-MD5, SCRAM-SHA-1, SCRAM-SHA-256, SCRAM-SHA-1-PLUS, SCRAM-SHA-256-PLUS (RFC 1321, RFC 2104, RFC 2554, RFC 2617, RFC 5802, RFC 7677, RFC 5056, RFC 5929, RFC 9266). Supports tls-unique, tls-exporter channel binding.
 * STARTTLS, TLS
 * Static and Dynamic Routing. (SMTPROUTES environment variable)
 * User based routing instead of normal DNS/smtproutes.
 * Spam control (SPAMFILTER environment variable)
 * Environment variable control via envrules (rules file controlled by environment variable RCPTRULES)
 * QMAILREMOTE environment variable to run any executable/script instead of <b>qmail-remote</b>
 * QMTP support, artificial routing using (control/qmtproutes)
 * ONSUCCESS\_REMOTE, ONFAILURE\_REMOTE scripts run on successful or failed remote deliveries environment variables SMTPTEXT, SMTPCODE available for these scripts
 * IP address binding on domain, sender address, recipient address and random selection from a pool of IP addresses
 * Return Receipt Responder - rrt
 * IPV6 support
 * DANE verification ([RFC 7672](http://tools.ietf.org/html/rfc7672 "RFC 7672"))
 * RFC 6530-32 Email Address Internationalization using libidn2
 * Domain based delivery rate control using drate command

## Local delivery

 * user-controlled address hierarchy    : fred controls fred-anything
 * mbox delivery
 * reliable NFS delivery (maildir)
 * user-controlled program delivery: procmail etc. (qmail-command)
 * optional new-mail notification (qbiff)
 * detailed Delivered-To Headers
 * optional NRUDT return receipts (qreceipt)
 * autoresponder RFC 3834 compliance (provide Auto-Submitted, In-Reply-To, References fields (RFC 3834))
 * conditional filtering (condredirect, bouncesaying, vfilter)
 * Environment variable control via envrules (rules file controlled by environment variable RCPTRULES)
 * Eliminate duplicate messages
 * QMAILLOCAL environment variable to run any executable/script instead of <b>qmail-local</b>
 * X-Forwarded-To, X-Forwarded-For headers
 * Message Disposition Notification (through qnotify)
 * Domain based delivery rate control using drate command

## Other

 * Unix Client Server Program Interface [ucspi]([http://cr.yp.to/proto/ucspi.txt "ucspi-tcp") through programs ''tcpserver'' and ''tcpclient''
 * Change concurrency of ''tcpserver'' without restart.
 * ability to load plugins in tcpserver - dynamically load shared objects given on command line. Load shared objects defined by env variables

   ```
   PLUGIN0, PLUGIN1, ...
   ```

   tcpserver plugin allows you to load qmail-smtpd, rblsmtpd once in memory
 * IPv4 CIDR extension and support for compact IPv6 addresses and CIDR notation
 * TLS/SSL Support in ''tcpserver''
 * STARTTLS extension in IMAP, STLS extension in POP3
 * Ability to restrict connection per IP (MAXPERIP)
 * run shutdown script if present on svc -d
 * ability to log ''svscan'' output using ''multilog''
 * nssd daemon providing [Name Service Switch](https://en.wikipedia.org/wiki/Name_Service_Switch "NSS") which allows extending of the system passwd database to IndiMail's database.
 * pam-multi - Generic [PAM](https://en.wikipedia.org/wiki/Pluggable_authentication_module "PAM") module allows any external programs to authenticate against IndiMails database.
 * multiple checkpassword modules sys-checkpwd, ldap-checkpwd, pam-checkpwd, vchkpass, systpass
 * ''inlookup'' – High Performance User Lookup Daemon.
 * ''indisrvr'' – Indimail Administration Daemon.
 * ''spawn-filter'' - Ability to add disclaimer, run multiple filters before<br />  local/remote delivery.
 * Post Execution Handle - Allows functionality of indimail to be extended by writing simple scripts
 * Proxy for IMAP/POP3 Protocol
 * On the fly migration of users by defining MIGRATEUSER environment variable.
 * ready to use QMQP service
 * ability to distribute QMQP traffic across multiple servers
 * ''sslerator'' - TLS/SSL protocol wrapper for non-tls aware applications
 * ''svctool'' – Configuration tool for IndiMail.
 * ''iwebadmin'' - CGI Web Frontend for IndiMail user administration.
 * mrtg graphs for detailed statistics
 * ability to specify commands in control files.
 * flash - ncurses customizable menu based Admin Tool
 * indium - Menu based administration tool written in tcl/tk
 * osh - Operator Shell (with configurable restricted command set) for Administrator
 * docker/podman support in daemontools
 * support /run, /var/run filesystem in daemontools

## Brief Feature List

Some of the features available in this package

1.  svctool - A simple tool with command-line options which helps you to configure any configuration item in indimail (creation of supervise scripts, qmail configuration, installation of all default MySQL tables, creation of default aliases, users, etc)
2.  configurable control files directory (using CONTROLDIR environment variable) (allows one to have multiple running copies of qmail using a single binary installation)
3.  configurable queue directory (using QUEUEDIR environment variable) (allows one to have multiple queues on a host with a single qmail installation).
    * <b>qmail-queue</b> deposit mails across multiple queues. Each queue has its own <b>qmail-send</b> process. You can spread the individual queues across multiple filesystems on different controllers  to maximize on IO throughput.
    * The number of queues is configurable by three environment variables **QUEUE_BASE**, **QUEUE_COUNT** and **QUEUE_START**. A queue in indimail is defined as a collection of multiple queues.
    * Each queue in the collection can have one or more SMTP listener but a single delivery (<b>qmail-send</b>) processes. It is possible to have the entire queue collection without a delivery process (e.g. SMTP on port 366 ODMR). The QUEUE\_COUNT can be defined based on how powerful your host is (IO bandwidth, etc). The configurable queue is possible with a single installation of indimail-mta and does not require you to have multiple indimail-mta installations (unlike qmail) to achieve this.
4.  uses getpwnam to use uids/gids, home from /etc/passwd, /etc/group (allows me to transfer the binary to another machine regardless of the ids in /etc/passwd)
5.  Hooks into local and remote deliveries
    * QMAILLOCAL - Run executable defined by this instead of <b>qmail-local</b>
    * QMAILREMOTE - Run executable defined by this instead of <b>qmail-remote</b>
    * Theoretically one can exploit QMAILLOCAL, QMAILREMOTE variables to route mails for a domain across multiple mail stores.
6.  Hook into <b>qmail-remote</b>'s routing by using SMTPROUTE environment variable.
    * Ability to do User Based Routing via SMTPROUTE environment. This gives the ability to split a domain across multiple hosts without using NFS to mount multiple filesystems on any host. One can even use a shell script, set the environment variable and deliver mails to users across multiple hosts. I call this dynamic SMTPROUTE.
    * Additionally <b>qmail-rspawn</b> has the ability to connect to MySQL and set SMTPROUTES based on values in a MySQL table. The connection to MySQL is kept open. This gives <b>qmail-rspawn</b> to do high speed user lookups in MySQL and to deliver the mail for a single domain split across multiple mail stores.
7.  Proxy for IMAP and POP3. Allows IMAP/POP3 protocol for users in a domain to be split across multiple hosts. Also allows seamless integration of proprietary email servers with indimail.  The proxy is generic and works with any IMAP/POP3 server. The proxy comes useful when you want to move out of a headache causing mail server like x-change and want to retain the same domain on the proprietary server. In conjunction with dynamic SMTPROUTES, you can migrate all your users to indimail, without any downtime/disruption to email service. The proxy and dynamic SMTPROUTES allow you to scale your email server horizontally without using NFS across geographical locations.
8.  ETRN, ATRN, ODMR (RFC 2645) support
9.  accesslist - restrictions between mail transactions between email ids (you can decide who can send mails to whom)
10. bodycheck - checks on header/body on incoming emails (for spam, virus security and other needs)
11. hostaccess - provides domain, IP address pair access list control. e.g. you can define from which set of addresses mail from yahoo.com will be accepted.
12. chkrcptdomains - rcpt check on selective domains
13. envrules - recipient/sender based - set or unset environment variables (qmail-smtpd, qmail-inject, <b>qmail-local</b>, <b>qmail-remote</b>) any variables which controls the behaviour of qmail-smtpd, qmail-inject, <b>qmail-local</b>, <b>qmail-remote</b> e.g. NODNSCHECKS, DATABYTES, RELAYCLIENT, BADMAILFROM, etc can be defined individually for a particular recipient or sender rather than a fixed value set at runtime.
14. NULLQUEUE, qmail-nullqueue (blackhole support - like qmail-queue but mails go into a blackhole). I typically uses this in conjunction with envrules to trash the mail into blackhole without spending any disk IO.
15. <b>qmail-multi</b> - run multiple filters (qmail-smtpd) (something like <b>qmail-qfilter</b>). Also distributes mails across multiple queues to do a load balancing act. <b>qmail-multi</b> allowed me to process massive rate of incoming mails at my earlier job with a ISP. In the current varsions of IndiMail, <b>qmail-queue</b> doesn't require <b>qmail-multi</b> to select a queue.
16. envheaders - Any thing defined here e.g. Return-Path, qmail-queue sets Return-Path as an environment variable with the value found in Return-Path header in the email. This environment variable gets passed across the queue and is also available to <b>qmail-local</b>, <b>qmail-remote</b>
17. logheaders - Any header defined in this control file, gets written to file descriptor 2 with the value found in the email.
18. removeheaders - Any header defined here, qmail-queue will remove that header from the email
19. quarantine or QUARANTINE env variable causes qmail-queue to replace the recipient list with the value defined in the control file or the environment variable. Additionally an environment variable X-Quarantine-ID: is set which holds the orignal recipient list.
20. Added ability in qmail-queue to do line processing. Line processing allows qmail-queue to do some of the stuff mentioned above
21. plugins support for QHPSI interface (qmail-queue). qmail-queue will use dlopen to load any shared objected defined by PLUGINDIR environment. Multiple plugins can be loaded. For details see man qmail-queue
22. Message Submission Port (port 587) RFC 2476
23. Integrated authenticated SMTP with Indimail (PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-SHA224, CRAM-SHA256, CRAM-SHA384, CRAM-SHA512, CRAM-RIPEMD, DIGEST-MD5, pop-bef-smtp)
24. duplicate eliminator using 822header
25. <b>qmail-remote</b> has configurable TCP timeout table (max\_tolerance, min\_backoff periods can be configured in smtproutes)
26. Ability to change concurrency of tcpserver without restarting tcpserver
27. Ability to restrict connections per IP
28. multilog replaced buffer functions with substdio
29. supervise can run script shutdown if present (when svc -d is issued)
30. rfc3834 compliance for qmail-autoresponder (provide Auto-Submitted, In-Reply-To, References fields (RFC 3834))
31. ability to add stupid disclaimer(s) to messages.
32. InLookup serves as a high performance user lookup daemon for qmail-smtpd (rcpt checks, authenticated SMTP, RELAY check). Even the IMAP, POP3 authentication gets served by inlookup. inlookup preforks configurable number of daemons which opens multiple connections to MySQL and keep the connection open. The query results are cached too. This gives inlookup a decent database performance when handling millions of lookups in few hours. Programs like qmail-smtpd use a fifo to communicate with inlookup
33. CHKRECIPIENT extension which rejects users not found in local MySQL or recipients.cdb database
34. indisrvr Was written to ease mail server administration across multiple hosts. Allows ones to create, delete, modify users and run any command as defined in variables.c. indisrvr listens on a AF\_INET/AF\_INET6 socket.
35. Identation of djb's code (using indent) so that a mortal like me could understand it :)
36. Works with systemd - systemd is an event-based replacement for the init daemon
37. Changed buffer libraries in daemontools to substdio.
38. Can work with external virus scanners (QHPSI, or Len Budney's qscanq)
39. qmail-queue custom error patch by Flavio Curti <fcu-software at no-way.org>
40. Domainkey-Signature, DKIM-Signature with ADSP/SSP
41. Greylisting Capability - [Look Here](http://www.gossamer-threads.com/lists/qmail/users/136740?page=last "Greylisting")
42. nssd - Name Service Switch daemon which extends systems password database to lookup IndiMail's database for authentication
43. pam-multi - Generic PAM which allows external program using PAM to authenticate against IndiMail's database. Using pam-multi and nssd, you can use any IMAP server like dovecot, etc with IndiMail.
44. Post execution Handle - Allows extending indimail's functionality by writing simple scripts
45. sslerator - TLS/SSL protocol wrapper for non-tls aware applications.
46. QMTP support in <b>qmail-remote</b>. QMTP support for mail transfers between IndiMail clusters.
47. Configured installation time QMQP support on server.
48. indimail-mini package providing QMQP client
49. IPV6 Support
50. Multiple checkpassword modules sys-checkpwd, ldap-checkpwd, pam-checkpwd, vchkpass and systpass
51. iwebadmin - Web frontend for IndiMail User administration
52. badhost, badip control files for spam control
53. mailarchive control file (SOX, HIPAA compliance)
54. Notify recipients when message size exceeds databyte limits
55. Ability to run programs on successful or failed remote deliveries
56. Ability to distribute QMQP traffic across servers.
57. Abuse Report Format Generator using qarf
58. Auto provision users in proxyimap/proxypop3
59. DNSBL Support (DNS Blacklist) Author "Fabio Busatto" <fabio.busatto@sikurezza.org>
60. SURBL Support (SURBL Blacklist). URL parsing code borrowed from surbl.c Pieter Droogendijk <pieter@binky.org.uk> http://binky.org.uk
61. Message Disposition Notification using qnotify
62. Return Receipt Responder - rrt
63. Enforce STARTTLS before AUTH using FORCE\_TLS environment variable
64. Updated man pages.
65. Jens Wehrenbrecht's IPv4 CIDR extension
66. Li Minh Bui's IPv6 support for compact IPv6 addresses and CIDR notation support
67. SRS support
68. domain based delivery rate control using drate
69. domain based queue using domainqueue control file
70. Ability to drop bounces
71. Ability to discard emails if filter exits 2
72. goodrcpt, goodrcptpatterns
73. udplogger service for logging messages through UDP
74. docker/podman images for
    * [indimail](https://hub.docker.com/repository/docker/cprogrammer/indimail "IndiMail")
    * [indimail-mta](https://hub.docker.com/repository/docker/cprogrammer/indimail-mta "indimail-mta")
    * [indimail-web](https://hub.docker.com/repository/docker/cprogrammer/indimail-web "RoundCubemail")
75. [FHS](https://refspecs.linuxfoundation.org/FHS_3.0/fhs-3.0.pdf) compliance.
76. Ed Neville - allow multiple Delivered-To in qmail-local using control file maxdeliveredto
77. Ed Neville - configure TLS method in control/tlsclientmethod (qmail-smtpd), control/tlsservermethod (<b>qmail-remote</b>)
78. roundcube support for password, autoresponder through roundcube plugin iwebadmin
79. ezmlm-idx mailing list manager from [https://untroubled.org/ezmlm/](https://untroubled.org/ezmlm/ "ezmlm-idx")
80. tcpserver plugin feature - dynamically load shared objects given on command line. Load shared objects defined by env variables PLUGIN0, PLUGIN1, ...
    tcpserver plugin allows you to load qmail-smtpd, rblsmtpd once in memory
81. tcpserver - enable mysql support by loading mysql library configured in /etc/indimail/control/mysql\_lib
82. indimail-mta - enable virtual domain support by loading indimail shared library configured in /etc/indimail/control/libindimail

# Current Compilation status of all IndiMail & related packages

This is obtained from github actions defined in each of the indimail repository. If during the development process, anything breaks, it will be visible here.

[![indimail ubuntu, mac osx ci](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-c-cpp.yml)
[![indimail freebsd ci](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-freebsd.yml)
[![pam-multi Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/pam-multi-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/pam-multi-c-cpp.yml)
[![pam-multi FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/pam-multi-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/pam-multi-freebsd.yml)
[![nssd Ubuntu CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/nssd-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/nssd-c-cpp.yml)
[![nssd FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/nssd-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/nssd-freebsd.yml)
[![iwebadmin Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/iwebadmin-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/iwebadmin-c-cpp.yml)
[![iwebadmin FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/iwebadmin-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/iwebadmin-freebsd.yml)
[![courier-imap Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/courier-imap-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/courier-imap-c-cpp.yml)
[![courier-imap FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/courier-imap-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/courier-imap-freebsd.yml)
[![fetchmail Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/fetchmail-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/fetchmail-c-cpp.yml)
[![fetchmail FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/fetchmail-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/fetchmail-freebsd.yml)
[![indimail-spamfilter Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-spamfilter-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-spamfilter-c-cpp.yml)
[![indimail-spamfilter FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-spamfilter-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-spamfilter-freebsd.yml)
[![bogofilter-wordlist Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/bogofilter-wordlist-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/bogofilter-wordlist-c-cpp.yml)
[![bogofilter-wordlist FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/bogofilter-wordlist-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/bogofilter-wordlist-freebsd.yml)
[![indimail-utils Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-utils-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-utils-c-cpp.yml)
[![indimail-utils FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-utils-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indimail-utils-freebsd.yml)
[![procmail Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/procmail-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/procmail-c-cpp.yml)
[![procmail FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/procmail-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/procmail-freebsd.yml)
[![logalert Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/logalert-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/logalert-c-cpp.yml)
[![logalert FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/logalert-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/logalert-freebsd.yml)
[![indium Ubuntu CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indium-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indium-c-cpp.yml)
[![indium FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indium-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/indium-freebsd.yml)
[![ircube Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/ircube-ubuntu-osx.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/ircube-ubuntu-osx.yml)
[![ircube FreeBSD CI](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/ircube-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-virtualdomains/actions/workflows/ircube-freebsd.yml)

[![libqmail Ubuntu, Mac OSX](https://github.com/mbhangui/libqmail/actions/workflows/libqmail-c-cpp.yml/badge.svg)](https://github.com/mbhangui/libqmail/actions/workflows/libqmail-c-cpp.yml)
[![libqmail FreeBSD](https://github.com/mbhangui/libqmail/actions/workflows/libqmail-freebsd.yml/badge.svg)](https://github.com/mbhangui/libqmail/actions/workflows/libqmail-freebsd.yml)
[![libdkim Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-mta/actions/workflows/libdkim-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/libdkim-c-cpp.yml)
[![libdkim FreeBSD CI](https://github.com/mbhangui/indimail-mta/actions/workflows/libdkim-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/libdkim-freebsd.yml)
[![libsrs2 Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-mta/actions/workflows/libsrs2-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/libsrs2-c-cpp.yml)
[![libsrs2 FreeBSD CI](https://github.com/mbhangui/indimail-mta/actions/workflows/libsrs2-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/libsrs2-freebsd.yml)
[![indimail-mta Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-mta/actions/workflows/indimail-mta-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/indimail-mta-c-cpp.yml)
[![indimail-mta FreeBSD CI](https://github.com/mbhangui/indimail-mta/actions/workflows/indimail-mta-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/indimail-mta-freebsd.yml)
[![daemontools Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-mta/actions/workflows/daemontools-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/daemontools-c-cpp.yml)
[![daemontools FreeBSD CI](https://github.com/mbhangui/indimail-mta/actions/workflows/daemontools-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/daemontools-freebsd.yml)
[![ucspi-tcp Ubuntu, Mac OSX CI](https://github.com/mbhangui/indimail-mta/actions/workflows/ucspi-tcp-c-cpp.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/ucspi-tcp-c-cpp.yml)
[![ucspi-tcp FreeBSD CI](https://github.com/mbhangui/indimail-mta/actions/workflows/ucspi-tcp-freebsd.yml/badge.svg)](https://github.com/mbhangui/indimail-mta/actions/workflows/ucspi-tcp-freebsd.yml)

# Performance / Benchmarks

As stated earlier, indimal-mta was built for supporting few million users with as little hardware as possible. The queue design gives indimail-mta unparalled performance. All details are available [here](https://github.com/mbhangui/indimail-mta/tree/master/indimail-mta-x/qmail-perf).

## Observations

* qmail based MTAs that use an dedicated todo processor demonstrate a lower qtime
* dedicated todo processor has a remarkable impact on the local concurrency. The concurrency never reaches high values with high inject rates.
* processing todo in batches has a significant impact on qmail-send performance and delivery times by as much as 30%. But this has an impact on the delivery of the first email.
* Increasing directory split has negligible effect in qmail-perf test and filesystem test
* statically linked binaries give much better performance. With dynamic linking, indimail-mta performs the worst amongst all MTAs.
* When delivery rate increases inject rate decreases
* The biggest impact on local delivery rate are the fsync() calls. Changing fsync() to fdatasync() did not result in improving the delivery rate. Disabling fsync() resulted in local deliveries increasing by 6x.
	* Disabling fsync, ext4 gave the best performance in the test carried out
	* Using fsync, zfs gave the best performance in the tests carried out
* netqmail gives the best injection rate. One of the reason is statically compiled uids, gids which avoids the need to do passwd, group entry lookups uisng the getpw, getgr libc functions. libqmail provides replacement for each of these getpw functions. Setting the environment variable USE_GETPW in indimail-mta to use the libqmail getpw interface results in significant improvement in user lookup times.

## Results

Results on [Google Sheet](https://docs.google.com/spreadsheets/d/1Dfr1c1RXh18Lc47fmGymTRV5nL9DRviS9Gy8kqH5iZM/edit?usp=sharing)

# Credits

This project has been built on the work of many. If I have missed out anyone, do let me know and I will be glad to add it to the [list](https://github.com/mbhangui/indimail-mta/blob/master/indimail-mta-x/doc/CREDITS.md). This is a comprehensive list and I have tried by best to list all contributions made by other whose work I have picked up to build bits and pieces of IndiMail.

# History

Both indimail-mta and indimail-virtualdomains started in late 1999 as a combined package of unmodified qmail and modified [vpopmail](https://www.inter7.com/vpopmail-virtualized-email/).

indimail-mta started as a unmodified qmail-1.03. This was when I was employed by an ISP in late 1999. The ISP was using [Critical Path's ISOCOR](https://www.wsj.com/articles/SB940514435217077581) for providing Free and Paid email service. Then the dot com burst happened and ISP didn't have money to spend on upgrading the E3500, E450 [Sun Enterprise servers](https://en.wikipedia.org/wiki/Sun_Enterprise). The mandate was to move to an OSS/FS solution. After evaluating sendmail, postfix and qmail, we chose qmail. During production deployment, qmail couldn't scale on these servers. The issue was the queue's todo count kept on increasing. We applied the <b>todo-proc</b> and the <b>big-todo</b> patch, but still we couldn't handle the incoming email rate. By now the customers were screaming, the corporate users were shooting out nasty emails. We tried a small hack which solved this problem. Compiled 20 different qmail setups, with conf-qmail as /var/qmail1, /var/qmail2, etc. Run <b>qmail-send</b> for each of these instance. A small shim was written which would get the current time and divide by 20. The remainder was used to do exec of /var/qmail1/bin/qmail-queue, /var/qmail2/bin/qmail-queue, etc. The shim was copied as /var/qmail/bin/qmail-queue. The IO problem got solved. But the problem with this solution was compiling the qmail source 20 times and copying the shim as <b>qmail-queue</b>. You couldn't compile qmail on one machine and use the backup of binaries on another machine. Things like uid, gid, the paths were all hardcoded in the source. That is how the base of indimail-mta took form by removing each and every hard coded uids, gids and paths. indimail-mta still does the same thing that was done in the year 2000. The installation creates multiple queues - /var/indimail/queue/queue1, /var/indimail/queue/queue2, etc. A new daemon named qscheduler uses QUEUE\_COUNT env variable to run multiple <b>qmail-send</b> instances. Each <b>qmail-send</b> instance picks up mail from a particular queue. All client programs use <b>qmail-queue</b> to load balance the incoming email across multiple queues. At one point of time in late 2000, indimail was handling 3.2 million users with two E450 server as relay servers (indimail-mta), one E3500 Sun Microsystem servers and 5 Compaq Proliant DL380 running indimail-virtualdomains.

indimail-virtualdomain started with a modified vpopmail base that could handle a distributed setup - Same domain on multiple servers. Having this kind of setup made the control file smtproutes unviable. email would arrive at a relay server for user@domain. But the domain '@domain' was present on multiple hosts, with each host having it's own set of users. This required special routing and modification of qmail (especially <b>qmail-remote</b>) to route the traffic to the correct host. <b>vdelivermail</b> to had to be written to deliver email for a local domain to a remote host, in case the user wasn't present on the current host. New MySQL tables were created to store the host information for a user. This table would be used by <b>qmail-local</b>, <b>qmail-remote</b>, <b>vdelivermail</b> to route the mail to the write host. All this complicated stuff had to be done because the ISP where I worked, had no money to buy/upgrade costly servers to cater to users, who were multiplying at an exponential rate. The govt had just opened the license for providing internet services to private players. These were Indians who were tasting internet and free email for the first time. So the solution we decided was to buy multiple intel servers [Compaq Proliant](https://en.wikipedia.org/wiki/ProLiant) running Linux and make the qmail/vpopmail solution horizontally scalable. This was the origin of indimail-1.x which borrowed code from vpopmail, modified it for a domain on multiple hosts. indimail-2.x was a complete rewrite using djb style, using [libqmail](https://github.com/mbhangui/libqmail) as the standard library for all basic operations. All functions were discarded because they used the standard C library. The problem with indimail-2.x was linking with MySQL libraries, which caused significant issues building binary packages on [openSUSE build service](https://build.opensuse.org/). Binaries got built with MySQL/MariaDB libraries pulled by OBS, but when installed on a target machine, the user would have a completely different MySQL/MariaDB setup. Hence a decision was taken to load the library at runtime using dlopen/dlsym. This was the start of indimail-3.x. The source was moved from sourceforge.net to github and the project renamed as indimail-virtualdomains from the original name IndiMail. The modified source code of qmail was moved to github as indimail-mta.

# See also

IndiMail's journey has been possible because of the qmail mailing list and these three excellent resources.

* [Official qmail website, maintained by Daniel J. Bernstein](http://cr.yp.to/qmail.html "qmail")
* [Unofficial qmail website, maintained by Russ Nelson](http://www.qmail.org "qmail.org")
* [Life with qmail](http://www.lifewithqmail.org/lwq.html "LWQ")
