<!-- # vim: wrap
-->
# IndiMail Frequently Answered Questions

## Intro

IndiMail is a secure, reliable, efficient, simple mail server with all major components coded entirely in C. It has very little hardcode paths and settings. Most settings are configurable through control files or environment variables. It has a small footprint. It provides the functionality of delivering mails to the User's mailbox and retrieving the same by any third party MUA used by the Internet Community. It also provided you the ability to insert your own scripts before, during or after a message submission or delivery.

However IndiMail does not provide any Email Client or web mail Client. It is designed for typical Internet-connected UNIX hosts. It is expected that one use any IMAP/POP3 client that you are comfortable with.

## How can I set one virtual domain to be the primary domain for the machine? We don't want to have any /etc/passwd users.

Create the default domain with

    $ sudo vadddomain vdomain

Setup your imap/pop3 services with DEFAULT\_DOMAIN=<u>vdomain</u>

    $ sudo sh -c "echo vdomain > /service/qmail-imapd-ssl.993/variables/DEFAULT_DOMAIN"
    $ sudo sh -c "echo vdomain > /service/qmail-imapd.143/variables/DEFAULT_DOMAIN"
    $ sudo sh -c "echo vdomain > /service/qmail-pop3d-ssl.995/variables/DEFAULT_DOMAIN"
    $ sudo sh -c "echo vdomain > /service/qmail-pop3d.110/variables/DEFAULT_DOMAIN"

Then restart the services for the variable to take effect

    $ sudo svc -r /service/qmail-imapd-ssl.993 /service/qmail-imapd.143 \
        /service/qmail-pop3d-995 /service/qmail-pop3d.110

If you have your own scripts to start these services, then you need to set the DEFAULT\_DOMIAN environment variable in your script.

This will allow users of the primary domain to set their pop user name to <b>user</b> instead of <b>user%vdomain</b> / <b>user@vdomain</b>.

