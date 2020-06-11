#INTRODUCTION

[IndiMail](https://github.com/mbhangui/indimail-virtualdomains "IndiMail") is a messaging Platform comprising of multiple software packages including

* [qmail](http://cr.yp.to/qmail.html "qmail")
* [serialmail](https://cr.yp.to/serialmail.html "serialmail")
* [qmailanalog](http://cr.yp.to/qmailanalog.html "qmailanalog")
* [dotforward](https://cr.yp.to/dot-forward.html "dot-foward")
* [fastforward](https://cr.yp.to/fastforward.html "fastforward")
* [mess822](https://cr.yp.to/mess822.html "mess822")
* [daemontools](https://cr.yp.to/daemontools.html "daemontools")
* [ezmlm-idx mailing list manager](https://untroubled.org/ezmlm/ "ezmlm-idx")
* [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp")
* [logalert](https://github.com/mbhangui/indimail-virtualdomains/tree/master/logalert-0.3 "logalert")
* [courier IMAP/POP3](https://www.courier-mta.org/imap/ "courier-imap")
* [fetchmail](https://www.fetchmail.info "fetchmail")
* [bogofilter - A Bayesian Spam Filter](https://bogofilter.sourceforge.io/ "bogofilter")
* [Clam AntiVirus - GPL anti-virus toolkit for UNIX](https://www.clamav.net/ "clamd")
* Utilities (mpack, unpack, altermime, ripmime, fortune, flash).

**IndiMail** consists of two main packages - [indimail-mta](https://github.com/mbhangui/indimail-mta "indimail-mta") and [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains "indimail-virtualdomains").

* 	**indimail-mta** is a re-engineered version of [qmail](http://cr.yp.to/qmail.html "qmail").  **indimail-mta** provides you a MTA with all the features and functionality of **[qmail](http://cr.yp.to/qmail.html "qmail")** plus many additional features.
* 	**indimail-virtualdomains** provides you tools to create and manage multiple virtual domains with its own set of users, who can send and receive mails.

This document will refer to **IndiMail** as a combined package of indimail-virtualdomains, indimail-mta & other packages namely indimail-access, indimail-auth, indimail-utils, indimail-spamfilter.

When you install indimail-virtualdomains, a shared library from the package is dynamically loaded by indimail-mta to provide virtual domain support in indimail-mta, along with the ability to work with IMAP/POP3 retrieval daemons.

indimail-virtualdomains provides programs to manage multiple virtual domains on a single host. It allows extending a domain across multiple servers. With indimail-mta installed, it can act as an SMTP router/interceptor. This is a very powerful feature which allows IndiMail to provide native horizontal scalability. It knows the location of each and every user and can distribute mails for any user residing on any server located anywhere on the network. If one uses IndiMail, one can simply add one more server and cater to more users without changing any configuration, software or hardware of existing servers. The architecture allows IndiMail to do away with costly NAS storage (i.e you don't require a common shared storage across servers). This allows you to scale indimail easily to serve millions of users using commodity hardware. You can read the feature list to get an idea of the changes and new features that have been added over the original packages. You can see a pictorial representation of the architecture ![Image](indimail_arch.png "IndiMail Architecture")

IndiMail allows servers to be distributed anywhere geographically. This is useful especially if you have users at different parts of the globe. e.g. Your Brazil users can have their server located in Brazil, Bombay users in Bombay, Delhi users in Delhi. And yet when your Brazil users comes to Delhi for a visit, he or she can access all emails sitting in Delhi by accessing the Delhi server. IndiMail provides this distributed feature using proxies for SMTP, IMAP and POP3 protocols. The proxy servers run using [ucspi-tcp](https://cr.yp.to/ucspi-tcp.html "ucspi-tcp") and are by default configured under supervise. You can use any IMAP/POP3 server behind the proxy. You can extend the domain across multiple servers without using any kind of NAS storage.

The ability of IndiMail to know user's location also allows IndiMail to setup a heterogeneous messaging environment. If you have IndiMail, you can have a server running MS exchange, few servers running IBM Lotus Notes, and few servers running IndiMail and all using a single domain. A utility called 'hostcntrl' allows you to add foreign users to IndiMail's database.  This feature also allows you to migrate your users from a proprietary platform to IndiMail without causing downtime or disruption to existing users. In fact, this method has been used very successfully in migrating corporate users out of MS Exchange & IBM Lotus Notes to IndiMail without the end users realizing it.

To migrate from an existing proprietary mail server like MS Exchange requires 5 steps.

1. You simply set up a new installation with IndiMail and create the existing domain using **vadddomain**.
2. Add the IP address of the Exchange Server in ***host_table*** and the SMTP port of the Exchange Server in the table ***smtp_port***.
3. Add users on the exchange server to a table called ***hostcntrl*** (either manually or using the utility **hostcntrl**).
4. Modify your user's mail configuration to use SMTP, IMAP Proxy, POP3 Proxy ports on the IndiMail server (**proxypop3**, **proxyimap**)
5. Change the MX to point to the indimail-mta server.

IndiMail is highly configurable. No hard-coded directories of qmail like /var/qmail/control or /var/qmail/queue. All directories are configurable at run time. IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration.  To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance.  Multiple queues eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture gives allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). Here is a pictorial representation of the IndiMail queue ![Pictorial](indimail_queue.png).

IndiMail is a pure messaging solution. It does not provide calendars, todo lists, address books, meeting requests and a web mail front-end. However, you can use [Roundcube Mail](https://roundcube.net/) or any web mail front-end that works with IMAP or POP3 protocol with IndiMail. If you decide to install [Roundcube Mail](https://roundcube.net/), you can install the **ircube** package from the IndiMail's DNF/YUM/Debian [Repository](https://build.opensuse.org/package/show/home:indimail/ircube "ircube") to have a fully functional web mail front-end. The ircube package provides plugins for Rouncube Mail to manage your passwords, vacation and SPAM filters.

IndiMail administrators can use a web administration tool called **iwebadmin**. It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/v3.2/iwebadmin-2.0) or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/iwebadmin "iwebadmin") on the openSUSE build Service.

IndiMail comes with a power tcl/tk administration client called [**indium**](https://github.com/mbhangui/indimail-virtualdomains/tree/v3.2/indium-1.1 "indium"). It can be installed from source from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/v3.2/indium-1.1) or from the [DNF/YUM/Debian Repository](https://build.opensuse.org/package/show/home:indimail/indium) on the openSUSE Build Service.


To install IndiMail you can take the help of the following documents in the [indimail-x/doc subdirectory](https://github.com/mbhangui/indimail-virtualdomains/tree/v3.2/indimail-x/doc) of [indimail-virtualdomains](https://github.com/mbhangui/indimail-virtualdomains) github repository.

File | Description
----- | --------------------
README-indimail.md|Introduction to IndiMail (this file)
INSTALL.md|Source Installation Instructions
INSTALL-RPM.md|Install Instructions using RPM
INSTALL-MYSQL.md|MySQL specific Installation Instructions
README-CLUSTER.md|Steps on configuring a clustered setup
Quick-INSTALL.md|A minimal documentation on Installation/Configuration)

If you have enough experience on your belt you can dive into this [document](https://github.com/mbhangui/indimail-virtualdomains/blob/v3.2/README.md).

Once you have installed IndiMail, you will find all man pages in /usr/share/man and documents in /usr/share/indimail/doc. You can do man indimail to get started with understanding IndiMail.

#DISCLAIMER

There is no warranty implied or otherwise with this package. I believe in OpenSource Philosophy and this is an attempt to give back to the OpenSource community. However, I welcome anyone who can provide some assistance for building few missing features (building new features, testing and documentation).

# LICENSING

IndiMail uses GPLv3 License. See file [LICENSE](https://github.com/mbhangui/indimail-virtualdomains/blob/v3.2/LICENSE). Additional licenses (if any) may be found in subfolder of each component that IndiMail uses.

# BRIEF FEATURE LIST

Some of the features available in this package

1.  svctool - A simple tool with command-line options which helps you to configure any configuration item in indimail (creation of supervise scripts, qmail configuration, installation of all default MySQL tables, creation of default aliases, users, etc)
2.  configurable control files directory (using CONTROLDIR environment variable) (allows one to have multiple running copies of qmail using a single binary installation)
3.  configurable queue directory (using QUEUEDIR environment variable) (allows one to have multiple queues on a host with a single qmail installation). 
	- 	qmail-multi (queue load balancer) uses qmail-queue to deposit mails across multiple queues. Each queue has its own qmail-send process. You can spread the individual queues across multiple filesystems on different controllers  to maximize on IO throughput.
	- 	The number of queues is configurable by three environment variables **QUEUE_BASE**, **QUEUE_COUNT** and **QUEUE_START**. A queue in indimail is defined as a collection of multiple queues.
	- 	Each queue in the collection can have one or more SMTP listener but a single delivery (qmail-send) processes. It is possible to have the entire queue collection without a delivery process (e.g. SMTP on port 366 ODMR). The QUEUE_COUNT can be defined based on how powerful your host is (IO bandwidth, etc). The configurable queue is possible with a single installation of indimail-mta and does not require you to have multiple indimail-mta installations (unlike qmail) to achieve this.
4.  uses getpwnam to use uids/gids, home from /etc/passwd, /etc/group (allows me to transfer the binary to another machine regardless of the ids in /etc/passwd)
5.  Hooks into local and remote deliveries
	- QMAILLOCAL - Run executable defined by this instead of qmail-local
	- QMAILREMOTE - Run executable defined by this instead of qmail-remote
	- Theoretically one can exploit QMAILLOCAL, QMAILREMOTE variables to route mails for a domain across multiple mail stores.
6.  Hook into qmail-remote's routing by using SMTPROUTE environment variable.
	- Ability to do User Based Routing via SMTPROUTE environment. This gives the ability to split a domain across multiple hosts without using NFS to mount multiple filesystems on any host. One can even use a shell script, set the environment variable and deliver mails to users across multiple hosts. I call this dynamic SMTPROUTE.
	- Additionally qmail-rspawn has the ability to connect to MySQL and set SMTPROUTES based on values in a MySQL table. The connection to MySQL is kept open. This gives qmail-rspawn to do high speed user lookups in MySQL and to deliver the mail for a single domain split across multiple mail stores.
7.  Proxy for IMAP and POP3. Allows IMAP/POP3 protocol for users in a domain to be split across multiple hosts. Also allows seamless integration of proprietary email servers with indimail.  The proxy is generic and works with any IMAP/POP3 server. The proxy comes useful when you want to move out of a headache causing mail server like x-change and want to retain the same domain on the proprietary server. In conjunction with dynamic SMTPROUTES, you can migrate all your users to indimail, without any downtime/disruption to email service. The proxy and dynamic SMTPROUTES allow you to scale your email server horizontally without using NFS across geographical locations.
8.  ETRN, ATRN, ODMR (RFC 2645) support
9.  accesslist - restrictions between mail transactions between email ids (you can decide who can send mails to whom)
10. bodycheck - checks on header/body on incoming emails (for spam, virus security and other needs)
11. hostaccess - provides domain, IP address pair access list control. e.g. you can define from which set of addresses mail from yahoo.com will be accepted.
12. chkrcptdomains - rcpt check on selective domains
13. envrules - recipient/sender based - set or unset environment variables (qmail-smtpd, qmail-inject, qmail-local, qmail-remote) any variables which controls the behaviour of qmail-smtpd, qmail-inject, qmail-local, qmail-remote e.g. NODNSCHECKS, DATABYTES, RELAYCLIENT, BADMAILFROM, etc can be defined individually for a particular recipient or sender rather than a fixed value set at runtime.
14. NULLQUEUE, qmail-nullqueue (blackhole support - like qmail-queue but mails go into a blackhole). I typically uses this in conjunction with envrules to trash the mail into blackhole without spending any disk IO.
15. qmail-multi - run multiple filters (qmail-smtpd) (something like qmail-qfilter). Also distributes mails across multiple queues to do a load balancing act. qmail-multi allowed me to process massive rate of incoming mails at my earlier job with a ISP.
16. envheaders - Any thing defined here e.g. Return-Path, qmail-queue sets Return-Path as an environment variable with the value found in Return-Path header in the email. This environment variable gets passed across the queue and is also available to qmail-local, qmail-remote
17. logheaders - Any header defined in this control file, gets written to file descriptor 2 with the value found in the email.
18. removeheaders - Any header defined here, qmail-queue will remove that header from the email
19. quarantine or QUARANTINE env variable causes qmail-queue to replace the recipient list with the value defined in the control file or the environment variable. Additionally an environment variable X-Quarantine-ID: is set which holds the orignal recipient list.
20. Added ability in qmail-queue to do line processing. Line processing allows qmail-queue to do some of the stuff mentioned above
21. plugins support for QHPSI interface (qmail-queue). qmail-queue will use dlopen to load any shared objected defined by PLUGINDIR environment. Multiple plugins can be loaded. For details see man qmail-queue
22. Message Submission Port (port 587) RFC 2476
23. Integrated authenticated SMTP with Indimail (PLAIN, LOGIN, CRAM-MD5, CRAM-SHA1, CRAM-RIPEMD, DIGEST-MD5, pop-bef-smtp)
24. duplicate eliminator using 822header
25. qmail-remote has configurable TCP timeout table (max_tolerance, min_backoff periods can be configured in smtproutes)
26. Ability to change concurrency of tcpserver without restarting tcpserver
27. Ability to restrict connections per IP
28. multilog replaced buffer functions with substdio
29. supervise can run script shutdown if present (when svc -d is issued)
30. rfc3834 compliance for qmail-autoresponder (provide Auto-Submitted, In-Reply-To, References fields (RFC 3834))
31. ability to add stupid disclaimer(s) to messages.
32. InLookup serves as a high performance user lookup daemon for qmail-smtpd (rcpt checks, authenticated SMTP, RELAY check). Even the IMAP, POP3 authentication gets served by inlookup. inlookup preforks configurable number of daemons which opens multiple connections to MySQL and keep the connection open. The query results are cached too. This gives inlookup a decent database performance when handling millions of lookups in few hours. Programs like qmail-smtpd use a fifo to communicate with inlookup
33. CHKRECIPIENT extension which rejects users not found in local MySQL or recipients.cdb database
34. indisrvr Was written to ease mail server administration across multiple hosts. Allows ones to create, delete, modify users and run any command as defined in variables.c. indisrvr listens on a AF_INET/AF_INET6 socket.
35. Identation of djb's code (using indent) so that a mortal like me could understand it :)
36. Works with systemd - systemd is an event-based replacement for the init daemon
37. Changed buffer libraries in daemontools to substdio.
38. Can work with external virus scanners (QHPSI, or Len Budney's qscanq)
39. qmail-queue custom error patch by Flavio Curti <fcu-software at no-way.org>
40. Domainkey-Signature, DKIM-Signature with ADSP/SSP
41. Greylisting Capability -  [Look Here](http://www.gossamer-threads.com/lists/qmail/users/136740?page=last)
42. nssd - Name Service Switch daemon which extends systems password database to lookup IndiMail's database for authentication
43. pam-multi - Generic PAM which allows external program using PAM to authenticate against IndiMail's database. Using pam-multi and nssd, you can use any IMAP server like dovecot, etc with IndiMail.
44. Post execution Handle - Allows extending indimail's functionality by writing simple scripts
45. sslerator - TLS/SSL protocol wrapper for non-tls aware applications.
46. QMTP support in qmail-remote. QMTP support for mail transfers between IndiMail clusters.
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
63. Enforce STARTTLS before AUTH using FORCE_TLS environment variable
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
    * [indimail](https://hub.docker.com/repository/docker/cprogrammer/indimail)
	* [indimail-mta](https://hub.docker.com/repository/docker/cprogrammer/indimail-mta)
	* [indimail-web](https://hub.docker.com/repository/docker/cprogrammer/indimail-web)
75. tcpserver plugin feature - dynamically load shared objects given on command line. Load shared objects defined by env variables PLUGIN0, PLUGIN1, ...
    tcpserver plugin allows you to load qmail-smtpd, rblsmtpd once in memory
76. FHS compliance
77. Ed Neville - allow multiple Delivered-To in qmail-local using control file maxdeliveredto
78. Ed Neville - configure TLS method in control/tlsclientmethod (qmail-smtpd), control/tlsservermethod (qmail-remote)
79. roundcube support for password, autoresponder through roundcube plugin iwebadmin
80. ezmlm-idx mailing list manager from https://untroubled.org/ezmlm/
81. tcpserver - enable mysql support by loading mysql library configured in /etc/indimail/control/mysql_lib
82. indimail-mta - enable virtual domain support by loading indimail shared library configured in /etc/indimail/control/libindimail

The complete list of features can be seen [HERE](https://sourceforge.net/p/indimail/wiki/IndiMail/)

# Support Information

## IRC
IndiMail has an IRC channel ##indimail and ##indimail-mta

## Mailing list

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here]
(https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)

# Binary Builds on openSUSE Build Service

You can get binary RPM / Debian packages at

* [Stable](http://download.opensuse.org/repositories/home:/indimail/)
* [Experimental](http://download.opensuse.org/repositories/home:/mbhangui/)

If you want to use DNF / YUM / apt-get, the corresponding install instructions for the two repositories, depending on whether you want to install a stable or an experimental release, are

* [Stable](https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail)
* [Experimental](https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail)

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

# GIT Repository
IndiMail has a git repository at [here](https://github.com/mbhangui/indimail-virtualdomains)

-- Manvendra Bhangui <manvendra@indimail.org>
