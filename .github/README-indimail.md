[IndiMail](indimail_logo.png "IndiMail")

Table of Contents
=================

   * [INTRODUCTION](#introduction)
   * [LICENSING](#licensing)
   * [TERMINOLOGY used for commands](#terminology-used-for-commands)
   * [indimail-mta Internals](#indimail-mta-internals)
      * [Programs](#programs)
         * [qmail-multi](#qmail-multi)
         * [qmail-queue](#qmail-queue)
         * [qmail-daemon, qmail-start](#qmail-daemon-qmail-start)
         * [qmail-send, qmail-send](#qmail-send-qmail-send)
            * [Preprocessing](#preprocessing)
            * [Delivery](#delivery)
               * [Retry Schedules](#retry-schedules)
            * [Cleanup](#cleanup)
            * [Global &amp; Queue Specific Concurrency, Parallelism limits](#global--queue-specific-concurrency-parallelism-limits)
      * [Setting Environment Variables](#setting-environment-variables)
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
   * [Distributing your outgoing mails from Multiple IP addresses](#distributing-your-outgoing-mails-from-multiple-ip-addresses)
   * [Processing Bounces](#processing-bounces)
      * [Using environment variable BOUNCEPROCESSOR](#using-environment-variable-bounceprocessor)
      * [Using environment variable BOUNCERULES or control files bounce.envrules.](#using-environment-variable-bouncerules-or-control-files-bounceenvrules)
      * [Using BOUNCEQUEUE environment variable to queue bounces](#using-bouncequeue-environment-variable-to-queue-bounces)
   * [Delivery Instructions for a virtual domain](#delivery-instructions-for-a-virtual-domain)
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
   * [Fedora - Using /usr/sbin/alternatives](#fedora---using-usrsbinalternatives)
   * [Post Handle Scripts](#post-handle-scripts)
   * [Relay Mechanism in IndiMail](#relay-mechanism-in-indimail)
      * [Using tcp.smtp](#using-tcpsmtp)
      * [Using control file relayclients](#using-control-file-relayclients)
      * [Using MySQL relay table](#using-mysql-relay-table)
   * [Set up Authenticated SMTP](#set-up-authenticated-smtp)
      * [Using control file relaydomains](#using-control-file-relaydomains)
      * [Using control file relaymailfrom](#using-control-file-relaymailfrom)
   * [CHECKRECIPIENT - Check Recipients during SMTP](#checkrecipient---check-recipients-during-smtp)
   * [IndiMail Control Files Formats](#indimail-control-files-formats)
   * [Inlookup database connection pooling service](#inlookup-database-connection-pooling-service)
   * [Setting limits for your domain](#setting-limits-for-your-domain)
   * [SPAM and Virus Filtering](#spam-and-virus-filtering)
   * [SPAM Control using bogofilter](#spam-control-using-bogofilter)
   * [SPAM Control using badip control file](#spam-control-using-badip-control-file)
   * [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi)
      * [Using tcprules](#using-tcprules)
      * [Using envdir for SMTP service under supervise(8)](#using-envdir-for-smtp-service-under-supervise8)
   * [SMTP Access List](#smtp-access-list)
   * [Using spamassasin with IndiMail](#using-spamassasin-with-indimail)
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
      * [DKIM Author Domain Signing Practices](#dkim-author-domain-signing-practices)
      * [Setting qmail-local to verify DKIM signatures](#setting-qmail-local-to-verify-dkim-signatures)
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
* [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp")
* [serialmail](https://cr.yp.to/serialmail.html "serialmail")
* [qmailanalog](http://cr.yp.to/qmailanalog.html "qmailanalog")
* [dotforward](https://cr.yp.to/dot-forward.html "dot-foward")
* [fastforward](https://cr.yp.to/fastforward.html "fastforward")
* [mess822](https://cr.yp.to/mess822.html "mess822")
* [daemontools](https://cr.yp.to/daemontools.html "daemontools")
* [dkim](https://en.wikipedia.org/wiki/DomainKeys_Identified_Mail "DKIM")
* [SRS2](https://en.wikipedia.org/wiki/Sender_Rewriting_Scheme "SRS2")

[**ezmlm-idx**](https://github.com/mbhangui/ezmlm-idx "ezmlm-idx")

* [ezmlm-idx mailing list manager](https://untroubled.org/ezmlm/ "ezmlm-idx")

[indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access "indimail-access")

* [courier IMAP/POP3](https://www.courier-mta.org/imap/ "courier-imap")
* [fetchmail](https://www.fetchmail.info "fetchmail")

[**indimail-auth**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-auth "indimail-auth")

* [nssd - providing Name Service Switch](https://github.com/mbhangui/indimail-virtualdomains/tree/master/nssd-xi "nssd")
* [pam-multi - PAM modules for flexible, configurable authentication methods](https://github.com/mbhangui/indimail-virtualdomains/tree/master/pam-multi-x "pam-multi")

[**indimail-utils**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-utils "indimail-utils")

* Utilities ([altermime](http://pldaniels.com/altermime/ "altermime"), [ripMIME](https://pldaniels.com/ripmime/ "ripmime"), [mpack](https://github.com/mbhangui/indimail-virtualdomains/tree/master/mpack-x "mpack"), [fortune](https://en.wikipedia.org/wiki/Fortune_\(Unix\ "fortune") and [flash](https://github.com/mbhangui/indimail-virtualdomains/tree/master/flash-x "flash"))
* [logalert](https://github.com/mbhangui/indimail-virtualdomains/tree/master/logalert-0.3 "logalert")

[**indimail-spamfilter**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/bogofilter-x "bogofilter")

* [bogofilter - A Bayesian Spam Filter](https://bogofilter.sourceforge.io/ "bogofilter")

Core **IndiMail** consists of three main packages - [indimail-mta](https://github.com/mbhangui/indimail-mta "indimail-mta"), [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains") and [indimail-access](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-access "indimail-access")

* **indimail-mta** is a re-engineered version of [qmail](http://cr.yp.to/qmail.html "qmail").  **indimail-mta** provides you a MTA with all the features and functionality of **[qmail](http://cr.yp.to/qmail.html "qmail")** plus many additional features. <b>indimail-mta</b> is an independent package. You can install it without requiring <b>indimail-virtualdomains</b> or <b>indimail-access</b>.
* **indimail-virtualdomains** provides you tools to create and manage multiple virtual domains with its own set of users, who can send and receive mails. <b>indimail-virtualdomains</b> is dependent on <b>indimail-mta</b> to function.
* **indimail-access** provides you IMAP/POP3 protocols & fetchmail utility. It allows the users to access their emails delivered by <b>indimail-mta</b> locally to a host to users in /etc/passwd or to <b>virtual users</b> on a host (or multiple hosts) on the network having <b>virtual domains</b> created by <b>indimail-virtualdomains</b>.

This document will refer to **IndiMail** as a combined package of indimail-virtualdomains, indimail-mta & other packages namely indimail-access, indimail-auth, indimail-utils, indimail-spamfilter.

When you install indimail-virtualdomains, a shared library from the package is dynamically loaded by indimail-mta to provide virtual domain support in indimail-mta, along with the ability to work with IMAP/POP3 retrieval daemons.

indimail-virtualdomains provides programs to manage multiple virtual domains on a single host. It allows extending any of the domains across multiple servers. With indimail-mta installed, two compoments - **qmail-rspawn**, **qmail-remote** can act as an SMTP router/interceptor. Both the components use a **MySQL** database to know the location of each and every user. Similarly, the Mail Delivery Agent (MDA) **vdelivermail** uses the same MySQL database, allowing it to re-route any email to any server where the user's mailbox is present. Additionally, indimail-virtualdomains provide **proxyimap** and **proxypop3** - proxies for IMAP and POP3 protocols, to retrieve the user's mailbox from any server. To deliver or retrieve any email, the user doesn't have to connect to any specific serer. This is a very powerful feature which allows IndiMail to provide native horizontal scalability. It knows the location of each and every user and can distribute or retrieve mails for any user residing on any server located anywhere on the network. If one uses IndiMail, one can simply add one more server and cater to more users without changing any configuration, software or hardware of existing servers.

This does not force a filesystem architecture like NFS to be used for provisioning large number of users (typically in an ISP/MSP environment). In the architecture below, you can keep on increasing the number of servers (incoming relay, outgoing relay or mailstores) to cater to large number of users. This allows you to scale indimail easily to serve millions of users using commodity hardware. You can read the feature list to get an idea of the changes and new features that have been added over the original packages. You can see a pictorial representation of the architecture below

![architecture](indimail_arch.png "IndiMail Architecture")

IndiMail allows servers to be distributed anywhere geographically. This is useful especially if you have users at different parts of the globe. e.g. Your Brazil users can have their server located in Brazil, Bombay users in Bombay, Delhi users in Delhi. And yet when your Brazil users comes to Delhi for a visit, he or she can access all emails sitting in Delhi by accessing the Delhi server. IndiMail provides this distributed feature using proxies for SMTP, IMAP and POP3 protocols. The proxy servers run using [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp") and are by default configured under supervise. You can use any IMAP/POP3 server behind the proxy. You can extend the domain across multiple servers without using any kind of NAS storage.

The ability of IndiMail to know user's location also allows IndiMail to setup a heterogeneous messaging environment. If you have IndiMail, you can have a server running MS exchange, few servers running IBM Lotus Notes, and few servers running IndiMail and all using a single domain. A utility called 'hostcntrl' allows you to add foreign users to IndiMail's database.  This feature also allows you to migrate your users from a proprietary platform to IndiMail without causing downtime or disruption to existing users. In fact, this method has been used very successfully in migrating corporate users out of MS Exchange & IBM Lotus Notes to IndiMail without the end users realizing it.

To migrate from an existing proprietary mail server like MS Exchange requires 5 steps.

1. You simply set up a new installation with IndiMail and create the existing domain using **vadddomain**.
2. Add the IP address of the Exchange Server in ***host_table*** and the SMTP port of the Exchange Server in the table ***smtp_port***.
3. Add users on the exchange server to a table called ***hostcntrl*** (either manually or using the utility **hostcntrl**).
4. Modify your user's mail configuration to use SMTP, IMAP Proxy, POP3 Proxy ports on the IndiMail server (**proxypop3**, **proxyimap**)
5. Change the MX to point to the indimail-mta server.

IndiMail is highly configurable. No hard-coded directories of qmail like /var/qmail/control or /var/qmail/queue. All directories are configurable at run time. IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration.  To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance.  Multiple queues eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). You can see a pictorial representation of the queue as well as read the [indimail-mta INTERNALS](#indimail-mta-internals).

IndiMail is a pure messaging solution. It does not provide calendars, todo lists, address books, meeting requests and a web mail front-end. However, you can use [RoundCubemail](https://roundcube.net/ "RoundCubemail") or any web mail front-end that works with IMAP or POP3 protocol with IndiMail. If you decide to install [RoundCubemail](https://roundcube.net/ "RoundCubemail"), you can install the **ircube** package from the IndiMail's DNF/YUM/Debian [Repository](https://build.opensuse.org/package/show/home:indimail/ircube "ircube") to have a fully functional web mail front-end. The ircube package provides plugins for Rouncube Mail to manage your passwords, vacation and SPAM filters.

IndiMail administrators can use a web administration tool called **iwebadmin**. It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/master/iwebadmin-x "iwebadmin") or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/iwebadmin "iwebadmin Repository") on the openSUSE build Service.

IndiMail comes with a power tcl/tk administration client called [**indium**](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indium-1.1 "indium"). It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indium-1.1 "indium") or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/indium "indium repository") on the openSUSE Build Service.

To install IndiMail you can take the help of the following documents in the [indimail-x/doc subdirectory](https://github.com/mbhangui/indimail-virtualdomains/tree/master/indimail-x/doc "Documents Subdirectory") of [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains") github repository. You can also jump to the section [Installing Indimail using DNF/YUM/APT Repository](#installing-indimail-using-dnfyumapt-repository) towards the bottom of this document.

File | Description
----- | --------------------
README-indimail.md|Introduction to IndiMail (this file)
[INSTALL-indimail.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-indimail.md)|Detailed Source Installation Instructions. A simpler version is [this](https://github.com/mbhangui/indimail-virtualdomains/blob/master/README.md)
[INSTALL-RPM.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-RPM.md)|Install Instructions using RPM
[INSTALL-MYSQL.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/INSTALL-MYSQL.md)|MySQL specific Installation Instructions
[README-CLUSTER.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/README-CLUSTER.md)|Steps on configuring a clustered setup
[Quick-INSTALL.md](https://github.com/mbhangui/indimail-virtualdomains/blob/master/.github/Quick-INSTALL.md)|A minimal documentation on Installation/Configuration)

If you have enough experience on your belt you can dive into this [document](https://github.com/mbhangui/indimail-virtualdomains/blob/master/README.md "README").

Once you have installed IndiMail, you will find all man pages in /usr/share/man and documents in /usr/share/indimail/doc. You can do man indimail to get started with understanding IndiMail.

#DISCLAIMER

There is no warranty implied or otherwise with this package. I believe in OpenSource Philosophy and this is an attempt to give back to the OpenSource community. However, I welcome anyone who can provide some assistance for building few missing features (building new features, testing and documentation).

# LICENSING

IndiMail uses GPLv3 License. See file [LICENSE](https://github.com/mbhangui/indimail-virtualdomains/blob/master/LICENSE "GPLv3"). Additional licenses (if any) may be found in subfolder of each component that IndiMail uses.

# TERMINOLOGY used for commands

```
$ command      - command was executed by a non-privileged user
# command      - command was executed by the `root` user
$ sudo command - command requires root privilege to run. sudo was used to gain root privileges
```

# indimail-mta Internals

Here's the data flow in the indimail-mta suite:

```
 qmail-smtpd --- qmail-multi --- qmail-queue --- qmail-todo
               /                                 /    \
qmail-inject _/                                 /      \___ qmail-clean
                              _________________/
                             /
                         qmail-send --- qmail-rspawn --- qmail-remote
                                    \
                                     \_ qmail-lspawn --- qmail-local
```

The diagram below shows how <b>qmail-multi</b>(8) works ![qmail-multi](qmail_multi.png)

Every message is added to a central queue directory by <b>qmail-queue</b>. <b>qmail-queue</b> is invoked by <b>qmail-multi</b>. The main purpose of <b>qmail-multi</b> is to select a queue as discussed in [IndiMail Queue Mechanism](#indimail-queue-mechanism). Here is a pictorial representation of the IndiMail queue. ![Pictorial](indimail_queue.png)

<b>qmail-multi</b> is invoked as needed, by setting QMAILQUEUE=/usr/sbin/qmail-queue, mostly by <b>qmail-inject</b> for locally generated messages, <b>qmail-smtpd</b> for messages received through SMTP, <b>qmail-local</b> for forwarded messages, or <b>qmail-send</b> for bounce messages.

Every message is then pre-processed by <b>qmail-todo</b> and then processed by <b>qmail-send</b> for delivery, in cooperation with <b>qmail-lspawn</b>, <b>qmail-rspawn</b> and cleaned up by <b>qmail-clean</b>. These five programs are long-running daemons. The diagram also shows a separate queue named <b>slowq</b>. This queue is special. It is a single queue that has <b>slowq-send</b> processing it instead of <b>qmail-todo</b>, <b>qmail-send</b> pair. This queue has a feature where the deliveries can be rate controlled. <b>slowq-send</b> is like the orignal qmail's <b>qmail-send</b> and unlike it, <b>slowq-send</b> does both pre-procesinsg and scheduling and is not as fast as <b>qmail-send</b> and hence the name. However the queue <b>slowq</b> ain't a queue where we require speed, and so it is ok.

The queue is designed to be crashproof, provided that the underlying filesystem is crashproof. All cleanups are handled by <b>qmail-send</b> and <b>qmail-clean</b> without human intervention.

## Programs

### qmail-multi

<b>qmail-multi</b> is a frontend for <b>qmail-queue</b>. It selects one of the multiple queues, sets the environment variable <b>QUEUEDIR</b> and calls <b>qmail-queue</b>. <b>qmail-multi</b> is discussed in detail in [IndiMail Queue Mechanism](#indimail-queue-mechanism). <b>qmail-multi</b> can also do spam filtering before a message gets queued. See [SPAM Control using bogofilter](#spam-control-using-bogofilter) for more details.

### qmail-queue

Each message in the queue is identified by a unique number, let's say 3016451. This number is related to the inode number of file created in queueX/pid directory. More on that below. From now on, we will refer to 3016451 as <u>inode</u> The queue is organized into several directories, each of which may contain files related to message 3016451:

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

* 	Received: (indimail-mta 37166 invoked by alias); 26 Sep 1995 04:46:54 -0000
* 	Received: (indimail-mta 37166 invoked from network from w.x.y.z by host argos by uid 123); 26 Sep 1995 04:46:54 -0000
* 	Received: (indimail-mta 37166 invoked for bounce); 26 Sep 1995 04:46:54 -0000

where:

* 	37166 is <b>qmail-queue</b>'s process ID.
* 	invoked by alias means <b>qmail-queue</b> was invoked by a process with uid of alias user. This will be through <b>qmail-local</b> reading processing a dot-qmail file
* 	invoked from network means <b>qmail-queue</b> was invoked by user qmaild received from host w.x.y.z on host argos.
* 	invoked from bounce means that this was a bounce generated by <b>qmail-send</b> running under uid qmails
* 	26 Sep 1995 04:46:54 -OOOO is the time and date at which <b>qmail-queue</b> created the message.

<b>qmail-queue</b> places a messages in the queue in four stages:

1. 	To add a message to the queue, <b>qmail-queue</b> first creates a file in a separate directory, queueX/pid, with a unique name. The filesystem assigns that file a unique inode number. <b>qmail-queue</b> looks at that number, say 3016451. By the guarantee above, message 3016451 must be in state S1.
2. 	The queueX/pid file is renamed to queueX/mess/split/<u>inode</u>, and the message is written to the file, moving to state S2. Here split is the remainder left from dividing inode number by the compile time conf-split value. For example, if inode is 3016451 and conf-split is the default, 151, then split is 75 (3016451 divided by 151 is 19976 which gives a remainder of (3016451 - 19976 * 151) = 75)
3.	The file queueX/intd/<u>inode</u> is created and the envelope is written to it in the form

	`u1011\0p28966\0Ftuser@example.com\0Tuser1@a.com\0Tuser2@b.com\0`

	It means the above message was sent by user tuser@example.com with uid 1011, process ID 28966 to two users user1@a.com, user2@b.com. At this point, we have moved to state S3
4.	queueX/todo/<u>inode</u> is linked to queueX/intd/<u>inode</u>, moving the state to S4. At this instant, message has been successfully queued for further processing by <b>qmail-todo</b>, <b>qmail-send</b>. 

At the moment queueX/todo/<u>inode</u> is created, the message has been queued. <b>qmail-send</b> eventually (within 25 minutes notices the new message, but to speed things up, <b>qmail-queue</b> writes a single byte to lock/trigger, a named pipe that <b>qmail-send</b> watches. When trigger contains readable data, qmail- send is awakened, empties trigger, and scans the todo directory.

<b>qmail-queue</b> starts a 24-hour timer before touching any files, and commits suicide if the timer expires.

Once a message is deposited in one of the indimail's queues, it will be sent by few programs working cooperatively. We will now look at qmail-daemon, qmail-start, <b>qmail-send</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, <b>qmail-local</b>, and <b>qmail-remote</b>.

### qmail-daemon, qmail-start

<b>qmail-daemon</b> invokes multiple invocations of <b>qmail-start</b> to invoke <b>qmail-send</b>, <b>qmail-todo</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, and <b>qmail-clean</b>, under the proper uids and gids for multiple queues. It starts multiple instances of <b>qmail-send</b>, <b>qmail-todo</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b> and <b>qmail-clean</b>. The number of instances it runs is defined by the environment variable QUEUE\_COUNT. For each instance the queue is defined by <b>qmail-daemon</b> by setting the environment variable <b>QUEUEDIR</b>. A queue is defined by the integers defined by environment variables QUEUE\_START and QUEUE\_COUNT as described in section [IndiMail Queue Mechanism](#indimail-queue-mechanism). <b>qmail-daemon</b> also monitors <b>qmail-send</b> and <b>qmail-todo</b> and restart them if they go down.

<b>qmail-start</b> invokes <b>qmail-send</b>, <b>qmail-todo</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, and <b>qmail-clean</b>, under the proper uids and gids for a single queue. These five daemons cooperate to deliver messages from the queue. <b>qmail-start</b> should be used if you desire to run only one queue. For running multiple parallel queues run <b>qmail-daemon</b>.

<b>slowq-start</b> invokes <b>slowq-send</b>, lspawn, <b>qmail-rspawn</b>, and <b>qmail-clean</b>, under the proper uids and gids for a single queue. These four daemons cooperate to deliver messages from the queue. <b>slowq-start</b> is invoked for the special queue <b>slowq</b> that can be rate controlled. We will talke about this in the chapter [Controlling Delivery Rates](#-controlling-delivery-rates).

<b>qmail-daemon</b>, <b>qmail-start</b>, <b>slowq-start</b> can be passed an argument - <b>defaultdelivery</b>. If <b>defaultdelivery</b> supplied, <b>qmail-start</b> or <b>qmail-daemon</b> passes it to <b>qmail-lspawn</b>. You can also have a control file named defaultdelivery. The mailbox type is picked up from the <b>defaultdelivery</b> control file. The table below outlines the choices for <b>defaultdelivery</b> control file

Mailbox Format |Name|Location|defaultdelivery|Comments
---------------|----|--------|---------------|--------
mbox|Mailbox|$HOME|./Mailbox|most common, works with most MUAs
maildir|Maildir|$HOME|./Maildir/|more reliable, less common MUA support
mbox|username|/var/spool/mail|See INSTALL.vsm|traditional mailbox

### qmail-send, qmail-send

Once a message has been queued, <b>qmail-todo</b> must decide which recipients are local and which recipients are remote. It may also rewrite some recipient addresses. <b>qmail-todo</b>/<b>qmail-send</b> process messages in the queue and pass them to <b>qmail-rspawn</b> and <b>qmail-lspawn</b>. We will talk about their function in the order that a message in the queue would experience them: preprocessing, delivery, and cleanup.

#### Preprocessing

<b>qmail-todo</b> does the preprocessing and like queuing, this is done in stages:

1. 	When <b>qmail-todo</b> notices queueX/todo/<u>inode</u>, it knows that the message <u>inode</u> is in S4. <b>qmail-todo</b> deletes queueX/info/split/<u>inode</u>, queueX/local/split/<u>inode</u>, and queueX/remote/split/<u>inode</u>, if they exist. Then it reads through queueX/todo/<u>inode</u>.
2. 	A new queueX/info/split/inode is created, containing the envelope sender address.
3. 	If the message has local recipients, they're added to queueX/local/split/<u>inode</u>.
4. 	If the message has remote recipients, they're added to queueX/remote/split/<u>inode</u>.
5. 	queueX/intd/<u>inode</u> is deleted. The message is still in S4 at this point.
6. 	queueX/todo/<u>inode</u> is deleted, moving to stage S5. At this instant the message has been succcesfully preprocessed. Recipients are considered local if the domain is listed in control/locals or the entire recipient or domain is listed in control/virtualdomains. If the recipient is virtual, the local part ofthe address is rewritten as specified in virtualdomains.

#### Delivery

Messages at S5 are handled as follows. Initially, all recipients in queueX/local/split/<u>inode</u> and queueX/remote/split/<u>inode</u> are marked NOT DONE, meaning that <b>qmail-send</b> should attempt to deliver to them. On its own schedule, <b>qmail-send</b> sends delivery commands to <b>qmail-lspawn</b> and <b>qmail-rspawn</b> using channels set up by qmail-start. When it receives responses from <b>qmail-lspawn</b> or <b>qmail-rspawn</b> that indicate successful delivery or permanent error, <b>qmail-send</b> changes their status in queueX/local/split/<u>inode</u> or queueX/remote/split/<u>inode</u> to DONE, meaning that it should not attempt further deliveries. When <b>qmail-send</b> receives a permanent error, it also records that in queueX/bounce/split/<u>inode</u>. Bounce messages are also handled on <b>qmail-send</b>'s schedule. Bounces are handled by injecting a bounce message based on queueX/mess/split/<u>inode</u> and queueX/bounce/split/<u>inode</u>, and deleting queueX/bounce/split/<u>inode</u>. When all ofthe recipients in queueX/local/split/<u>inode</u> or queueX/remote/split/<u>inode</u> are marked DONE, the respective local or remote file is removed.

<b>qmail-send</b> may at its leisure try to deliver a message to a NOT DONE address. If the message is successfully delivered, <b>qmail-send</b> marks the address as DONE. If the delivery attempt meets with permanent failure, <b>qmail-send</b> first appends a note to queueX/bounce/split/<u>inode</u>, creating queueX/bounce/split/<u>inode</u> if necessary; then it marks the address as DONE. Note that queueX/bounce/split/<u>inode</u> is not crashproof.

<b>qmail-send</b> may handle queueX/bounce/split/<u>inode</u> at any time, as follows: it

1. 	injects a new bounce message, created from queueX/bounce/split/<u>inode</u> and queueX/mess/split/<u>inode</u>;
2. 	deletes queueX/bounce/split/<u>inode</u>.

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

1. 	Processing queueX/bounce/split/<u>inode</u>, if it exists.
2. 	Deleting queueX/info/split/<u>inode</u>.
3. 	Deleting queueX/mess/split/<u>inode</u>.

Partially queued and partially dequeued messages left when a system crash interrupts <b>qmail-queue</b> or <b>qmail-send</b> are deleted by <b>qmail-send</b> using <b>qmail-clean</b>, another long-running daemon started by qmail-start. Messages with a queueX/mess/split/<u>inode</u> file and possibly an queueX/intd/<u>inode</u> but no todo, info, local, remote, or bounce, are safe to delete after 36 hours because <b>qmail-queue</b> kills itself after 24 hours. Similarly, files in the pid directory more than 36 hours old are also deleted.

If the computer crashes while <b>qmail-queue</b> is trying to queue a message, or while <b>qmail-send</b> is eliminating a message, the message may be left in state S2 or S3.

When <b>qmail-send</b> sees a message in state S2 or S3, other than one it is currently eliminating, where queueX/mess/split/<u>inode</u> is more than 36 hours old, it deletes queueX/intd/split/<u>inode</u> if that exists, then deletes queueX/mess/split/<u>inode</u>. Note that any <b>qmail-queue</b> handling the message must be dead.

Similarly, when <b>qmail-send</b> sees a file in the queueX/pid directory that is more than 36 hours old, it deletes it.

Cleanups are not necessary if the computer crashes while <b>qmail-send</b> is delivering a message. At worst a message may be delivered twice. (There is no way for a distributed mail system to eliminate the possibility of duplication. What if an SMTP connection is broken just before the server acknowledges successful receipt of the message. The client must assume the worst and send the message again. Similarly, if the computer crashes just before <b>qmail-send</b> marks a message as DONE, the new <b>qmail-send</b> must assume the worst and send the message again. The usual solutions in the database literature e.g., keeping log files amount to saying that it's the recipient's computer's job to discard duplicate messages.)

#### Global & Queue Specific Concurrency, Parallelism limits

<b>qmail-lspawn</b> and <b>qmail-rspawn</b> can do multiple concurrent deliveries. The default concurrency limit is 5 for local deliveries and 10 for remote deliveries. These can be increased upto a maximum of 500 by setting it in the control files <b>concurrencylocal</b> for local deliveries and <b>concurrencyremote</b> for remote deliveries. These two (like any other indimail control files) lie in <u>/etc/indimail/control</u> directory. These concurrency limits are inherited by each of the indimail's multiple queues. Additionally indimail allows you to have queue specific concurrency limits. e.g. You can have the control files <b>concurrencyl.queue2</b>, <b>concurrencyr.queue2</b> for setting concurrency specific to <u>/var/indimail/queue/queue2</u>.

## Setting Environment Variables

indimail-mta can be fine tuned, configured using environment variables (> 200) of them. There are many methods of setting them.

1. Setting them in variables directory. All indimail services are configured as supervised services in <u>/service</u> directory. Each of these services has a directory named named after the service and a subdir inside it named <b>variables</b>. In the <b>variables</b> directory, you just need to create a file to create an environment variable. The name of the environment variable is the filename and the value of the environment variable is the content of the file. An empty file, removes the environment variable. As an exercise, explore the directory <u>/service/qmail-smtpd.25/variables</u>. All IndiMail services use the program <b>envdir</b>(8) to set environment variables from the <b>variables</b> directory.
2. Using control files <b>from.envrules</b>, <b>fromd.envrules</b>, <b>rcpt.envrules</b>, <b>auth.envrules</b> - These are control files used by programs like <b>qmail-smtpd</b>, <b>qmail-inject</b>. They match on the sender or recipient address. Here you can set or unset environment variables for a sender, recipient or any regular expression to match multipler sender or recipients. To know these environment variables, read the man pages for <b>qmail-smtpd</b>, <b>qmail-inject</b>, <b>spawn-filter</b>.
3. Using control file domainqueue - This can be used to set environment variable for any recipient domain. Read the man page for <b>qmail-smtpd</b>, <b>qmail-inject</b>.
4. Using environment directory <u>/etc/indimail/control/defaultqueue</u> - This is just like the supervise <b>variables</b> directory. The environment variables configured in this directory get used when calling <b>qmail-inject</b>, sendmail for injecting mails into the queue. Read the man page for <b>qmail-inject</b>.
5. Using environment directory <u>$HOME/.defaultqueue</u> - This is just like the supervise <b>variables</b> directory. The environment variables configured in this directory get used when calling <b>qmail-inject</b>, sendmail for injecting mails into the queue. Here $HOME refers to the home directory of the user. Read the man page for <b>qmail-inject</b>.
6. Nothing prevents a user from writing a shell script to set environment variables before calling any of indimail-mta programs. If you are familiar with UNIX, you will know how to set them.

## Notes

Currently queueX/info/split/<u>inode</u> serves two purposes: first, it records the envelope sender; second, its modification time is used to decide when a message has been in the queue too long. In the future queueX/info/split/<u>inode</u> may store more information. Any non-backwards-compatible changes will be identified by version numbers.

When <b>qmail-queue</b> has successfully placed a message into the queue, it pulls a trigger offered by <b>qmail-send</b>. Here is the current triggering mechanism: lock/trigger is a named pipe. Before scanning todo/, <b>qmail-send</b> opens lock/trigger O\_NDELAY for reading. It then selects for readability on lock/trigger. <b>qmail-queue</b> pulls the trigger by writing a byte O\_NDELAY to lock/trigger. This makes lock/trigger readable and wakes up <b>qmail-send</b>. Before scanning todo/ again, <b>qmail-send</b> closes and reopens lock/trigger.

# IndiMail Queue Mechanism

IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration. To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance. Multiple queues eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). When you have massive injecting rates, your software may place multiple files in a single directory. This drastically reduces file system performance. IndiMail avoids this by injecting your email in a queue consisting of multiple directories and mails distributed as evenly as possible across these directories.

Balancing of emails across multiple queues is achieved by the program <b>qmail-multi</b>, which is actually just a <b>qmail-queue</b>(8) replacement. Any <b>qmail-queue</b> frontend can use <b>qmail-multi</b>. The list of <b>qmail-queue</b> frontends in IndiMail are

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

You just need to configure the following environment variables to have the <b>qmail-queue</b>(8) frontends use <b>qmail-multi</b>(8)

1. QUEUE\_BASE – Base directory where all queues will be placed
2. QUEUE\_COUNT – number of queues
3. QUEUE\_START – numeric prefix of the first queue
4. QMAILQUEUE - Path of <b>qmail-multi</b> (/usr/sbin/qmail-multi)
5. QUEUEDIR - Not required if you set QUEUE\_BASE, QUEUE\_COUNT, QUEUE\_START. Else it should be full path to a queue (e.g. /var/indimail/queue/queue3). If QUEUEDIR is set, then QUEUE\_BASE, QUEUE\_COUNT and QUEUE\_START are not used.

<b>qmail-multi</b>'s job is to select a queue. indimail-mta uses multiple queue. If QUEUEDIR is set, it execs <b>qmail-queue</b> without doing anything. If QUEUEDIR is not defined, it uses QUEUE\_START, QUEUE\_COUNT and QUEUE\_BASE environment variables to set QUEUEDIR environment variable. QUEUE\_BASE is the common basename for all the queues. e.g. QUEUE\_BASE=/var/indimail/queue. Now, if QUEUE\_START is 1, QUEUE\_COUNT is 5, then <b>qmail-multi</b> will use the time to do a modulus operation and get a number ranging from 1 to 5. It will then set the QUEUEDIR environment variable to set any of the 5 queues in /var/indimail/queue. e.g. QUEUEDIR=/var/indimail/queue/queueX, where X is the number selected between 1 to 5. It then does a exec of <b>qmail-queue</b>. Throughout this document we will abbreviate /var/indimail/queue/queueX as queueX.

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

Now all you need is restart of all services to use the new QUEUE\_BASE, QUEUE\_COUNT, QUEUE\_START environment variables

```
$ sudo svc -d /service/qmail-smtpd* /service/qmail-send.25 /service/qmail-qm?pd.*
$ sudo svc -u /service/qmail-smtpd* /service/qmail-send.25 /service/qmail-qm?pd.*
```

indimail-mta also has a special queue <b>slowq</b> where the emails injected into this queue, the deliveries can be rate controlled. This is achived by setting the environment variable <b>QUEUEDIR</b>=<u>/var/indimail/queue/slowq</u>. indimail-mta provies you various [methods](#setting-environment-variables) to set environment variables. One of the method is using <b>domainqueue</b> control file discussed in [Controlling Delivery Rates](#controlling-delivery-rates).

# Using systemd to start IndiMail

IndiMail compoments get started by indimail-mta service. It does not have it's own startup service, but rather places all its services under indimail-mta's svscan directory /service.

[systemd](http://en.wikipedia.org/wiki/Systemd "systemd") is a system and service manager for Linux, compatible with SysV and LSB init scripts. systemd provides aggressive parallelization capabilities, uses socket and D-Bus activation for starting services, offers on-demand starting of daemons, keeps track of processes using Linux cgroups, supports snapshots and restoring of the system state, maintains mount and automount points and implements an elaborate transactional dependency-based service control logic. It can work as a drop-in replacement for sysvinit.

The first step is to write the service configuration file for IndiMail in /lib/systemd/system/svscan.service

```
[Unit]
Description=IndiMail Messaging Platform
After=local-fs.target network.target

[Service]
ExecStart=/usr/libexec/indimail/svscanboot /service
ExecStop=/etc/init.d/indimail stop
Restart=on-failure
Type=simple

[Install]
Alias=indimail.service
Alias=indimail-mta.service
WantedBy=multi-user.target
```

You can override values in the above file by creating a file override.conf in /etc/systemd/system/svscan.service.d. As an example, on a raspberry pi system, you should have svscan started only after the system clock is synchronized with a NTP source (many SBC don't have battery backed Real Time Clock - RTC). This ensures that svscan gets started when the system has a correct date, time so that logs created will not have absurd timestamps.

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

Now to start IndiMail you can use any of the below commands.

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
# /usr/sbin/initsvc -off   (to disable indimail service)
```

You can now also query the status of the running IndiMail service by using the systemctl command

```
# systemctl status indimail.service
● svscan.service - SVscan Service
     Loaded: loaded (/usr/lib/systemd/system/svscan.service; enabled; vendor preset: disabled)
     Active: active (running) since Mon 2021-05-31 08:09:37 IST; 9h ago
   Main PID: 2416 (svscan)
      Tasks: 227 (limit: 9422)
     Memory: 901.7M
     CGroup: /system.slice/svscan.service
             ├─ 2416 /usr/sbin/svscan /service
             ├─ 2426 supervise log .svscan
             ├─ 2428 supervise qmail-qmqpd.628
             ├─ 2429 supervise log qmail-qmqpd.628
             ├─ 2430 supervise resolvconf
             ├─ 2431 supervise log resolvconf
             ├─ 2432 supervise mpdev
             ├─ 2433 supervise log mpdev
             ├─ 2434 supervise qmail-imapd.143
             ├─ 2435 supervise log qmail-imapd.143
             ├─ 2436 supervise qmail-poppass.106
             ├─ 2437 supervise log qmail-poppass.106
             ├─ 2438 supervise qmail-daned.1998
             ├─ 2439 supervise log qmail-daned.1998
             ├─ 2440 supervise qmail-imapd-ssl.993
             ├─ 2441 supervise log qmail-imapd-ssl.993
             ├─ 2442 supervise qmail-smtpd.25
             ├─ 2443 supervise log qmail-smtpd.25
             ├─ 2444 supervise indisrvr.4000
             ├─ 2445 supervise log indisrvr.4000
             ├─ 2446 supervise qmail-pop3d.110
             ├─ 2447 supervise log qmail-pop3d.110
             ├─ 2448 supervise slowq-send
             ├─ 2449 supervise log slowq-send
             ├─ 2450 supervise update
             ├─ 2451 supervise log update
             ├─ 2452 supervise qmail-smtpd.366
             ├─ 2453 supervise log qmail-smtpd.366
             ├─ 2454 supervise freshclam
             ├─ 2455 supervise log freshclam
             ├─ 2456 supervise fclient
             ├─ 2457 supervise log fclient
             ├─ 2458 supervise qmail-pop3d-ssl.995
             ├─ 2459 supervise log qmail-pop3d-ssl.995
             ├─ 2460 supervise qmail-qmtpd.209
             ├─ 2461 supervise log qmail-qmtpd.209
             ├─ 2462 supervise mrtg
             ├─ 2463 supervise log mrtg
             ├─ 2464 supervise rsync.873
             ├─ 2465 supervise log rsync.873
             ├─ 2466 supervise qmail-logfifo
             ├─ 2467 supervise log qmail-logfifo
             ├─ 2468 supervise qscanq
             ├─ 2469 supervise log qscanq
             ├─ 2470 supervise qmail-send.25
             ├─ 2471 supervise log qmail-send.25
             ├─ 2472 supervise greylist.1999
             ├─ 2473 supervise log greylist.1999
             ├─ 2474 supervise qmail-smtpd.465
             ├─ 2475 supervise log qmail-smtpd.465
             ├─ 2476 supervise proxy-imapd.4143
             ├─ 2477 supervise log proxy-imapd.4143
             ├─ 2478 supervise fetchmail
             ├─ 2479 supervise log fetchmail
             ├─ 2480 supervise qmail-smtpd.587
             ├─ 2481 supervise log qmail-smtpd.587
             ├─ 2482 supervise pwdlookup
             ├─ 2483 supervise log pwdlookup
             ├─ 2484 supervise proxy-imapd-ssl.9143
             ├─ 2485 supervise log proxy-imapd-ssl.9143
             ├─ 2486 supervise dnscache
             ├─ 2487 supervise log dnscache
             ├─ 2488 supervise proxy-pop3d-ssl.9110
             ├─ 2489 supervise log proxy-pop3d-ssl.9110
             ├─ 2490 supervise inlookup.infifo
             ├─ 2491 supervise log inlookup.infifo
             ├─ 2492 supervise clamd
             ├─ 2493 supervise log clamd
             ├─ 2494 supervise proxy-pop3d.4110
             ├─ 2495 supervise log proxy-pop3d.4110
             ├─ 2496 supervise mysql.3306
             ├─ 2497 supervise log mysql.3306
             ├─ 2498 supervise udplogger.3000
             ├─ 2499 supervise log udplogger.3000
             ├─ 2500 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 9110 /usr/bin/couriertls -server -tcpd /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 2501 /usr/bin/dnscache
             ├─ 2502 /usr/bin/freshclam -v --stdout --datadir=/var/indimail/clamd -d -c 2 --config-file=/etc/freshclam.conf
             ├─ 2503 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 9143 /usr/bin/couriertls -server -tcpd /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 2504 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 995 /usr/bin/couriertls -server -tcpd /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/bin/pop3d Maildir
             ├─ 2505 /usr/sbin/mysqld --defaults-file=/etc/indimail/indimail.cnf --port=3306 --basedir=/usr --datadir=/var/indimail/mysqldb/data --memlock --ssl --require-secure-transport --skip-external-locking --delay-key-write=all --skip-name-resolve --sql-mode=NO_ENGINE_SUBSTITUTION,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,STRICT_TRANS_TABLES --explicit-defaults-for-timestamp=TRUE --general-log=1 --general-log-file=/var/indimail/mysqldb/logs/general-log --slow-query-log=1 --slow-query-log-file=/var/indimail/mysqldb/logs/slowquery-log --log-queries-not-using-indexes --log-error-verbosity=3 --pid-file=/var/run/mysqld/mysqld.3306.pid
             ├─ 2506 /bin/sh ./run
             ├─ 2509 /usr/bin/qmail-cat /run/indimail/logfifo
             ├─ 2513 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 143 /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/bin/imapd Maildir
             ├─ 2514 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 110 /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/bin/pop3d Maildir
             ├─ 2519 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.poppass.cdb -X -c variables/MAXDAEMONS -C 25 -o -b 40 -n /etc/indimail/certs/servercert.pem -u 555 -g 555 0 106 /usr/sbin/qmail-poppass argos.indimail.org /usr/sbin/vchkpass /bin/false
             ├─ 2520 multilog t ./main
             ├─ 2521 /usr/sbin/indisrvr -i 0 -p 4000 -b 40 -t 300 -n /etc/indimail/certs/servercert.pem
             ├─ 2522 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 4110 /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 2523 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 993 /usr/bin/couriertls -server -tcpd /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authpwd /usr/libexec/indimail/imapmodules/authshadow /usr/libexec/indimail/imapmodules/authpam /usr/bin/imapd Maildir
             ├─ 2526 /usr/bin/tcpserver -v -c variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l argos.indimail.org -R -u 555 -g 555 0 4143 /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 2557 /usr/sbin/multilog t /var/log/svc/update
             ├─ 2558 /usr/sbin/multilog t /var/log/svc/slowq
             ├─ 2561 /usr/sbin/multilog t /var/log/svc/svscan
             ├─ 2564 /usr/sbin/multilog t /var/log/svc/smtpd.465
             ├─ 2565 /usr/sbin/multilog t /var/log/svc/proxyPOP3.4110
             ├─ 2566 /usr/sbin/multilog t /var/log/svc/resolvconf
             ├─ 2567 /usr/sbin/multilog t /var/log/svc/proxyIMAP.4143
             ├─ 2568 /usr/sbin/multilog t /var/log/svc/smtpd.25
             ├─ 2569 /usr/sbin/multilog t /var/log/svc/pop3d-ssl.995
             ├─ 2570 /usr/sbin/multilog t /var/log/svc/pop3d.110
             ├─ 2571 /usr/sbin/multilog t /var/log/svc/inlookup.infifo
             ├─ 2572 /usr/sbin/multilog t /var/log/svc/imapd-ssl.993
             ├─ 2573 /usr/sbin/multilog t /var/log/svc/indisrvr.4000
             ├─ 2574 /usr/sbin/multilog t /var/log/svc/imapd.143
             ├─ 2575 /usr/sbin/multilog t /var/log/svc/fetchmail
             ├─ 2576 /usr/sbin/multilog t /var/log/svc/deliver.25
             ├─ 2583 /usr/sbin/multilog t /var/log/svc/fclient
             ├─ 2584 /usr/sbin/multilog t /var/log/svc/proxyIMAP.9143
             ├─ 2615 /usr/sbin/multilog t /var/log/svc/proxyPOP3.9110
             ├─ 2616 /usr/sbin/multilog t /var/log/svc/greylist.1999
             ├─ 2617 /usr/sbin/multilog t /var/log/svc/daned.1998
             ├─ 2618 /usr/sbin/multilog t /var/log/svc/qscanq
             ├─ 2619 /usr/sbin/multilog t /var/log/svc/poppass.106
             ├─ 2620 /usr/sbin/multilog t /var/log/svc/mrtg
             ├─ 2621 /usr/sbin/multilog t /var/log/svc/logfifo
             ├─ 2622 /usr/sbin/multilog t /var/log/svc/smtpd.366
             ├─ 2623 /usr/sbin/multilog t /var/log/svc/rsyncd.873
             ├─ 2635 /usr/sbin/multilog t /var/log/svc/mysql.3306
             ├─ 2636 /usr/sbin/multilog t /var/log/svc/udplogger.3000
             ├─ 2637 /usr/sbin/multilog t /var/log/svc/freshclam
             ├─ 2638 /usr/sbin/multilog t /var/log/svc/clamd
             ├─ 2640 /usr/sbin/multilog t /var/log/svc/qmqpd.628
             ├─ 2641 /usr/sbin/multilog t /var/log/svc/qmtpd.209
             ├─ 2642 /usr/sbin/multilog t /var/log/svc/mpdev
             ├─ 2643 /usr/sbin/multilog t /var/log/svc/pwdlookup
             ├─ 2644 /usr/sbin/multilog t /var/log/svc/smtpd.587
             ├─ 2901 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2902 /usr/sbin/nssd -d notice
             ├─ 2911 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2912 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2913 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2914 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2915 /usr/sbin/inlookup -i 5 -c 5184000
             ├─52089 /usr/sbin/qmail-greyd -w /etc/indimail/control/greylist.white -t 30 -g 24 -m 2 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/greylist.context
             ├─52093 /usr/sbin/qmail-daned -w /etc/indimail/control/tlsa.white -t 30 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/tlsa.context
             ├─52098 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.qmtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 209 /usr/sbin/qmail-qmtpd
             ├─52103 /usr/sbin/qmail-daemon ./Maildir/
             ├─52112 /usr/bin/tcpserver -v -h -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 25 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so
             ├─52118 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 150 -u 555 -g 555 0 366 /usr/sbin/qmail-smtpd
             ├─52123 qmail-send
             ├─52124 qmail-send
             ├─52125 qmail-send
             ├─52126 qmail-send
             ├─52128 qmail-send
             ├─52131 /usr/bin/tcpserver -v -h -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 465 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so argos.indimail.org /usr/sbin/sys-checkpwd /usr/sbin/vchkpass /bin/false
             ├─52136 /usr/bin/tcpserver -v -H -R -l argos.indimail.org -x /etc/indimail/tcp/tcp.smtp.cdb -c variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 587 /usr/lib/indimail/plugins/qmail_smtpd.so argos.indimail.org /usr/sbin/sys-checkpwd /usr/sbin/vchkpass /bin/false
             ├─52139 qmail-lspawn ./Maildir/
             ├─52140 qmail-rspawn
             ├─52141 qmail-clean
             ├─52142 qmail-todo
             ├─52143 qmail-clean
             ├─52144 slowq-send
             ├─52149 /usr/sbin/cleanq -l -s 200 /var/indimail/qscanq/root/scanq
             ├─52151 qmail-lspawn ./Maildir/
             ├─52152 qmail-rspawn
             ├─52153 qmail-clean
             ├─52156 qmail-todo
             ├─52158 qmail-clean
             ├─52160 /usr/sbin/udplogger -p 3000 -t 10 0
             ├─52172 qmail-lspawn ./Maildir/
             ├─52173 qmail-rspawn
             ├─52174 qmail-clean
             ├─52175 qmail-todo
             ├─52176 qmail-clean
             ├─52178 qmail-lspawn ./Maildir/
             ├─52179 qmail-rspawn
             ├─52180 qmail-clean
             ├─52181 qmail-todo
             ├─52182 qmail-clean
             ├─52186 qmail-lspawn ./Maildir/
             ├─52188 qmail-rspawn
             ├─52189 qmail-clean
             ├─52190 qmail-todo
             ├─52191 qmail-clean
             ├─52197 qmail-lspawn ./Maildir/
             ├─52198 qmail-rspawn
             ├─52199 qmail-clean
             └─55890 sleep 300
```

IndiMail also proves the svps command which gives a neat display on the status of all services configured for IndiMail. You can omit the -a flag to omit the logging processes.

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
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
```

If you do not use <b>vdelivermail</b> and want to use your own delivery agent? Fear not by using <b>ismaildup</b>(1). <b>ismaildup</b> expects the email on standard input and is easily scriptable like the example below in a .qmail file.

`|ismaildup /usr/bin/maildirdeliver /home/manny/Maildir/`

will deliver mails to /home/manny/Maildir while discarding duplicates.

If you are not happy with the 900 seconds (15 minutes) time interval for checking duplicates, you can change it by setting the DUPLICATE_INTERVAL environment variable. The following will not allow a single duplicate to be entertained within 24 hours

```
$ sudo /bin/bash
# echo 86400 > /service/qmail-send.25/variables/DUPLICATE_INTERVAL
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
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
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25

NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).
```

### Using QMAILQUEUE with qmail-qfilter

You can use <b>qmail-qfilter</b>(1). <b>qmail-qfilter</b> allows you to run multiple filters passed as command line arguments to <b>qmail-qfilter</b>. Since QMAILQUEUE doesn't allow you to specify multiple arguments, you can write a shell script which calls <b>qmail-qfilter</b> and have the shell script defined as QMAILQUEUE environment variable.

```
$ sudo /bin/bash
# echo /usr/bin/qmail-qfilter /usr/local/bin/myfilter > /usr/local/bin/qfilter
# chmod +x /usr/local/bin/qfilter
# echo /usr/local/bin/qfilter > /service/qmail-smtpd.25/variables/QMAILQUEUE
# echo /usr/bin/qmail-dk > /service/qmail-smtpd.25/variabels/QQF_QMAILQUEUE
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
```

NOTE: you can define QQF\_MAILQUEUE to /usr/bin/qmail-nullqueue to discard the mail (blackhole).

### Using QMAILQUEUE with your own program

When you want to use your own program as QMAILQUEUE, then your program is responsible for queuing the email. It is trivial to queue the email by calling <b>qmail-multi</b>(8). You script can read the stdin for the raw message (headers + body) and pipe the output (maybe after modifications) to <b>qmail-multi</b>(8). If you are doing DK/DKIM signing, you can execute <b>qmail-dk</b>(8) instead of <b>qmail-multi</b>(8). You can have <b>qmail-dk</b>(8) call <b>qmail-dkim</b>(8) and <b>qmail-dkim</b>(8) calls <b>qmail-multi</b>(8). Assuming you want to do DK/DKIM signing, and myfilter calls <b>qmail-dk</b>(8), you can do the following

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-smtpd.25/variables/QMAILQUEUE
# echo /usr/bin/qmail-dkim > /service/qmail-smtpd.25/variables/DKQUEUE
# echo /usr/bin/qmail-multi > /service/qmail-smtpd.25/variables/DKIMQUEUE
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
```

NOTE: You can set the environment variable NULLQUEUE before calling <b>qmail-multi</b> to discard the mail completely (blackhole).

## Filtering during local / remote delivery

### Using FILTERARGS environment variable

The below configuration causes all local / remote deliveries to be fed through the filter /usr/local/bin/myfilter. You can use the programs 822header(1), 822body(1) inside myfilter to get and manipulate the headers and body.

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-send.25/variables/FILTERARGS
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
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
e.g. The following entry in /var/indimail/control/filterargs causes all mails to yahoo.com be fed through the filter dk-filter(8) for DK/DKIM signing.
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
exec /usr/bin/qmail-multi
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

The delivery mode depends on the argument passed to qmail-daemon during startup. The script /service/qmail-send.25/run passes the content of the file /etc/indimail/control/defaultdelivery as an argument to qmail-daemon.
See INSTALL.mbox, INSTALL.maildir, and INSTALL.vsm for more information.
To select your default mailbox type, just enter the defaultdelivery value from the table into /var/indimail/control/defaultdelivery.
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

<b>qmail-send</b> has the ability to control delivery rates of jobs scheduled for delivery. Every queue can have a directory named <b>ratelimit</b> where you can store rate control definition for a domain. These rate control definition files can be created using the <b>drate</b> command. All you need to provide is a mathematical expression that defines your rate. e.g. 100/3600 means 100 emails per hour. This rate control works at the queue level. indimail-mta uses multiple queues. Each queue has its own delivery handled by by its own set of <b>qmail-todo</b>, <b>qmail-send</b>, <b>qmail-lspawn</b>, <b>qmail-rspawn</b>, <b>qmail-clean</b> daemons. Enforcing rate control on such a queue has a practical problem of having to define rate control definition for each and every queue.

To avoid the practical problem of rate limiting every queue, we can use a dedicated queue to handle rate controllled delivery. The default indimail installation creates a special queue named as <b>slowq</b> and uses a daemon <b>slowq-send</b> instead of <b>qmail-send</b> to process the <u>slowq</u> queue. This is done by calling <b>slowq-start<b> instead of <b>qmail-daemon</b> / <b>qmail-start</b>. <b>slowq-send</b> doesn't have its own <b>qmail-todo</b> process. We don't require it as we aren't looking at high delivery rates for this queue. <b>slowq-start</b> is invoked using a supervised service in <u>/service/slowq</u>.

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

# Processing Bounces

IndiMail allows a mechanism by which you can use your own script/program to handle bounces. All bounces in IndiMail is generated by <b>qmail-send</b>. <b>qmail-send</b> generates a bounce when <b>qmail-lspawn</b> or <b>qmail-rspawn</b> reports a permanent failed delivery. A bounce is generated by <b>qmail-send</b> by injecting a new mail in the queue using <b>qmail-queue</b>. This bounce generation by <b>qmail-send</b> can be modified in three ways

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
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
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
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
```

disables bounces system-wide. Though disabling bounces may not be the right thing to do but in some situations where bounces are not at all needed, disabling bounces will surely result in performance improvements of your system, especially so if your system does mass-mailing.

# Delivery Instructions for a virtual domain

IndiMail uses a modified version of qmail as the MTA. For local deliveries, <b>qmail-lspawn</b> reads a series of local delivery commands from descriptor 0, invokes <b>qmail-local</b> to perform the deliveries. <b>qmail-local</b> reads a mail message and delivers to to a user by the procedure described in dot-qmail(5). IndiMail uses <b>vdelivermail</b> as the local delivery agent.
A virtual domain is created by the command vadddomain(1).

`$ sudo vadddomain example.com some_password`

The above command creates a virtual domain with delivery instructions in /var/indimail/domains/example.com/.qmail-default file. A line in this file is of the form

```
/usr/sbin/vdelivermail '' delivery_instruction_for_non_existing_user
```

The `delivery\_instruction\_for\_non\_existing\_user` can have one of the following 5 forms

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
# svc -d /service/qmail-smtpd.587
# svc -u /service/qmail-smtpd.587
```

Read **altermime**(1) man page for more details

# Email Archiving

IndiMail provides multiple options for those who want their emails archived automatically. For easy retrieval, you can use tools like google desktop, beagle, etc If you use IndiMail, you have two methods to achieve automatic archiving of emails

## using environment variable EXTRAQUEUE

If EXTRAQUEUE environment variable is set to any environment variable, <b>qmail-queue</b> will deposit an extra copy of the email which it receives for putting it in the queue. Normally you would set EXTRAQUEUE variable in any of the clients which use <b>qmail-queue</b>. e.g. <b>qmail-smtpd</b>, <b>qmail-inject</b>, sendmail, etc. If you have setup IndiMail as per the official instructions, you can set EXTRAQUEUE for incoming and outgoing mails as given below

```
$ sudo /bin/bash
# echo "archive@example.com" > /service/qmail-smtpd.25/variables/EXTRAQUEUE
# svc -d /service/qmail-smtpd.25 /service/qmail-smtpd.587
# svc -u /service/qmail-smtpd.25 /service/qmail-smtpd.587
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

IndiMail allows you to configure most of its functionality through set of environment variables. In fact there more more than 200 features that can be controlled just by setting or un-setting environment variables. envrules is applicable to <b>qmail-smtpd</b>, <b>qmail-inject</b>, <b>qmail-local</b>, <b>qmail-remote</b> as well. It can also be used to control programs called by the above programs (e.g <b>qmail-queue</b>). IndiMail allows you to configure quite many things using environment variables. Just set the environment variable <b>CONTROLDIR=control2</b> and all qmail components of IndiMail start looking for control files in <u>/var/indimail/control2</u>. You can set <b>CONTROLDIR=/etc/indimail</b> and all control files can be conveniently placed in <u>/etc/indimail</u>.
Some of these environment variables can be set during the startup of various services. IndiMail has all its services configured as directories in the /service directory. As an example, if you want to force authenticated SMTP on all your users, setting the environment variable <b>REQUIREAUTH</b> allows you to do so.

```
$ sudo /bin/bash
# echo 1 > /service/qmail-smtpd.587/variables/REQUIREAUTH
# svc -d /service/qmail-smtpd.587
# svc -u /service/qmail-smtpd.587
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
REQUIREAUTH, QREGEX, ENFORCE_FQDN_HELO, DATABYTES, BADHELOCHECK, BADHELO, BADHOST, BADHOSTCHECK, TCPPARANOID, NODNSCHECK, VIRUSCHECK, VIRUSFORWARD, REMOVEHEADERS, ENVHEADERS, LOGHEADERS, LOGHEADERFD, SIGNATURES, BODYCHECK, BADMAILFROM, BADMAILFROMPATTERNS, BOUNCEMAIL, CUGMAIL, MASQUERADE, BADRCPTTO, BADRCPTPATTERNS, GOODRCPTTO, GOODRCPTPATTERNS, GREYIP, GREETDELAY, CLIENTCA, TLSCIPHERS, SERVERCERT, BLACKHOLERCPT, BLACKHOLERCPTPATTERNS, SIGNKEY, SIGNKEYSTALE, SPFBEHAVIOR, TMPDIR, TARPITCOUNT, TARPITDELAY, MAXRECIPIENTS, MAX_RCPT_ERRCOUNT, AUTH_ALL, CHECKRELAY, CONTROLDIR, ANTISPOOFING, CHECKRECIPIENT, SPAMFILTER, LOGFILTER, SPAMFILTERARGS, SPAMEXITCODE, REJECTSPAM, SPAMREDIRECT, SPAMIGNORE, SPAMIGNOREPATTERNS, FILTERARGS, QUEUEDIR, QUEUE_BASE, QUEUE_START, QUEUE_COUNT, QMAILQUEUE, QUEUEPROG, RELAYCLIENT, QQEH, BADEXT, BADEXTPATTERNS, ACCESSLIST, EXTRAQUEUE, QUARANTINE, QHPSI, QHPSIMINSIZE, QHPSIMAXSIZE, QHPSIRC, QHPSIRN, USE_FSYNC, SCANCMD, PLUGINDIR, QUEUE_PLUGIN, PASSWORD_HASH, MAKESEEKABLE, MIN_FREE, ERROR_FD, DKSIGN, DKVERIFY, DKSIGNOPTIONS, DKQUEUE, DKEXCLUDEHEADERS, DKIMSIGN, DKIMVERIFY, DKIMPRACTICE, DKIMIDENTITY, DKIMEXPIRE, SIGN_PRACTICE DKIMQUEUE, SIGNATUREDOMAINS, and NOSIGNATUREDOMAINS
```

The following list of environment variables can be modified using envrules if QMAILLOCAL and QMAILREMOTE is set to /var/indimail/bin/spawn-filter.

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

indimail-mta has multiple daemons qmail-daemon/qmail-start, qmail-send, qmail-lspawn, qmail-rspawn and qmail-clean for processing a queue. The standard indimail-mta installation is meant for servers that can withstand high loads resulting from high inbound/outbound mail traffic. For small servers which have minimal or sporadic traffic, the full indimail-mta installation isn't required or necessary. In such cases you can either install indimail-mini to use QMQP protocol or qmta to use standard delivery mechanisms. indimail-mini comprises of just 10 binaries, whereas qmta comprises of 25 binaries. For most cases, you just require one binary <b>qmail-qmqpc</b> if you use indimail-mini. If you use qmta, for most cases you just require <b>qmta-send</b> and <b>qmail-queue</b> to send process and send mails.

## indimail-mini - Using QMQP protocol provided by qmail-qmqpc / qmail-qmqpd

QMQP provides a centralized mail queue within a cluster of hosts. QMQP clients do not require local queue for queueing messages. QMQP is faster than SMTP. You can use QMQP to send mails from your relay servers (or any server) to a server running QMQP service. The QMQP service can deliver mails to your local mailboxes or/and relay mails to the outside world, using a server running QMQP servic, using a server running QMQP servicee. To use QMQP, you need a client that uses QMQP protocol and a server that provides the QMQP service. <b>qmail-qmqpc</b> which uses [QMQP](https://cr.yp.to/proto/qmqp.html) protocol, is a QMQP client. <b>qmail-qmqpd</b> is a server that provides a central hub offering the QMQP service. You can transfer both local and remote mails through the central hub. The central hub can also be a collection of multiple servers to aid performance. The QMQP protocol doesn't require clients to have a local queue. In fact you can use QMQP protocol on a truly diskless client which doesn't even have any remote filesystem mounted. If you run web servers which need to send out emails, you can use QMQP to send emails without impacting performance. This is because you don't have the overhead of running an MTA like sendmail, postfix or qmail/netqmail/notqmail/indimail-mta. If you already have an installation of indimail-mta on your network, you can quickly setup a indimail-mini installation on your web server. The sendmail client provided by indimail-mini is fully compatible with PHP, python and any software that uses sendmail interface to push out emails. QMQP service is provided by enabling qmail-qmqpd service on a server having indimail-mta installed. All other servers (including your webservers) can have a indimail-mini installation to use qmail-qmqpc to push emails to the central hub. The process for setting up QMQP service is outlined in the next chapter.

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
$ sudo /usr/sbin/svctool --qmqp=628 --servicedir=/service \
  --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
  --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --fsync --syncdir --memory=104857600 --min-free=52428800
```

The above command will create a supervised service which runs qmail-qmqpd under tcpserver. In case you are setting up this service to relay mails to outside world, you might want to also specify --dkfilter, --qhpsi, --virus-filter, etc arguments to svctool(8) so that tasks like virus scanning, dk, domainkey signing, etc is done by the QMQP service.

Note: Some of the tasks like virus/spam filtering, dk, dkim signing, etc can be done either by the client (if `QMAILQUEUE=/usr/sbin/qmail-multi`), or can be performed by QMQP service if **QMAILQUEUE** is defined as <b>qmail-multi</b> in the service's variable directory.

A QMQP server shouldn't even have to glance at incoming messages; its only job is to queue them for <b>qmail-send</b>(8). Hence you should allow access to QMQP service only from your authorized clients. You can edit the file /etc/indimail/tcp.qmqp to grant specific access to clients. Here's how to set up QMQP service to authorized client hosts on your indimail-mta server.

first create /etc/indimail/tcp.qmqp in tcprules format to allow queueing from the authorized hosts. make sure to deny connections from unauthorized hosts. for example, if queueing is allowed from 1.2.3.\*:

```
1.2.3.:allow
:deny
```

Then create /etc/indimail//tcp/tcp.qmqp.cdb:

```
$ sudo qmailctl cdb
```

You can change /var/indimail/etc/tcp.qmqp and run tcprules again at any time.

### Client Setup - How do I install indimail-mini to use qmail-qmqpc

A indimail-mini installation is just like a indimail installation, except that it's much easier to set up:

* You don't need MySQL
* You don't need /var/indimail/alias. A indimail-mini installation doesn't do any local delivery.
* You don't require daemontools, ucspi-tcp. You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail.
* You don't need indimail entries in /etc/group or /etc/passwd. indimail-mini runs with the same privileges as the user sending mail; it doesn't have any of its own files.
* You don't need to start anything from your boot scripts. indimail-mini doesn't have a queue, so it doesn't need a long-running queue manager.

Installation and setup is trivial if you use the RPM package.

```
$ sudo rpm -ivh indimail-mini
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
* shared libs that the above binaries reference. You can use the ldd command (or otool -L command on OSX) (for source installation only).
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail (for source installation only);
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision. Everything can be shared across hosts except for /etc/indimail/control/idhost.
* Setup QMAILQUEUE environment variable to have qmail-qmqpc called instead of qmail-queue when any client injects mails in the queue (for source installation only).
  ```
  # mkdir /etc/indimail/control/defaultqueue
  # cd /etc/indimail/control/defaultqueue
  # echo qmail-qmqpc > QMAILQUEUE
  ```
* All manual pages for the above binaries (not a hard requirement but good for future reference) (for source installation only).

Remember that users won't be able to send mail if all the QMQP servers are down. Most sites have two or three independent QMQP servers.

Note that users can still use all the <b>qmail-inject</b> environment variables to control the appearance of their outgoing messages. This will include environment variables in $HOME/.defaultqueue directory.

If you want to setup a SMTP service, it might be easier to install the entire indimail-mta package and remove the services qmail-send.25. You can use svctool to remove the service e.g.

`$ sudo /usr/sbin/svctool --rmsvc qmail-send.25`

In case you setup SMTP service, you may want to handle tasks is dkim, virus and spam filtering. You can use QHPSI along with a virus scanner like clamav. You can also choose not to have these tasks done at the client end, but rather have it carried out by the QMQP service. For virus scanning refer to chapter [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi). You can set QMAILQUEUE to qmail-multi, qmail-dkim, etc. However, you must remember to have qmail-qmqpc called at the end in case you change QMAILQUEUE to something other than qmail-qmqpc.

## qmta - Using a minimal standalone qmta-send MTA

<b>qmta-send</b> can work like <b>qmail-send</b> to transfer local and remote mails, but unlike qmail-send, it doesn't require mutiple daemons (qmail-todo, qmail-lspawn/qmail-rspawn, qmail-clean) to process the queue. In case you don't have a central host running QMQP (provided by qmail-qmqpd) or you have a small host that sends out insignificant number of emails in a day, then <b>qmta-send</b> is what you would want to setup. <b>qmta-send</b> is well suited for tiny computers like raspberry pi, banana pi and other [Single Board Computers](https://en.wikipedia.org/wiki/Single-board_computer).

![image](indimail_mini.png)

### How do I set up a standalone MTA using qmta-send

A qmta installation using qmta-send is just like an indimail-mini installation, except that it has a single queue named <u>qmta</u>. This queue is processed by a single daemon <b>qmta-send</b>. <b>qmta-send</b> is like <b>qmail-send</b> provided by indimail-mta, except that it doesn't require multiple daemons - <b>qmail-todo</b>, <b>qmail-lspawn</b>/<b>qmail-rspawn</b>, <b>qmail-clean</b>. Just like indimail-mini,

* You don't require MySQL
* You don't require daemontools, ucspi-tcp. You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail.

Installation and setup is trivial if you use the RPM package.

```
$ sudo rpm -ivh qmta
```

If you are doing a source installation then you need to manually copy few binaries and few shared librareis. Here's what you do need if you want to setup from a source installation.

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
* shared libs that the above binaries reference. You can use the ldd command (or otool -L command on OSX) (for source installation only)
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
  3. If you have installed the qmta RPM/debian package, this will be already setup for you in systemd
* All manual pages for the above binaries (not a hard requirement but good for future reference).

Note that users can still use all the <b>qmail-inject</b> environment variables to control the appearance of their outgoing messages. Also you can setup environment variables in $HOME/.defaultqueue apart from /etc/indimail/control/defaultqueue

# Fedora - Using /usr/sbin/alternatives

Sometimes two or more Fedora package exist that serve the same purpose. The alternatives system provides a mechanism for selecting an active default application from several valid alternatives. You can use the alternatives system to configure as an alternative MTA for your system. Using alternatives, you don't have to create the links to /usr/bin/sendmail manually as instructed above.

```
$ sudo /usr/sbin/alternatives --install \
    /usr/sbin/sendmail mta /usr/bin/sendmail 120 \
    --slave /usr/share/man/man8/sendmail.8.gz mta-sendmailman \
    /usr/share/man/man8/qmail-inject.8.gz \
    --slave /usr/lib/sendmail mta-sendmail \
    /usr/bin/sendmail
    /usr/sbin/alternatives --set mta /usr/bin/sendmail
```

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
$ sudo /var/indimail/bin/vadduser test05@example.com
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
env - PATH="/usr/bin" tcpserver -H -R -x /etc/indimail/tcp.smtp.cdb \
  -c 20 -u 555 -g 555 0 smtp /var/indimail/bin/qmail-smtpd 2>&1
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

# Set up Authenticated SMTP

IndiMail also provides you authenticated SMTP providing **AUTH PLAIN**, **AUTH LOGIN**, **AUTH CRAM-MD5**, **CRAM-SHA1**, **CRAM-SHA256**, **CRAM-SHA512**, **CRAM-RIPEMD**, **DIGEST_MD5** methods. Whenever a user successfully authenticates through SMTP, the **RELAYCLIENT** environment variable gets set. <b>qmail-smtpd</b> uses the **RELAYCLIENT** environment variable to allow relaying.

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

IndiMail has a feature called **CHECKRECIPIENT** which allows indimail to check at SMTP, if the recipient to whom the mail is being addressed exists. It is always better to reject such users at SMTP rather than later during the actual delivery to the mailbox. Due to spam, in most of the cases, the Return Path will be forged or undeliverable. Hence you will be left with a condition where plenty of bounces will be left on your system, impacting the performance of your messaging system.

**CHECKRECIPIENT** can be also be used to reject mails for inactive users, overquota users and users who do not have the privilege to receive mails. **CHECKRECIPIENT** can be enabled by setting the environment variable **CHECKRECIPIENT** to one of the following values

1. Reject the user if not present in IndiMail's MySQL database
2. Reject the user if not present in IndiMail's MySQL database and recipients.cdb
3. Reject user if not present in recipients.cdb

You can selectively turn on **CHECKRECIPIENT** for selective domains by including those domains (prefixing the domain with '@' sign) in the control file /etc/indimail/control/chkrcptdomains.

If the environment variable MAX\_RCPT\_ERRCOUNT is set <b>qmail-smtpd</b> will reject an email if in a SMTP session, the number of such recipients who do not exist, exceed MAX\_RCPT\_ERRCOUNT.

**CHECKRECIPIENT** also causes the RCPT TO command to be delayed by 5 seconds for every non-existent recipient, to make harvesting of email addresses difficult.

If you do not have large number of users

```
$ sudo /bin/bash
# echo 1 > /service/qmail-smtpd.25/variables/CHECKRECIPIENT
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
# exit
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

Version 1.7.4 of indiMail and above comes with a utility named **qmail-sql** which allows you to create the MySQL table and also insert values from command line or convert an existing plain text version to MySQL.

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

On of the easiest method to enable bogofilter is to set few environment variable for indimail's <b>qmail-multi</b>(8) the frontend for <b>qmail-queue</b>(8) program. e.g. to enable spam filter on the incoming SMTP on port 25:

bogofilter requires training to work. You can refer to Section 3, Step 8 in the document [INSTALL](INSTALL-indimail.md). You can also have a pre-trained database installed by installing the **bogofilter-wordlist** package.

```
$ sudo /bin/bash
# echo "/usr/bin/bogofilter -p -d /etc/indimail" > /service/qmail-smtpd.25/variables/SPAMFILTER
# echo "0" > /service/qmail-smtpd.25/variables/SPAMEXITCODE
# echo "0" > /service/qmail-smtpd.25/variables/REJECTSPAM
# echo "1" > /service/qmail-smtpd.25/variables/MAKESEEKABLE
```

Now <b>qmail-multi</b>(8) will pass every mail will pass through bogofilter before it passes to <b>qmail-queue</b>(8). You can refer to chapter [IndiMail Queue Mechanism](#indimail-queue-mechanism) and look at the picture to understand how it works. bogofilter(1) will add X-Bogosity in each and every mail. A spam mail will have the value `Yes` along with a probabality number (e.g. 0.999616 below). You can configure bogofilter in /etc/indimail/bogofilter.cf. The SMTP logs will also have lines having this X-Bogosity field. A detailed mechanism is depicted pictorially in the chapter [Virus Scanning using QHPSI](#virus-scanning-using-qhpsi).

```
X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The method describe above is a global SPAM filter. It will happen for all users, unless you use something like **envrules** to unset **SPAMFILTER** environment variable. You can use **envrules** to set **SPAMFILTER** for few specific email addresses. You can refer to the chapter [Envrules](#envrules) for more details.

There is another way you can do spam filtering - during local delivery (you could do for remote delivery, but what would be the point?). IndiMail allows you to call an external program during local/remote delivery by settting **QMAILLOCAL** / **QMAILREMOTE** environment variable. You could use any method to call bogofilter (either directly in *filterargs* control file, or your own script). You can see a Pictorial representation of how this happens. ![LocalFilter](indimail_spamfilter_local.png)

You can also use <b>vcfilter</b>(1) to set up a filter that will place your spam emails in a designated folder for SPAM. Refer to the chapter [Writing Filters for IndiMail](#writing-filters-for-indiMail) for more details.

# SPAM Control using badip control file

IndiMail has many methods to help deal with spam. For detecting spam, IndiMail uses bogofilter a fast bayesian spam filter. IndiMail's <b>qmail-smtpd</b> which provides SMTP protocol is neatly integrated with bogofilter. When bogofilter detects spam, <b>qmail-smtpd</b> prints the X-Bogosity header as part of SMTP transaction log

```
$ grep "X-Bogosity, Yes" /var/log/svc/smtpd.25/current
@400000004bc8183f01fcbc54 qmail-smtpd: pid 16158 from ::ffff:88.191.35.203 HELO X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The value "Yes" in X-Bogosity indicates spam. You can tell <b>qmail-smtpd</b> to reject such mails at SMTP just by doing

```
# echo 1 > /service/qmail-smtpd.25/variables/REJECTSPAM
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
```

SMTP clients which tries to send a spam mail will get the following error at the end of the SMTP transaction
554 SPAM or junk mail threshold exceeded (#5.7.1)
The mail will get bounced. In some cases you would want to issue temporary error to such clients. In the above SMTP transaction log, the IP address of the client was 88.191.35.203. To put such client's into IndiMail's SPAM blacklist, you just need to put the IP address in the control file /etc/indimail/control/badip

`# echo 88.191.35.203 >> /etc/indimail/control/badip`

For turning on the BADIP functionality, you need to set the BADIPCHECK or the BADIP environment variable. i.e.

```
# echo badip > /service/qmail-smtpd.25/variables/BADIP
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
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
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
```

One can also create a vfilter to deliver such email to the quarantine folder

```
/usr/bin/vcfilter -i -t virusFilter -c 0 -k "virus found" -f Quarantine -b 0 -h 28 prefilt@$1
```

If you implement different method, than explained above, let me know.

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

# Greylisting in IndiMail

Greylisting is a method of defending email users against spam, by temporarily rejecting any email from a IP/Sender which it does not recognize. As per SMTP, the originating server should after a delay retry. A server implementing greylisting should accept the mail if sufficient time has elapsed. If the mail is from a spammer it will probably not be retried since a spammer goes through thousands of email addresses and typically cannot afford the time delay to retry.

IndiMail 1.6 onwards implements greylisting using <b>qmail-greyd</b> daemon. You additionally need to have the environment variable GREYIP defined for the <b>qmail-smtpd</b> process. The environment variable GREYIP specifies on which IP and port, <b>qmail-greyd</b> is accepting greylisting requests. <b>qmail-smtpd</b> uses UDP to send a triplet (IP+RETURN\_PATH+RECIPIENT) to the greylisting server and waits for an answer which tells <b>qmail-smtpd</b> to proceed ahead or to temporarily reject the mail. <b>qmail-greyd</b> also accepts a list of whitelisted IP addresses for which greylisting should not be done.

## Enabling qmail-greyd greylisting server

```
$ sudo svctool --greylist=1999 --servicedir=/service --min-resend-min=2 \
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

Alternatively (and particularly if you're not using the -x option to tcpserver) you can enable greylisting for all SMTP connections by setting GREYIP in the environment in which <b>qmail-smtpd</b> is started - for example your variables directory for <b>qmail-smtpd</b> can contain a file with the name GREYIP

```
$ sudo /bin/bash
# echo GREYIP=\"127.0.0.1@1999\" > /service/qmail-smtpd.25/variables/GREYIP
```

NOTE: The above instructions are for IndiMail/indimail-mta 2.x and above. For 1.x releases, use /var/indimail/etc for the location of tcp.smtp and tcp.smtp.cdb

# Configuring dovecot as the IMAP/POP3 server

IndiMail stores it's virtual user information in MySQL. However, IndiMail can work with virtually any IMAP/POP3 server which has a mechanism to authenticate using [checkpassword](https://cr.yp.to/checkpwd/interface.html) interface or PAM which can use the system's passwd database for user's home directory. This is because IndiMail provides checkpassword, PAM module and a NSS service, all independent of each other. The beauty of providing checkpassword, PAM module and NSS mechansism, is that you do not have to modify a single line of code anywhere. In this respect, IndiMail is probably the most flexible messaging server available at the moment.
 * checkpassword module is vchkpass
 * PAM module is [pam-multi - PAM modules for flexible, configurable authentication methods](https://github.com/mbhangui/indimail-virtualdomains/tree/master/pam-multi-x "pam-multi")
 * NSS is through [nssd - providing Name Service Switch](https://github.com/mbhangui/indimail-virtualdomains/tree/master/nssd-xi "nssd")


[Dovecot](https://www.dovecot.org/) is an open source IMAP and POP3 server for Linux/UNIX-like systems, written with security primarily in mind. Dovecot is an excellent choice for both small and large installations. It's fast, simple to set up, requires no special administration and it uses very little memory. Though I do not use dovecot, I have heard excellent reviews from users about dovecot. It took me less than 20 minutes to download dovecot today and have it working with IndiMail with all existing mails intact and accessible. So at the moment, my IndiMail installation is working with both courier-imap and dovecot simultaneously (with different IMAP/POP3 ports assigned to courier-imap and dovecot). dovecot isn't as modular and pluggable as courier-imap and extremely supervise friendly. With dovecot however, you will need to install and configure it outside supervise.

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

DomainKeys Identified Mail (DKIM) lets an organization take responsibility for a message while it is in transit. DKIM has been approved as a Proposed Standard by IETF and published it as RFC 4871. There are number of vendors/software available which provide DKIM signing. IndiMail is one of them. You can see the full list here.
DKIM uses public-key cryptography to allow the sender to electronically sign legitimate emails in a way that can be verified by recipients. Prominent email service providers implementing DKIM (or its slightly different predecessor, DomainKeys) include Yahoo and Gmail. Any mail from these domains should carry a DKIM signature, and if the recipient knows this, they can discard mail that hasn't been signed, or has an invalid signature.

IndiMail from version 1.5 onwards, comes with a drop-in replacement for <b>qmail-queue</b> for DKIM signature signing and verification (see <b>qmail-dkim</b>(8) for more details). You need the following steps to enable DKIM. IndiMail from version 1.5.1 onwards comes with a filter dk-filter, which can be enabled before mail is handed over to <b>qmail-local</b> or <b>qmail-remote</b> (see <b>spawn-filter</b>(8) for more details).

You may want to look at an excellent [setup instructions](http://notes.sagredo.eu/node/92 "Roberto's Notes") by Roberto Puzzanghera for configuring dkim for qmail

## Create your DKIM signature

```
$ sudo /bin/bash
# mkdir -p /etcindimail/control/domainkeys
# cd /etc/indimail/control/domainkeys
# openssl genrsa -out rsa.private 1024
# openssl rsa -in rsa.private -out rsa.public -pubout -outform PEM
# mv rsa.private default
# chown indimail:qmail default (name of our selector)
# chmod 440 default
```

## Create your DNS records

```
$ grep -v ^- rsa.public | perl -e 'while(<>){chop;$l.=$_;}print "t=y; p=$l;\n";'
  _domainkey.indimail.org.  IN TXT  "t=y; o=-;"
  default._domainkey.indimail.org.  IN TXT  "DNS-public-key"
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
# svc -d /service/qmail-smtpd.25; svc -u /service/qmail-smtpd.25
```


## Set SMTP to verify DKIM signatures

You can setup <b>qmail-smtpd</b> for verification by setting DKIMIVERIFY environment variable instead of DKIMSIGN environment variable.

```
$ sudo /bin/bash
# cd /service/qmail-smtpd.25/variables
# echo "/usr/bin/qmail-dkim" > QMAILQUEUE
# echo "/etc/indimail/control/domainkeys/default" > DKIMVERIFY
# svc -d /service/qmail-smtpd.25; svc -u /service/qmail-smtpd.25
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

```
$ sudo /bin/bash
# cd /service/qmail-send.25/variables
# echo "/usr/bin/spawn-filter" > QMAILREMOTE
# echo "/usr/bin/dk-filter" > FILTERARGS
# echo "/etc/indimail/control/domainkeys/default" > DKIMSIGN
# echo "-h" > DKSIGNOPTIONS
# svc -d /service/qmail-send.25; svc -u /service/qmail-send.25
```


## Setting qmail-local to verify DKIM signatures

On your host which serves as your incoming gateway for your local domains, it only makes sense to do DKIM verification with <b>qmail-local</b>

```
$ sudo /bin/bash
# cd /service/qmail-send.25/variables
# echo "/usr/bin/spawn-filter" > QMAILLOCAL
# echo "/usr/bin/dk-filter" > FILTERARGS
# echo "/etc/indimail/control/domainkeys/default" > DKIMVERIFY
# svc -d /service/qmail-send.25; svc -u /service/qmail-send.25
```


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
The image assets get installed in /var/www/html/images/iwebadmin
The html assets get installed in /usr/share/iwebadmin/html
The language files get installed in /usr/share/iwebadmin/lang

The screen shots are below

![iwebadmin1](iwebadmin1.png)
![iwebadmin2](iwebadmin2.png)
![iwebadmin3](iwebadmin3.png)


# Publishing statistics for IndiMail Server

You can now configure MRTG Graphs to show statistics for IndiMail . You need to have mrtg installed on your system. If you do not have mrtg, you can execute yum/dnf

`$ sudo yum install mrtg`

You need to execute the following steps (assuming your web server document root is /var/www/html)

`$ sudo /usr/sbin/svctool --mrtg=/var/www/html/mailmrtg --servicedir=/service`

After carrying out the above step,  check the status of mrtg service

```
$ sudo svstat /service/mrtg
/service/mrtg/: up 35254 seconds pid 2443
```

Point your browser to /var/www/html/mailmrtg and you should see the graphs.

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
# /var/indimail/bin/svstat /service/\*
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

You now have a fully functional mail server with a pre-configured virtual domain indimail.org and a pre-configured virtual user testuser01@indimail.org. You can use IMAP/POP3/SMTP to your heart's content. If not satisfied, try out the ssl enabled services IMAPS/POP3S/SMTPS or STARTTLS command. If still not satisfied, read the man pages in /var/indimail/man/\*. You can stop the container by executing the docker stop command.

`$ docker stop fd09c7ca75be`

You can make your changes to the container and commit changes by using the docker commit command. Learning how to use docker is not difficult. Just follow the Docker Documentation. If you are lazy like me, just read the Getting Started guide.
I am also a newbie as far as docker is concerned. Do let me know your experience with network settings and other advanced docker topics, that you may be familiar with. Do send few bottles of beer my way if you can.

NOTE:
Currently IndiMail supports both docker and podman. Both commands are interchangeble. There is much that has happened since this chapter was written in Apr 2016. You should be actually reading [this](https://github.com/mbhangui/docker/blob/master/README.md "Docker README ").

# Installation & Repositories

## Installing Indimail using DNF/YUM/APT Repository

You can get binary RPM / Debian packages at

* [Stable](http://download.opensuse.org/repositories/home:/indimail/ "Stable Build Repository")
* [Experimental](http://download.opensuse.org/repositories/home:/mbhangui/ "Experimental Build Repository")

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable](https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail "Stable Build Repository")
* [Experimental](https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail "Experimental Build Repository")

```
Currently, the list of supported distributions for IndiMail is

    * Arch Linux

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
          o SUSE Linux Enterprise 15 SP2
          o SUSE Linux Enterprise 15 SP3

    * Red Hat
          o Fedora 33
          o Fedora 32
          o Red Hat Enterprise Linux 7
          o Scientific Linux 7
          o CentOS 7
          o CentOS 8

    * Debian
          o Debian  9.0
          o Debian 10.0
          o Univention_4.3
          o Univention_4.4

    * Ubuntu
          o Ubuntu 16.04
          o Ubuntu 17.04
          o Ubuntu 18.04
          o Ubuntu 19.04
          o Ubuntu 19.10
          o Ubuntu 20.04
          o Ubuntu 20.10
```

## Docker / Podman Repository

The [docker repository](https://hub.docker.com/r/cprogrammer/indimail "Docker Repository") can be used to pull docker/podman images for indimail.

For latest details refer to [README](https://github.com/mbhangui/docker/blob/master/README.md "Docker README ")

## GIT Repository

IndiMail has a git repository at [here](https://github.com/mbhangui/indimail-virtualdomains "IndiMail")

# Support Information

## IRC / Matrix

* Join me https://matrix.to/#/#indimail:matrix.org
* IndiMail has an IRC channel #indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support "Support"). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel "Devel Support"). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here]
(https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail "Announce")
4. Archive at [Google Groups](http://groups.google.com/group/indimail "Archive"). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686 "Project Trackerr") for IndiMail (Bugs, Feature Requests, Patches, Support Requests)

# Features

## Speed

IndiMail uses a modified [qmail](http://cr.yp.to/qmail.html "qmail") as the MTA. qmail's modular, lightweight design and sensible queue management provides IndiMail the speed to make it one of the fastest available message transfer agent.
IndiMail provides ''qmail-multi'', a drop-in replacement to ''qmail-queue''. ''qmail-multi'' uses multiple queues with each queue running its own <b>qmail-send</b>/qmail-todo, <b>qmail-lspawn</b>, <b>qmail-rspawn</b> processes. This allows IndiMail to process mails faster than what can be provided by qmail. A process named qmail-daemon monitors these processes and restarts them if they go down, which hasn't yet been observed to go down.

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
 * authenticated SMTP PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD5 (HMAC (RFC 1321, RFC 2104, RFC 2554, RFC 2617))
 * STARTLS extension, TLS
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
 * [SPF](http://www.openspf.org/ "openspf") – Sender Permitted From
 * [SRS](http://www.libsrs2.org/ "libsrs2") - Sender Rewriting Scheme
 * [Bounce Address Tag Validation (BATV)](http://www.linux-magazine.com/Online/News/IndiMail-1.6-Includes-Greylisting-and-BATV?category=13404 "batv")
 * Per User control of environment variable by envrules(rules file set by environment variable FROMRULES)
 * [Greylisting](http://www.gossamer-threads.com/lists/qmail/users/136761 "greylisting") capability using <b>qmail-greyd</b> or greydaemon
 * SMTP Plugins - External plugins using shared objects in /var/indimail/plugins to enhance functionality of MAIL, RCPT & DATA session.
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
 * Configurable number of queues and time slicing algorithm for load balancing via <b>qmail-multi</b>. A queue in indimail is configurable by three environment variables QUEUE\_BASE, QUEUE\_COUNT, and QUEUE\_START. A queue in IndiMail is a collection of queues. Each queue in the collection can have one or more SMTP listener but a single or no delivery (<b>qmail-send</b>) process. It is possible to have the entire queue collection without a delivery process (e.g. SMTP on port 366 – ODMR). The QUEUE\_COUNT can be defined based on how powerful your host is (IO bandwidth, etc). NOTE: This configurable number of queues is possibe with a single installation and does not require you to install multiple instances of qmail.
 * Ability to hold local, remote or both deliveries (holdlocal, holdremote control file)
 * Qmail Queue Extra Header – Ability to pass extra headers to local and remote deliveries via <b>qmail-queue</b> (Environment variable QQEH).
 * External Virus scanning via QHPSI – Qmail High Performance Scanner Interface
 * Ability to extend QHPSI interface through plugins. The keyword plugin:shared\_lib defined in the environment variable QHPSI denotes 'shared\_lib' to be loaded.
 * Virus scanner qscanq. Ability to detect virus via a third party scanner defined by SCANCMD environment variable (clamscan, clamdscan, etc)
 * Blocking of prohibited filename extensions via qscanq program
 * [Domainkeys](https://en.wikipedia.org/wiki/DomainKeys "Yahoo Domainkeys") (<b>qmail-dk</b>) RFC 4870
 * [DomainKeys Identified Mail](http://marc.info/?l=qmail&m=123817543532399&w=2 "DKIM") with ADSP/SSP (<b>qmail-dkim</b>) RFC 4871
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
 * Authenticated SMTP (userid/passwd in control/smtproutes) - PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD
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
    * <b>qmail-multi</b> (queue load balancer) uses <b>qmail-queue</b> to deposit mails across multiple queues. Each queue has its own <b>qmail-send</b> process. You can spread the individual queues across multiple filesystems on different controllers  to maximize on IO throughput.
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
15. <b>qmail-multi</b> - run multiple filters (qmail-smtpd) (something like <b>qmail-qfilter</b>). Also distributes mails across multiple queues to do a load balancing act. <b>qmail-multi</b> allowed me to process massive rate of incoming mails at my earlier job with a ISP.
16. envheaders - Any thing defined here e.g. Return-Path, qmail-queue sets Return-Path as an environment variable with the value found in Return-Path header in the email. This environment variable gets passed across the queue and is also available to <b>qmail-local</b>, <b>qmail-remote</b>
17. logheaders - Any header defined in this control file, gets written to file descriptor 2 with the value found in the email.
18. removeheaders - Any header defined here, qmail-queue will remove that header from the email
19. quarantine or QUARANTINE env variable causes qmail-queue to replace the recipient list with the value defined in the control file or the environment variable. Additionally an environment variable X-Quarantine-ID: is set which holds the orignal recipient list.
20. Added ability in qmail-queue to do line processing. Line processing allows qmail-queue to do some of the stuff mentioned above
21. plugins support for QHPSI interface (qmail-queue). qmail-queue will use dlopen to load any shared objected defined by PLUGINDIR environment. Multiple plugins can be loaded. For details see man qmail-queue
22. Message Submission Port (port 587) RFC 2476
23. Integrated authenticated SMTP with Indimail (PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD5, pop-bef-smtp)
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
41. Greylisting Capability -  [Look Here](http://www.gossamer-threads.com/lists/qmail/users/136740?page=last "Greylisting")
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

# History

Both indimail-mta and indimail-virtualdomains started in late 1999 as a combined package of unmodified qmail and modified vpopmail.

indimail-mta started as a unmodified qmail-1.03. This was when I was employed by an ISP in late 1999. The ISP was using [Critical Path's ISOCOR](https://www.wsj.com/articles/SB940514435217077581) for providing Free and Paid email service. Then the dot com burst happened and ISP didn't have money to spend on upgrading the E3500, E450 [Sun Enterprise servers](https://en.wikipedia.org/wiki/Sun_Enterprise). The mandate was to move to an OSS/FS solution. After evaluating sendmail, postfix and qmail, we chose qmail. During production deployment, qmail couldn't scale on these servers. The issue was the queue's todo count kept on increasing. We applied the <b>qmail-todo</b> patch, but still we couldn't handle the incoming email rate. By now the customers were screaming, the corporate users were shooting out nasty emails. We tried a small hack the solved this problem. Compiled 20 different qmail setups, with conf-qmail as /var/qmail1, /var/qmail2, etc. Run <b>qmail-send</b> for each of these instance. A small shim was written which would get the current time and divide by 20. The remainder was used to do exec of /var/qmail1/bin/qmail-queue, /var/qmail2/bin/qmail-queue, etc. The shim was copied as /var/qmail/bin/qmail-queue. The IO problem got solved. But the problem with this solution was compiling the qmail source 20 times and copying the shim as <b>qmail-queue</b>. You couldn't compile qmail on one machine and use the backup of binaries on another machine. Things like uid, gid, the paths were all hardcoded in the source. That is how the base of indimail-mta took form by removing each and every hard coded uids, gids and paths. indimail-mta still does the same thing that was done in the year 2000. The installation creates multiple queues - /var/indimail/queue/queue1, /var/indimail/queue/queue2, etc. A new daemon named qmail-daemon uses QUEUE\_COUNT env variable to run multiple <b>qmail-send</b> instances. Each <b>qmail-send</b> instance can instruct <b>qmail-queue</b> to deposit mail in any of the queues installed. All programs use <b>qmail-multi</b>, a <b>qmail-queue</b> frontend to load balance the incoming email across multiple queues.

indimail-virtualdomain started with a modified vpopmail base that could handle a distributed setup - Same domain on multiple servers. Having this kind of setup made the control file smtproutes unviable. email would arrive at a relay server for user@domain. But the domain '@domain' was preset on multiple hosts, with each host having it's own set of users. This required special routing and modification of qmail (especially <b>qmail-remote</b>) to route the traffic to the correct host. <b>vdelivermail</b> to had to be written to deliver email for a local domain to a remote host, in case the user wasn't present on the current host. New MySQL tables were created to store the host information for a user. This table would be used by <b>qmail-local</b>, <b>qmail-remote</b>, <b>vdelivermail</b> to route the mail to the write host. All this complicated stuff had to be done because the ISP where I worked, had no money to buy/upgrade costly servers to cater to users, who were multiplying at an exponential rate. The govt had just opened the license for providing internet services to private players. These were Indians who were tasting internet and free email for the first time. So the solution we decided was to buy multiple intel servers [Compaq Proliant](https://en.wikipedia.org/wiki/ProLiant) running Linux and make the qmail/vpopmail solution horizontally scalable. This was the origin of indimail-1.x which borrowed code from vpopmail, modified it for a domain on multiple hosts. indimail-2.x was a complete rewrite using djb style, using [libqmail](https://github.com/mbhangui/libqmail) as the standard library for all basic operations. All functions were discarded because they used the standard C library. The problem with indimail-2.x was linking with MySQL libraries, which caused significant issues building binary packages on [openSUSE build service](https://build.opensuse.org/). Binaries got built with MySQL/MariaDB libraries pulled by OBS, but when installed on a target machine, the user would have a completely different MySQL/MariaDB setup. Hence a decision was taken to load the library at runtime using dlopen/dlsym. This was the start of indimail-3.x. The source was moved from sourceforge.net to github and the project renamed as indimail-virtualdomains from the original name IndiMail. The modified source code of qmail was moved to github as indimail-mta.

# See also

* [Official qmail website, maintained by Daniel J. Bernstein](http://cr.yp.to/qmail.html "qmail")
* [Unofficial qmail website, maintained by Russ Nelson](http://www.qmail.org "qmail.org")
* [Life with qmail](http://www.lifewithqmail.org/lwq.html "LWQ")