See the documentation for [vadddomain](https://github.com/mbhangui/indimail-mta/wiki/vadddomain.1) for all options.

## How do I  pick up my virtual domain email? Virtual domain users need to use the following format for their user name when popping in:

    user%vdomain

Eudora might require the following syntax

    user%vdomain@pophost

Microsoft clients may the following syntax:

    user@vdomain

If you don't include the %vdomain or @vdomain, IndiMail will assume it is either a /etc/passwd user or a user in /etc/indimail/users (See [qmail-users(5)](https://github.com/mbhangui/indimail-mta/wiki/qmail-users.5)) or configured with "default domain".

## How do I forward all mail that doesn't match any users or .qmail files for a particular domain?

Edit the `~indimail/domains/virtual_domain/.qmail-default` file and change the last parameter to an email address of the form: user@domain. For example

    |/sbin/vfilter '' user@domain

or

    |/sbin/vdelivermail '' user@domain

See the documentation for [vfilter](https://github.com/mbhangui/indimail-mta/wiki/vfilter.8) and [vdelivermail](https://github.com/mbhangui/indimail-mta/wiki/vdelivermail.8)

## How do I bounce all mail that doesn't match any users or .qmail files for a particular domain?

Edit the `~indimail/domains/vdomain/.qmail-default` file and change the last parameter to <b>bounce-no-mailbox</b>.

    | /sbin/vfilter '' bounce-no-mailbox

or

    | /sbin/vdelivermail '' bounce-no-mailbox

## I don't want to bounce emails for non existent users. Instead I want to delete them, how?

The last parameter in the .qmail-default file tells vdelivermail what to do with non-matching emails. The default is <b>bounce-no-mailbox</b>, which bounces the email back to the sender. But you can also delete it instead.

    | /sbin/vfilter '' delete

or

    | /sbin/vdelivermail '' delete

## How do hard quota's for users work? How do I set a default quota for new user creation

Read this [document](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#maildir-quotas)

## I want to use a different file name than /etc/indimail/tcp/tcp.smtp for my static IPs for permanent relay.

For compile time configuration, pass --enable-tcpsever-file to ./configure

    $ ./configure –enable-tcpserver-file=smtp-relay-file

For binary installations, edit your tcpserver run file and change the -x option.

See the documentation for [tcpserver](https://github.com/mbhangui/indimail-mta/wiki/tcpserver.1) for all options.

## How can we use an IP address per domain, so that users don't need to authenticate with user%domain or user@domain,they just want to use "user"?

    $ ./configure --enable-ip-alias-domains=y

Then run the below command for each IP you want to link to a domain.

    $ vipmap -a IP vdomain

Consult the documentation for [vipmap](https://github.com/mbhangui/indimail-mta/wiki/vipmap.1) for all options

## How do I alias a new virtual domain to a current virtualdomain?

    $ sudo vaddaliasdomain newdomain olddomain

See [vaddaliasdomain](https://github.com/mbhangui/indimail-mta/wiki/vaddaliasdomain.1)

## How do I post a email to all users of a virtualdomain or a set of domains?

use the new vbulletin program:

    $ vbulletin -f email_file vdomain1 vdomain2 ...

To see all the available options see this documentation on [vbulletin](https://github.com/mbhangui/indimail-mta/wiki/vbulletin.1).

## How do I setup IndiMail to talk to MySQL

Learn how to use dbinfo or the control file mcdinfo. A non-distributed setup uses the control file host.mysql
Read this [document](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#mysql-control-file)

## I want to upgrade IndiMail, what do I need to worry about?

Following things happen in any upgrade that involves software

Binaries, library change. Sometimes database table structure change, sometimes files get new permissions. The right way to for any software to do is to handle changes seamlessly. In few cases this may not be possible where existing software configuration needs to be modified for the new software to run. Usually this gets taken care by the upgrade post install scripts. This is how indimail does it either through the upgrade scripts <b>qlocal\_upgrade</b> and <b>ilocal\_upgrade</b>. IndiMail also has a utility [ischema](https://github.com/mbhangui/indimail-mta/wiki/ischema.1), which allows custom action on every upgrade. <b>ischema</b> has the ability to update MySQL tables, run commands, edit configuration files.

However, none of the users or domain get touched. So to upgrade IndiMail, you just need to run yum/dnf update, apt-get update && apt-get upgrade, zypper upgrade, etc (depending on your OS). For Source installation you need to

1. download new IndiMail software / git pull
2. run configure with your options
3. make
4. make install-strip
5. check if ilocal\_upgrade.in, qlocal\_upgrade.in have been updated. Check if indimail.schema file has been updated. If yes, run the scripts manually.
6. upgrade completed.

If you want to backup the programs, libraries and include files then run the svctool program with the --backup option. This will backup your configuration, service configuration and also create a MySQL export file.

    $ sudo svctool --backup=/home/mail/backup --mysqlPrefix=/usr \
        --servicedir=/service

[svctool](https://github.com/mbhangui/indimail-mta/wiki/svctool.8) is the main configuration tool for IndiMail.

## How do I log when people authenticate with pop/imap?

Make sure you include this configuration option when building IndiMail

    ./configure –enable-auth-logging=y

You can turn of auth logging by setting <b>NOLASTAUTHLOGGING</b>. e.g.

    $ sudo sh -c "echo 1 > /service/qmail-imapd-ssl.993/variables/NOLASTAUTHLOGGING"
    $ sudo sh -c "echo 1 > /service/qmail-imapd.143/variables/NOLASTAUTHLOGGING"
    $ sudo sh -c "echo 1 > /service/qmail-pop3d-ssl.995/variables/NOLASTAUTHLOGGING"
    $ sudo sh -c "echo 1 > /service/qmail-pop3d.110/variables/NOLASTAUTHLOGGING"

Restart the above services using the `svc -r` command. If you have your own scripts to start these services, then you need to set the NOLASTAUTHLOGGING environment variable in your script.

## How can I uninstall IndiMail?

Two Options (Use the one you find easier). The options below may also remove all associated software (iwebadmin, indimail-auth, indimail-access, indimail-mta, indimail-utils, procmail, logalert).

Run the following commands for binary installations. The commands below do not remove existing data and configuration.

    rpm -e indimail # for rpm based systems
or
    dpkg --purge indimail # for debian based systems

For source installations

On Gnu/Linux type systems as root: This will completely wipe your data and some configuration.

    $ sudo userdel -r indimail
    $ sudo groupdel indimail

To completly clean your indimail installation of virtual domains

    $ sudo rm -rf /etc/indimail /usr/libexec/indimail /usr/share/indimail
    $ sudo rm -rf /var/indimail/domains/*
    $ sudo cp /etc/indimail/control/locals /etc/indimail/control/rcpthosts
    $ sudo rm -rf /etc/indimail/users/*

If enable roaming users and your tcp.smtp file was not in the /etc/indimail/tcp directory, you will need to remove the open-smtp* files, where ever they are. Remove shared library libindimail.so* from /usr/lib or /usr/lib64 and remove /etc/indimail/control/libindimail /etc/indimail/control/libmysql

## I get errors about not finding a .so library, how to fix?

Your operating system does not know where those shared object libraries are. On linux read up on how to use ld.so.conf. You can run the svctool with the --fixsharedlibs option. IndiMail doesn't link the libindimail and libmysql library during compile time. They are dynamically opened using dlopen(3). The names of the files are maintained in /etc/indimail/control/libmysql for the MySQL / MariaDB shared library. indimail-mta uses /etc/indimail/control/libindimail to enable indimail virtual domains. It is possible in cases where you upgrade the library, the library name/path changes. In such a case you need to run the following command after upgrade the library (e.g. upgrading MySQL can result in change of the libmysqlclient library name).

    $ sudo svctool --fixsharedlibs

## What is the relationship between /etc/indimail/tcp/open-smtp /etc/indimail/tcp/tcp.smtp and /etc/indimail/tcp/tcp.smtp.cdb and how do they work qmail-smtpd?

If you are running qmail-smtpd under tcpserver, you should add a -x /etc/indimail/tcp/tcp.smtp.cdb option to it. This makes sure that qmail-smtpd checks in that file for allowed relaying (on top of the ones in /etc/indimail/control/rcpthosts).

The file /etc/indimail/tcp/tcp.smtpd needs to be created manually first. After that run the [qmailctl](https://github.com/mbhangui/indimail-mta/wiki/qmailctl.8) command to create the cdb file (/etc/indimail/tcp/tcp.smtp.cdb)

    $ sudo qmailctl cdb

This will enable the roaming for all hosts/networks you put in /etc/indimail/tcp/tcp.smtp. This is necessary if you want to use your mail server from the local network.

Secondly, check the permissions of /etc/indimail/tcp directory and the files in /etc/indimail/tcp. They should be owned by the user running the tcpserver with the IMAP/POP3 server as that user needs to update those files.

Everytime a user logs into the POP3/IMAP server, his ip and a timestamp gets written to /etc/indimail/tcp/open-smtp. If he's allowed in, the tcp.smtp.cdb gets regenerated automatically from the open-smtp file and the tcp.smtp file.

## I want to have IndiMail access mysql as indimail and not root. What sql commands to I run?

Read this [document](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#setting-up-mysql)

## I have changed my IP address on a clustered setup. What gets affected

1. The following tables

    1. fstab (host)
    2. host_table (ipaddr)
    3. dbinfo (server, mdahost)
    4. ip_alias_map (ipaddr)
    5. smtp_port (host, src_host)

2. The control file <u>hostip</u>

3. On the relay servers the file <u>smtproutes</u>. You can use the program ipchange to change IP address in IndiMail tables

## How to put your custom filters before queuing mail

Read this chapter on [filters](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#writing-filters-for-indimail)

## I have an atrn domain. I want to set quota while delivering

    $ sudo vadddomain -t etrn.dom
    $ sudo vadduser postmaster@etrn.dom pass -q quota_in_bytes
    $ sudo vatrn -i etrn.dom postmaster@etrn.dom

Have the following entry in .qmail-default

    |/usr/sbin/vfilter '' homedir/Maildir

where <u>homedir</u> is the home directory created by vadduser for the user postmaster. See the documentation on [vatrn](https://github.com/mbhangui/indimail-mta/wiki/vatrn.1) for all options.

## I have an atrn domain called etrn.dom. My users are split across multiple servers. How do I distribute mails for my users according to the server on which they belong?

1. Create a virtual domain (e.g. mysme.com). This domain will just act as a container domain for atrn MAPS.

    $ sudo vadddomain -t mysme.com

2. Create artificial ATRN domains for each server with ATRN access.

    $ vadddomain -t chn.mysme.com
    $ vadddomain -t mum.mysme.com

    NOTE: If you want to set quota, edit the file /var/indimail/autoturn/chn.mysme.com/Maildir/maildirsize

3. Create rule for each user's mail to get distributed to the subdomain Maildir

    $ valias -i /var/indimail/autoturn/chn.mysme.com/Maildir/ chn_user1@mysme.com
    $ valias -i /var/indimail/autoturn/mum.mysme.com/Maildir/ mum_user1@mysme.com
    $ valias -i /var/indimail/autoturn/mum.mysme.com/Maildir/ mum_user2@mysme.com

    See the documentation for [valias](https://github.com/mbhangui/indimail-mta/wiki/valias.1) for all options.

4. Create the .qvirtual file in each of these subdomains to etrn.dom

    echo mysme.com > /var/indimail/autoturn/chn.mysme.com/.qvirtual
    echo mysme.com > /var/indimail/autoturn/mum.mysme.com/.qvirtual

5. Create atrn maps (users having access to the above atrn domains)

    $ vadduser master_chn@mysme.com xxxxxxxx # For auth and atrn
    $ vatrn -i chn.mysme.com master_chn@mysme.com

    $ vadduser master_mum@mysme.com xxxxxxxx
    $ vatrn -i mum.mysme.com master_mum@mysme.com

    .qvirtual allows mails for a main domain to be distributed across multiple directories in /var/indimail/autoturn directory. i.e. if etrn.dom is the main domain and mails have been split into directories location1.etrn.dom and location2.etrn.dom specify .qvirtual having etrn.dom in /var/indimail/autoturn/location1.etrn.dom and /var/indimail/autoturn/location2.etrn.dom.
    .qvirtual also allows mails for a domain to be delivered to any directory and the domain identified by looking up the .qvirtual file
    xxxxxxxx is the password for the above users.

    See the documentation on [vatrn](https://github.com/mbhangui/indimail-mta/wiki/vatrn.1) for all options.

## How do I setup stupid disclaimers

Disclaimers are stupid and organizations too if they want disclaimers.

    http://www.goldmark.org/jeff/stupid-disclaimers

However if your organization still insists on stupidity read this [Setting Disclaimers in your emails](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#setting-disclaimers-in-your-emails)

## How do i delete mails lying in a queue having a certain pattern. Can I use regex

Use [qmail-rm](https://github.com/mbhangui/indimail-mta/wiki/qmail-rm.1) command. It can quickly delete mails matching a given pattern.

## I have been hit by virus. My virus scanner is not able to handle the load. What should I do?

Use the signatures or the bodycheck control files. For external virus scanner you can set VIRUSCHECK variable to 4, 5 or 6 which uses qscanq to run an external virus scanner

## How do I control access to SMTP for specific domains from specific or set of IP addresses

Use the hostaccess control file. e.g. to allow mails from yahoo.com from 2 IP addresses, 210.210.122.80 and 210.210.122.81 have the following in hostaccess control file.

    yahoo.com:210.210.122.80-81

The behaviour of hostaccess can be modified by setting the PARANOID or DOMAIN\_MASQUERADE environment variables. See qmail-smtpd(8) for more details

## How do I restrict and control mail transactions between senders and recipients

Read this [SMTP Access List](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki#smtp-access-list)

## I want to run a program every time post imap/pop3 authentication for every user.

Define the environment variable POSTAUTH pointing to an executable which you desire to run. This can be defined in the IMAP/POP3 variables directory. e.g. to enable it for IMAPS, do the following.

    $ sudo sh -c "echo "path_to_program" > /service/qmail-imapd-ssl.993/variables/POSTAUTH"
    $ sudo sh -c "echo "path_to_program" > /service/qmail-imapd.143/variables/POSTAUTH"
    $ sudo sh -c "echo "path_to_program" > /service/qmail-pop3d-ssl.995/variables/POSTAUTH"
    $ sudo sh -c "echo "path_to_program" > /service/qmail-pop3d.110/variables/POSTAUTH"

Restart the above services using the `svc -r` command. If you have your own scripts to start these services, then you need to set the POSTAUTH environment variable in your script.

## I want to run a program just once in a lifetime post imap/pop3 authentication for every user.

Define the environment variable MIGRATEUSER pointing to an executable which you desire to run. Additionally define MIGRATEFLAG which should be a name of a file without any path component. e.g. indimail.txt is a valid name but /tmp/indimail.txt is invalid. After running the program, IndiMail will create a 0 bytes file and prevent further invocation of the program. If the program is again desired to be run, either delete the file defined by MIGRATEFLAG or change the value of MIGRATEFLAG environment variable in the IMAP/POP3 variables directory.

This scheme of defining MIGRATEUSER and MIGRATEFLAG is useful for doing adhoc migrations.

## How do I have a wildcard smtp route?

Having the following entry in /etc/indimail/control/smtproutes

    :202.144.77.55:25

directs all remote deliveries to the host 202.144.77.55 on port 25

## How do I use postfix with IndiMail?

IndiMail has a wrapper to vdelivermail called 'postdel' for postfix.

1. Add the following two lines to main.cf

    mydestination=indimail.org,satyam.net.in # domains added by vadddomain
    local_transport=vdel
    vdel_destination_recipient_limit=1

2. Add the following to master.cf

    vdel     unix  -       n       n       -       100       pipe
    flags=Fq. user=indimail argv=/usr/sbin/postdel -f -u $user -d $recipient -r $sender


## Controlling the appearance of outgoing messages (using qmail-inject/sendmail)

1. How do I set up host masquerading? All the users on this host, webmail1.indimail.org, are users on indimail.org. When raj sends a message to prem, the message should say ``From: raj@indimail.org'' and ``To: prem@indimail.org'', without ``webmail1'' anywhere.

    $ sudo sh -c "echo indimail.org > /etc/indimail/control/defaulthost"
    $ sudo chmod 644 /etc/indimail/control/defaulthost.

2. How do I set up user masquerading? I'd like my own From lines to show "The Boss" <boss@indimail.org> rather than god@heaven.indimail.org.

Add MAILHOST=indimail.org, MAILNAME="The Boss" and MAILUSER=boss to your environment. To override From lines supplied by your MUA, add QMAILINJECT=f to your environment.

    $ echo indimail.org > ~/.defaultqueue/MAILHOST
    $ echo boss         > ~/.defaultqueue/MAILUSER
    $ echo "The Boss"   > ~/.defaultqueue/MAILNAME
    $ echo f            > ~/.defaultqueue/QMAILINJECT

3. How do I set up Mail-Followup-To automatically? When I send a message to the sos@heaven.indimail.org mailing list, I'd like to include ``Mail-Followup-To: sos@heaven.indimail.org''

Add QMAILMFTFILE=$HOME/.lists to your environment, and put sos@heaven.indimail.org into ~/.lists.

    $ echo $HOME/.lists > ~/.defaultqueue/QMAILMFTFILE
    $ echo sos@heaven.indimail.org > ~/.lists

## Routing outgoing messages

1. How do I send local messages to another host? All the mail for indimail.org should be delivered to our disk server, pokey.indimail.org. I've set up an MX from indimail.org to pokey.indimail.org, but when a user on the indimail.org host sends a message to boss@indimail.org, indimail.org tries to deliver it locally. How do I stop that?

Remove indimail.org from /etc/indimail/control/locals. If qmail-send is running, give it a HUP. Make sure the MX is set up properly before you do this. Also make sure that pokey can receive mail for indimail.org. Read point 1 in [Routing incoming messages by host](#routing-incoming-messages-by-host). Also create a [wildcard smtproute](#-how-do-i-have-a-wildcard-smtp-route) to send mail mails to pokey.indimail.org

2. How do I set up a null client? I'd like zippy.indimail.org to send all mail to bigbang.indimail.org

    $ sudo sh -c "echo :bigbang.indimail.org > /etc/indimail/control/smtproutes"
    $ sudo chmod 644 /etc/indimail/control/smtproutes.

Disable local delivery as in question 50.1. Turn off qmail-smtpd in /service*/qmail.smtpd*.

3. How do I send outgoing mail through UUCP? I need qmail to send all outgoing mail via UUCP to my upstream UUCP site, gonzo.

Put

    :alias-uucp

into /etc/indimail/control/virtualdomains and

    |preline -df /usr/bin/uux - -r -gC -a"${SENDER:-MAILER-DAEMON}" gonzo!rmail "($DEFAULT@$HOST)"

(all on one line) into ~alias/.qmail-uucp-default. (For some UUCP software you will need to use -d instead of -df.) If qmail-send is running, give it a HUP.

4. How do I set up a separate queue for a SLIP/PPP link?

Deliver to a maildir. Configure ETRN/AUTOTURN or ATRN to access the maildir and deliver the mails.

5. How do I deal with ``CNAME lookup failed temporarily''? The log showed that a message was deferred for this reason. Why is qmail doing CNAME lookups, anyway?

The SMTP standard does not permit aliased hostnames, so qmail has to do a CNAME lookup in DNS for every recipient host. If the relevant DNS server is down, qmail defers the message. It will try againsoon.

## Routing incoming messages by host

1. How do I receive mail for another host name? I'd like our disk server, pokey.indimail.org, to receive mail addressed to indimail.org. I've set up an MX from indimail.org to pokey.indimail.org, but how do I get pokey to treat indimail.org as a name for the local host?

Add indimail.org to /etc/indimail/control/locals and to /etc/indimail/control/rcpthosts. If qmail-send is running, give it a HUP(or do svc -h /service/*-send.* if qmail-send is supervised).

2. How do I set up a virtual domain? I'd like any mail for indimail.org, including root@indimail.org and postmaster@indimail.org and so on, to be delivered to Manny. I've set up the MX already.

Answer: Put

    indimail.org:manny

into /etc/indimail/control/virtualdomains. Add indimail.org to /etc/indimail/control/rcpthosts. If qmail-send is running, give it a HUP (or do svc -h /service/*-send.* if qmail-send is supervised).

Now mail for whatever@indimail.org will be delivered locally to `manny-whatever`. Manny can set up `~manny/.qmail-default` to catch all the possible addresses, `~manny/.qmail-info` to catch info@indimail.org, etc.

3. How do I set up several virtual domains for one user? Manny wants another virtual domain, everywhere.org, but he wants to handle indimail.org users and everywhere.org users differently. How can we do that without setting up a second account?

Answer: Put two lines into /etc/indimail/control/virtualdomains:

    indimail.org:manny-nowhere
    everywhere.org:manny-everywhere

Add indimail.org and everywhere.org to /etc/indimail/control/rcpthosts. If qmail-send is running, give it a HUP (or do

    $ sudo svc -h /service*/*-send.* (if qmail-send is supervised).

Now Manny can set up separate .qmail-nowhere-* and everywhere-* files. He can even set up .qmail-nowhere-default and .qmail-everywhere-default.

## Routing incoming messages by user

1. How do I forward unrecognized usernames to another host? I'd like to set up a LUSER\_RELAY pointing at bigbang.indimail.org.

Put

    | forward "$LOCAL"@bigbang.indimail.org

into ~alias/.qmail-default.

2. How do I set up a mailing list? I'd like me-sos@my.host.name to be forwarded to a bunch of people.

Put a list of addresses into ~me/.qmail-sos, one per line. Then incoming mail for me-sos will be forwarded to each of those addresses. You should also touch ~me/.qmail-sos-owner so that bounces come back to you rather than the original sender.

Alternative: ezmlm (http://pobox.com/~djb/ezmlm.html) is a modern mailing list manager, supporting automatic subscriptions, confirmations, archives, fully automatic bounce handling (including warnings to subscribers saying which messages they've missed), and more.

## How do I use procmail with qmail?

Put

    | preline procmail

into ~/.qmail. You'll have to use a full path for procmail unless procmail is in the system's startup PATH. Note that procmail will try to deliver to /var/spool/mail/$USER by default; to change this, see INSTALL.mbox.

4. How do I use elm's filter with qmail?

Put

    | preline filter

into ~/.qmail. You'll have to use a full path for filter unless filter is in the system's startup PATH.

5. How do I create aliases with dots? I tried setting up ~alias/.qmail-P.D.Q.Bach, but it doesn't do anything.

Use .qmail-p:d:q:bach. Dots are converted to colons, and uppercase is converted to lowercase.

6. How do I use sendmail's .forward files with qmail?

    Use dot-forward

7. How do I use sendmail's /etc/aliases with qmail?

    Use fastforward

8. How do I make qmail defer messages during NFS or NIS outages? If ~joe suddenly disappears, I'd like mail for joe to be deferred.

Build a qmail-users database, so that qmail no longer checks home directories and the password database. This takes three steps.

First, put your complete user list (including local and NIS passwords) into /etc/indimail/users/passwd.

Second, run qmail-pw2u

    $ sudo qmail-pw2u -h < /etc/indimail/users/passwd > /etc/indimail/users/assign

Here -h means that every user must have a home directory; if you happen to run qmail-pw2u during an NFS outage, it will print an error message and stop.

Third, run qmail-newu

    $ sudo qmail-newu

Make sure to rebuild the database whenever you change your user list.

9. How do I change which account controls an address? I set up ~alias/.qmail-www, but qmail is looking at ~www/.qmail instead.

If you do

    $ sudo chown root ~www

then qmail will no longer consider www to be a user; see qmail-getpw.0. For more precise control over address assignments, see qmail-users.0.

## Setting up servers

1. How do I run qmail-smtpd under tcpserver? inetd is barfing at high loads, cutting off service for ten-minute stretches. I'd also like better connection logging.

tcpserver -u 7770 -g 2108 0 smtp /usr/sbin/qmail-smtpd &

into your system startup files. Replace 7770 and 2108 with your indimail uid, gid. Don't forget the &. The change will take effect at your next reboot.

By default, tcpserver allows at most 40 simultaneous qmail-smtpd
processes. To raise this limit to 400, use tcpserver -c 400. To keep track of who's connecting and for how long, run (on two lines)

    tcpserver -v -u 7770 -g 2108 0 smtp /usr/sbin/qmail-smtpd 2>&1 | /usr/sbin/splogger smtpd 3 &

2. How do I set up qmail-qmtpd?

If you have tcpserver installed, set up

    tcpserver -u 7770 -g 2108 0 qmtp /usr/sbin/qmail-qmtpd &

replacing 7770 and 2108 with the indimail uid and gid.

3. How do I allow selected clients to use this host as a relay? I see that qmail-smtpd rejects messages to any host not listed in /etc/indimail/control/rcpthosts.

Three steps. First, install tcp-wrappers, available separately, including hosts\_options. Second, change your qmail-smtpd line in inetd.conf to

    smtp stream tcp nowait qmaild /usr/local/bin/tcpd /bin/tcp-env /usr/sbin/qmail-smtpd

(all on one line) and give inetd a HUP. Third, in tcpd's hosts.allow, make a line setting the environment variable RELAYCLIENT to the empty string for the selected clients:

    tcp-env: 1.2.3.4, 1.2.3.5: setenv = RELAYCLIENT

Here 1.2.3.4 and 1.2.3.5 are the clients' IP addresses. qmail-smtpd ignores /etc/indimail/control/rcpthosts when RELAYCLIENT is set. (It also appends RELAYCLIENT to each envelope recipient address. See question 53.4 for an application.)

Alternative procedure, if you are using tcpserver: Create /etc/indimail/tcp/tcp.smtp containing

    1.2.3.6:allow,RELAYCLIENT=""
    127.:allow,RELAYCLIENT=""

to allow clients with IP addresses 1.2.3.6 and 127.*.

After creating the above entry run tcprules by executing

    $ sudo qmailctl cdb

Finally, insert

    -x /etc/indimail/tcp/tcp.smtp.cdb

after tcpserver in your qmail-smtpd invocation.

## How do I fix up messages from broken SMTP clients?

Three steps. First, put

    | bouncesaying 'Permission denied' [ "@$HOST" != "@fixme" ]
    | qmail-inject -f "$SENDER" -- "$DEFAULT"

into ~alias/.qmail-fixup-default.

Second, put

    fixme:fixup

into /etc/indimail/control/virtualdomains, and give qmail-send a HUP.

Third, follow the procedure in question 5.4, but set RELAYCLIENT to the string ``@fixme'':

    tcp-env: 1.2.3.6, 1.2.3.7: setenv = RELAYCLIENT @fixme

Here 1.2.3.6 and 1.2.3.7 are the clients' IP addresses. If you are using tcpserver instead of inetd and tcpd, put

    1.2.3.6:allow,RELAYCLIENT="@fixme"
    1.2.3.7:allow,RELAYCLIENT="@fixme"

into /etc/indimail/tcp/tcp.smtp, and run tcprules as in question 53.3.

## How do I set up qmail-qmqpd? I'd like to allow fast queueing of outgoing mail from authorized clients.

Make sure you have installed tcpserver. Create /etc/indimail/tcp/qmqp.tcp in tcprules format to allow connections from authorized hosts. For example, if queueing is allowed from 1.2.3.\*:

    1.2.3.:allow
    :deny

Convert /etc/indimail/tcp/qmqp.tcp to /etc/indimail/tcp/qmqp.cdb:

    $ sudo qmailctl cdb

Finally, set up

tcpserver -x /etc/indimail/tcp/qmqp.cdb -u 7770 -g 2108 0 628 /usr/sbin/qmail-qmqpd &

replacing 7770 and 2108 with the indimail uid and gid.

## How do I set up closed user group mailing

Follow the two simple steps below

    1. Set the environment variable CUGMAIL - qmail-smtpd accepts mail only from local users (domains in rcpthosts)
    2. unset the environment variable CHECKRELAY - This stops relaying of mails (i.e. domains not in rcpthosts)

## How do I reject mails from open relays

Set the following in the qmail-smtpd run file

    tcpserver -u 7770 -g 2108 0 smtp sh -c '
    if tcpclient -T2 -RH -l0 $TCPREMOTEIP 25 /sbin/relaytest
    then
      env OPENRELAY=1 /usr/sbin/qmail-smtpd
    else
      /usr/sbin/qmail-smtpd
    fi'

## How do I use a MDA different from vdelivermail

You can either set this in the .qmail-default file or in case you are using vfilter in .qmail-default, set the environment variable MDA (in the qmail-send variables directory)

## How do I implement an efficient virus scanner using clamav

    1. Install clamav http://www.clamav.net

       Download and extract clamav-0.94.2.tar.gz
       ./configure –prefix=/usr -sysconfdir=/etc/indimail \
       --with-user=qscand –with-group=qscand
       make
       su root
       make install-strip
       groupadd qscand
       useradd -g qscand -G qmail -d /var/indimail/qscanq qscand

    2. Define the environment variables

       QHPSI=/bin/clamdscan %s --quiet –disable-summary”
       QHPSIRC=1
       QHPSIRN=0
       REJECTVIRUS=””
       in the variables directory or the run file for qmail-smtpd

    3. Configure /etc/clamd.d/scan.conf with the following

       LogFile stderr
       DatabaseDirectory /var/indimail/clamd
       LocalSocket /run/clamd.scan/clamd.sock
       FixStaleSocket
       User qscand
       AllowSupplementaryGroups
       Foreground
       ScanPE
       DetectBrokenExecutables
       ScanOLE2
       ScanMail
       ScanHTML
       ScanArchive

    4. Install Services freshclam and clamd
    
       svctool --qscanq --servicedir=/service --clamdPrefix=/usr --sysconfdir=/etc/clamd.d

## How do I setup SSL encryption for SMTP, IMAP, POP3

You can either use a certificate from a Certificate Authoriity like verisign or generate your own self-signed certificate

Certificate from CA

    1. Download your certificate files from your DigiCert Customer Account.

    2. Create a combined servercert.pem certificate file
Once you have downloaded your Certificate files from your DigiCert Web-PKI Customer Account, gather your new certificate files and the private key you generated when you created your CSR. Open a text editor and paste the contents of each key/certificate one after another in the following order:

    1. The Private Key (your_domain_name.key)
    2. The Primary Certificate (your_domain_name.cert)
    3. The Intermediate Certificate (DigiCertCA.crt)
    4. The Root Certificate (TrustedRoot.crt)

Self-Signed Certificate

Just run the command

    $ sudo svctool –-postmaster=postmaster@indimail.org –-config=cert

(replace indimail.org with your domain)


## Configuring MUAs to work with qmail

1. How do I make BSD mail generate a Date with the local time zone? When I send mail, I'd rather use the local time zone than GMT, since some MUAs don't know how to display Date in the receiver's time zone.

Put

    set sendmail=/bin/datemail

into your .mailrc or your system-wide Mail.rc. Beware that BSD mail is neither secure nor reliable.

2. How do I make pine work with qmail?

Put

    sendmail-path=/usr/lib/sendmail -oem -oi -t

into /usr/local/lib/pine.conf. (This will work with sendmail too.) Beware that pine is neither secure nor reliable.

3. How do I make MH work with qmail?

Put

    postproc: /usr/mh/lib/spost

into each user's .mh\_profile. (This will work with sendmail too.) Beware that MH is neither secure nor reliable.

## Managing the mail system

1. How do I safely stop qmail-send? Back when we were running sendmail, it was always tricky to kill sendmail without risking the loss of current deliveries; what should I do with qmail-send?

Go ahead and kill the qmail-send process. It will shut down cleanly. Wait for ``exiting'' to show up in the log. To restart qmail, do svc -t /service*/*-send.* The supervise process will kill qmail, wait for it to stop, and restart it.  Use -d instead of -t if you don't want qmail to restart automatically; to manually restart it, use -u.

2. How do I manually run the queue? I'd like qmail to try delivering all the remote messages right now.

Give the qmail-daemon process an ALRM.

    $ sudo svc -a /service/*-send.*

You may want to run qmail-tcpok first, to guarantee that qmail-remote will try all addresses. Normally, if an address fails repeatedly, qmail-remote leaves it alone for an hour.

3. How do I rejuvenate a message? Somebody broke into indimail's computer again; it's going to be down for at least another two days. I know Ramraj has been expecting an important message---in fact, I see it sitting here in /queue/queue1/mess/15/26902. It's been in the queue for six days; how can I make sure it isn't bounced tomorrow?

Just touch /queue/queue1/info/15/26902. (This is the only form of queue modification that's safe while qmail is running.)

## How do I organize a big network? I have a lot of machines, and I don't know where to start.

First, choose the domain name where your users will receive
mail. This is normally the shortest domain name you control. If you are in charge of *.indimail.org, you can use addresses like prem@indimail.org.

Second, choose the machine that will know what to do with different users at indimail.org. Set up a host name in DNS for this machine:

    mailhost.indimail.org IN A 1.2.3.4
    4.3.2.1.in-addr.arpa IN PTR mailhost.indimail.org

Here 1.2.3.4 is the IP address of that machine.

Third, make a list of machines where mail should end up. For example, if mail for Raj should end up on Raj's workstation, put Raj's workstation onto the list. For each of these machines, set up a host name in DNS:

    rajshost.indimail.org IN A 1.2.3.7
    7.3.2.1.in-addr.arpa IN PTR rajshost.indimail.org

Fourth, install qmail on rajshost.indimail.org. qmail will automatically configure itself to accept messages for raj@rajshost.indimail.org and deliver them to ~raj/Mailbox on rajshost. Do the same for the other machines where mail should end up.

Fifth, install qmail on mailhost.indimail.org. Put

    indimail.org:alias-movie

into /etc/indimail/control/virtualdomains on mailhost. Then forward raj@indimail.org to raj@rajshost.indimail.org, by putting

    raj@rajshost.indimail.org

into ~alias/.qmail-movie-raj. Do the same for other users.

Sixth, put indimail.org into /etc/indimail/control/rcpthosts on mailhost.indimail.org, so that mailhost.indimail.org will accept messages for users at indimail.org.

Seventh, set up an MX record in DNS to deliver indimail.org messages to mailhost:

    indimail.org IN MX 10 mailhost.indimail.org

Eighth, on all your machines, put indimail.org into /etc/indimail/control/defaulthost.

## How do I back up and restore the queue disk?

You can't.

One difficulty is that you can't get a consistent snapshot of the queue while qmail-send is running. Another difficulty is that messages in the queue must have filenames that match their inode numbers.

However, the big problem is that backups---even twice-daily backups--- are far too unreliable for mail. If your disk dies, there will be very little overlap between the messages saved in the last backup and the messages that were lost.

There are several ways to add real reliability to a mail server. Battery backups will keep your server alive, letting you park the disk to avoid a head crash, when the power goes out. Solid-state disks have their own battery backups. RAID boxes let you replace dead disks without losing any data.

## How do i immediately schedule messages

    i=0
    while true
    do
      sudo find $QUEUEDIR/queue"$i"/info -type f -exec touch {} \;
      i=`expr $i + 1`
      if [$i -eq $QUEUE_COUNT] ; then
          break
      fi
    done
    sudo /usr/sbin/qmail-tcpok
    sudo svc -a /service/*-send.*
    
Where QUEUEDIR is the directory where IndiMail's queue has been
created by queue-fix

## Is it safe to simply delete old file, or is there a cleaner way?

You can't just delete a file, no. Each message is represented in the queue by several files in the various queue/subdirectories.

The entry in the 'info/' directory is key, since it identifies the sender and qmail uses the last-modified timestamp of that file to determine when to expire a message.

The following example expires and bounces all mails in the IndiMail queue

    i=0
    while true
    do
      find $QUEUEDIR/queue"$i"/info -type f -exec \
      sudo touch -c --date '10 days ago' {} \;
      i=`expr $i + 1`
      if [$i -eq $QUEUE_COUNT] ; then
        break
      fi
    done

You can force a particular message to expire, then, by using "touch" like this:

    sudo touch -c --date '10 days ago' /var/indimail/queue/queue1/info/15/40288

At the next failed delivery attempt, the message will bounce. This is about the only half-way safe "live" queue modification you can do.

(There's a small possibility of expiring a new message if the one you intended is delivered/expired and then replaced by a new message with the same id by the time you type the command.)

For anything more advanced, you have to stop qmail. There are various queue manipulation tools on the qmail.org website that I expect can do the job (since you may want to just kill the message, not bounce it), like

## How to have a different per-IP-concurrency for some hosts.

The number of concurrent connections allowed by tcpserver is determined by the value of MAXPERIP environment variable. This can be set globally in the variables directory or individually for hosts in the tcprules cdb file.

e.g. the following rules file gives a default per-IP-concurrency limit of 5 except for the host 192.9.200.1 which has 20

    192.9.200.1:allow,MAXPERIP=”20”
    :allow,MAXPERIP=”5”

Don't forget to run tcprules after modifying the rules file.

## Miscellany

1. How do I tell qmail to do more deliveries at once? It's running only 20 parallel qmail-remote processes.

Decide how many deliveries you want to allow at once. Put that number into /etc/indimail/control/concurrencyremote. Restart qmail-send as in question 55.1. If your system has resource limits, make sure you set the descriptors limit to at least double the concurrency plus 5; otherwise you'll get lots of unnecessary deferrals whenever a big burst of mail shows up. Note that qmail also imposes a compile-time concurrency limit, 120 by default; this is set in conf-spawn.

2. How do I keep a copy of all incoming and outgoing mail messages?

Set EXTRAQUEUE environment variable to log.

    echo log > /service/qmail-send.25/variables/EXTRAQUEUE

Put ./msg-log into ~alias/.qmail-log.

You can also use EXTRAQUEUE to, e.g., record the Message-ID of every message: run

    | awk '/^$/ { exit } /^[mM][eE][sS][sS][aA][gG][eE]-/ { print }'

You can also forward all mails to external@domain.com

    | awk '/^$/{exit}/^X-Queue-Extra: yes$/{exit 99}'
    |(echo 'X-Qmail-Extra: yes'; cat ) |forward external@domain.com
    ./Maildir/

from ~alias/.qmail-log.

3. How do I switch slowly from sendmail to qmail? I'm thinking of moving the heaven.indimail.org network over to qmail, but first I'd like to give my users a chance to try out qmail without affecting current sendmail deliveries. We're using NFS.

Find a host in your network, say pc.heaven.indimail.org, that isn't
running an SMTP server. (If addresses at pc.heaven.indimail.org are used, you should already have an MX pointing pc.heaven.indimail.org to your mail hub.)

Set up a new MX record pointing lists.heaven.indimail.org to pc.heaven.indimail.org.

Install qmail on pc.heaven.indimail.org. Replace pc with lists in the control files. Make the qmail man pages available on all your machines.

Now tell your users about qmail. A user can forward joe@heaven.indimail.org to joe@lists.heaven.indimail.org to get ~/Mailbox delivery; he can set up .qmail files; he can start running his own mailing lists @lists.heaven.indimail.org.

When you're ready to turn sendmail off, you can set up pc.heaven.indimail.org as your new mail hub. Add heaven.indimail.org to /etc/indimail/control/locals, and change the heaven.indimail.org MX to point to pc.heaven.indimail.org. Make sure you leave lists.heaven.indimail.org in /etc/indimail/control/locals so that transition addresses will continue to work.

4. How do I send mails from a Maildir

Learn how to use maildirsmtp. It will use the Delivered-To header for the prefix. e.g. Assuming all mails for indimail.org have been delivered to the directory /mail/tempMaildir, the following will deliver all mails to the host mx.indimail.org

    maildirsmtp /mail/tmpMaildir indimail.org mx.indimail.org `hostname`

## How do i test vchkpass, systpass or any other checkpassword implementation for authenticated SMTP

There is a clever way to test your checkpassword with a bit of command line re-direction.

For example, with username manny, password ssh-1.5-,

    printf "manny\0ssh-1.5-\0\0" | /usr/sbin/vchkpass /bin/false 3<&0

You can set the environment variable DEBUG, PASSWD\_CACHE to change the behavior of vchkpass.

## The entire world uses crazy attachments. Any idea on how to block them

1. Using badext, badextpatterns control filename

You can have extensions listed in the badext control file to block them (use badextpatterns for wildcards). Set VIRUSCHECK to one of the four values - 3,4,5 or 7

2. Using bodycheck control file

Have the following in the bodycheck control file to block 54 attachments (please note the line wrap)

    ^Content-Disposition:.*filename=.*(document|readme|doc|text|file|data|test|message|body).(pif|scr|exe|cmd|bat|zip).*:Bad Attachments-header

3. Using Custom Scripts

You can check the Content-Type: header, especially with a helper program like 822field, from the mess822 package. Have the following in user's .qmail

    |bouncesaying "Only plain text, please." check-text

check-text is something like:

    #!/bin/sh
    ct=`822field content-type`
    # If there's no Content-Type: header, it's plaintext
    if [ -z "$ct" ] ; then exit 1 ; fi
    # Otherwise, check that it includes "text/plain"
    echo "$ct" | grep -q 'text/plain' && exit 1

If you want to accept mime messages but only keep the plain text part of them (or failing that to have their html or rich text and html alternatives rendered as plain text), these are some options ("demime" seems to be still maintained - not sure about the others):

    emime: http://scifi.squawk.com/demime.html
    mimefilter: http://ftp.br.debian.org/debian/pool/main/m/mimefilter/

    stripmime: http://www.phred.org/~alex/stripmime.html
    stripmime: http://www.clarity.net/~adam/stripmime/

Note: There are two different tools called "stripmime".

In principle you could use them in conjunction through procmail. There is also the "no-alternative" package of Russell Nelson, which has only basic functionality (no attempt at transforminh other formats to plain text):

http://www.qmail.org/no-alternative

## How to I prevent spoofing of my domain in mails.

Use either of the two methods described below. Please note that however the second method requires your outgoing SMTP server to be different from your incoming (MX) SMTP server

    1. Set the environment variable CHECKSENDER in qmail-smtpd run file or the variables directory (if using envdir). This will force authenticated SMTP or POP/IMAP before SMTP for my own domains before accepting mails.
    2. Set up badmailfrom control file containing my local domain on the qmail-smtpd server serving as MX

## qmail doesn't deliver mail to superusers

To prevent the possibility of qmail-local running commands as a privileged user, qmail ignores all users whose UID is 0. This is documented in the qmail-getpw man page.

That doesn't mean qmail won't deliver to root, it just means that such a delivery will have to be handled by a non-privileged user. Typically, one creates an alias for root by populating

    ~alias/.qmail-root.

## qmail doesn't deliver mail to users who don't own their home directory

Another security feature, and just good general practice. This is documented in the qmail-getpw man page.

## qmail doesn't deliver mail to users whose usernames contain uppercase letters

qmail converts the entire "local part"--everything left of the "@" in an address, to lowercase. The man page doesn't come out and say that, but the code does. The fact that it ignores users with uppercase characters is documented in the qmail-getpw man page.

## qmail replaces dot(.) in extension addresses with colons(:)

Another security feature. The purpose is prevent extension addresses from backing up the file tree using "..". By replacing them with colons, qmail ensures that all .qmail files for a user are under their home directory. Documented in the dot-qmail man page.

## qmail converts uppercase characters in extension addresses to lowercase

This is another result of the fact that qmail lowercases the entire local part of addresses. Documented in the dot-qmail man page.

## qmail doesn't use /etc/hosts

qmail never uses /etc/hosts to determine the IP address associated with a host name. If you use names in control files, qmail must have access to a name server.

It is possible to run qmail on systems without access to a name server, though. Hosts in control files can be specified by IP address by enclosing them in square brackets ([]), e.g.:

[10.1.2.219]

Actually, the square brackets aren't always necessary--but it's a good idea to use them anyway.

## qmail doesn't generate deferral notices

If Sendmail is unable to deliver a message within a few hours, typically four, it sends a deferral notice to the originator. These notices look like bounce messages, but don't indicate that the delivery has failed permanently, yet.

qmail doesn't send such warnings. An undeliverable message will only be returned to the originator after it spends queuelifetime in the queue.

## qmail is slow if queue/lock/trigger is gone/has the wrong permissions/is a regular file

qmail-queue and qmail-send communicate via a named pipe called /var/indimail/queue/queue1/lock/trigger. If this pipe gets messed up, qmail-send doesn't notice new messages for a half hour or so.

The best way to ensure that it's set up right is to run "make check" from the source directory. If that's not possible, make sure it looks like:

    # ls -l /var/indimail/queue/queue1/lock/trigger
    prw--w--w-   1 qmails   qmail           0 Jul  5 21:25 /var/indimail/queue/queue1/lock/trigger

Pay particular attention to the "p" at the beginning of the line (says it's a named pipe), the mode (especially world writable), and the owner/group.

## DNS or IDENT lookups can make SMTP slow

If qmail-smtpd is slow to respond to connections, the problem is probably due to DNS reverse lookups or IDENT lookups. If you're starting qmail-smtpd with tcpserver, remove the "-h", "-p", and "-r" options and add "-H", "-P", "-R", and "-l hostname".

## Carriage Return/Linefeed (CRLF) line breaks don't work

qmail-inject and other local injection mechanisms like sendmail don't work right when messages are injected with DOS-style carriage return/linefeed (CRLF) line breaks. Unlike Sendmail, qmail requires locally-injected messages to use Unix newlines (LF only). This is a common problem with PHP scripts.

## qmail-send or tcpserver stop working if logs back up

If you're logging to a supervised log service, and the log service fails for any reason: disk full, typo in the run script, log directory configuration error, etc., the pipeline will eventually fill up, causing the service to block, or hang. Fix the problem (see Troubleshooting) and everything will return to normal. Run the `svps -a` command to see if any service has a very small uptime.

 ## qmail-inject sets From field to anonymous if USER and LOGNAME aren't set

If a message sent via qmail-inject doesn't contain a From field, qmail-inject looks for environment variables to tell it which user is sending the message. The variables it looks for, in order, are: QMAILUSER, MAILUSER, USER,  and  LOGNAME,

Normal user login sessions usually set one or both of USER and LOGNAME, but some batch jobs, such as those started by cron might not have either of these set.

To cause your cron jobs to have a valid From field, set one these environment variables before sending any mail messages.

## How do i disable qmail from adding any headers related to my host when it relays.

Set the following in your tcp.smtp file

    1.2.3.4:allow,RELAYCLIENT=””,TCPREMOTEIP=””,TCPREMOTEHOST=””

## Is there a mailing list available for IndiMail package?

There are four Mailing Lists for IndiMail

1. indimail-support  - You can subscribe for Support [here](https://lists.sourceforge.net/lists/listinfo/indimail-support). You can mail [indimail-support](mailto:indimail-support@lists.sourceforge.net) for support Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-support)
2. indimail-devel - You can subscribe [here](https://lists.sourceforge.net/lists/listinfo/indimail-devel). You can mail [indimail-devel](mailto:indimail-devel@lists.sourceforge.net) for development activities. Old discussions can be seen [here](https://sourceforge.net/mailarchive/forum.php?forum_name=indimail-devel)
3. indimail-announce - This is only meant for announcement of New Releases or patches. You can subscribe [here](http://groups.google.com/group/indimail)
4. Archive at [Google Groups](http://groups.google.com/group/indimail). This groups acts as a remote archive for indimail-support and indimail-devel.

There is also a [Project Tracker](http://sourceforge.net/tracker/?group_id=230686) for IndiMail (Bugs, Feature Requests, Patches, Support Requests)

## What should I do if I have trouble with IndiMail

Read the documentation! Most questions are answered by

1. this list of frequently answered questions
2. the [IndiMail Wiki](https://github.com/mbhangui/indimail-mta/wiki/0-IndiMail-Wiki)
3. the other README pages in /usr/share/indimail/doc; and
4. the man pages in your system man directory and [Online Man Pages](https://github.com/mbhangui/indimail-mta/wiki/1-Man-Pages).

Your system includes a wide variety of monitoring tools to show  you what IndiMail is doing

1. The IndiMail logs as configured in supervise. These logs are in /var/log/svc directory
2. svps -a command
3. svctool --check-install command
4. qmail-showctl -a command
5. The top command
6. dot-forward -n (if you have installed dot-forward), which lets you see how a .forward file will be interpreted
7. fastforwrad -n (if you have installed fast-forward), which lets you see how a forwarding table will be interpreted
8. ps command, which lets you see what processes are running
9. systemctl status svscan.service command
10. recordio and tcpdump commands , which lets you see what data is flowing over a TCP connection.
11. MRTG graphs in /var/www/html/mailmrtg
12. a syscall tracing tool, trace or truss or strace or ktrace, which lets you see how a program is interacting with the operating system
