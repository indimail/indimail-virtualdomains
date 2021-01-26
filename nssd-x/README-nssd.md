NSSD is a fork of FSSOS. FSSOS stands for Flexible Single Sign-On Solution written by Ben Goodwin.

[Official website](http://fssos.sourceforge.net/)

This source has been hacked to extract the username, domain component from an email address to allow email addresses as usernames. NSSD will also work with vpopmail, by just changing the configuration file - nssd.conf

NSSD is experimental and without warranty.

NSSD can be downloaded from [here](https://github.com/mbhangui/indimail-virtualdomains/tree/master/nssd-x)

NSSD has been modified to have user and domain in the query e.g.

manvendra@indimail.org gets split into manvendra as the user and indimail.org as the domain.

This split allows authentication against IndiMail's MySQL database. By just changing the configuration, authentication should also work for vpopmail. The other change made to NSSD, is to make it run in the foreground and make it supervise(8) friendly.

You may also want to look at the wonderful original code written by Ben.

You may find NSSD of use if you want to run a IMAP/POP3 server which does not yet have support for IndiMail or vpopmail

NSSD allows many IMAP/POP3 servers, which use getpwnam(), getspnam(), PAM, etc to authenticate against IndiMail's database without making a single change to the IMAP/POP3 server code.  This gives a Yet Another Way to have courier-imap, dovecot, etc to authenticate against your own custom MySQL database.

## NSSD - Name Service Switch Daemon

Supported Operating Systems:

    * Linux (glibc >= 2.2.5)
    * Solaris (Sparc or Intel >= 8) (SEE NOTE BELOW)
    * FreeBSD (5.1+, prefer 5.2+)   (SEE NOTE BELOW)

Supported MySQL Versions:

    * MySQL 3.23.9 - 6.0.3-alpha

Supported Compilers:

    * GCC (2.95.2, 3.x)

## Prerequisites

* Installing from source:

  * A functional compile environment (system headers, gcc, ...)
  * MySQL client library & header files (local)
  * MySQL server (local or remote)

## INSTALLATION DETAILS

If installing from source:

```
$ cd /usr/local/src
$ git clone https://github.com/mbhangui/indimail-virtualdomains.git
$ cd /usr/local/src/indimail-virtualdomains/nssd-x
$ ./default.configure
$ make
$ sudo make install-strip
```

On some systems, libtool insists on adding "-lc" to the link stage (due to the way gcc was built for that system), which breaks NSSD threading in daemon mode.  If you see a "-lc" before a "-pthread" or "-lpthread", then you're in trouble.  You'll notice the broken behavior in the form of fewer-than-expected threads running (3) and the inability to kill the parent process off without a "-9" signal. To fix this, do the following:

`PTRHEAD_LIBS="-lpthread -lc" ./configure`

and then run make/make install.

If your MySQL installation is based in a strange directory, use
the --with-mysql=DIR option of ./configure to specify.  For example,

`./configure --with-mysqlprefix=/usr/local`

* Edit /etc/indimail/nssd.conf. You will find nssd.conf in samples directory of the source

* Edit (or create) /etc/nsswitch.conf such that it contains at least the following:

  ```
  passwd: files nssd
  shadow: files nssd
  ```

  If you don't want groups from MySQL, simply don't include 'nssd' in in the 'group' line.

* Start 'nssd' (e.g. "/usr/sbin/nssd" )

  For IndiMail, to install a supervise service, run the svctool command

  ```
  # /usr/sbin/svctool --pwdlookup="/run/indimail/nssd.sock"
      --threads="5" --timeout="-1" --mysqlhost="localhost"
      --mysqluser="indimail" --mysqlpass="ssh-1.5-"
      --mysqlsocket="/var/run/mysqld/mysqld.sock"
      --servicedir="/service"
  ```

  The above command will create /service/pwlookup/run as below

  ```
  #!/bin/sh
  # $Id: svctool.in,v 2.478 2020-05-24 23:55:49+05:30 Cprogrammer Exp mbhangui $
  # generated on x86_64-pc-linux-gnu on Monday 25 May 2020 06:35:11 PM IST
  # /usr/sbin/svctool --pwdlookup="/run/indimail/nssd.sock" --threads="5" --timeout="-1" --mysqlhost="localhost" --mysqluser="indimail" --mysqlpass="ssh-1.5-" --mysqlsocket="/var/run/mysqld/mysqld.sock" --servicedir="/service"

  if [ -d /run ] ; then
    mkdir -p /run/indimail
    chown indimail:indimail /run/indimail
    chmod 775 /run/indimail
  elif [ -d /var/run ] ; then
    mkdir -p /var/run/indimail
    chown indimail:indimail /var/run/indimail
    chmod 775 /var/run/indimail
  fi
  exec 2>&1
  exec /usr/bin/envdir /service/pwdlookup/variables \
      /usr/bin/setuidgid indimail /usr/sbin/nssd -d notice
  ```

* Test pwlookup using a virtual user configured in indimail/vpopmail
  `# /usr/libexec/indimail/check_getpw user@domain`

## Example usage of nssd - Configure Dovecot to work with IndiMail

IndiMail stores it's virtual user information in MySQL. However, IndiMail can work with virtually any IMAP/POP3 server which has a mechanism to authenticate using PAM and can use the system's passwd database for user's home directory. This is because IndiMail provides a PAM module and **nssd** NSS service . The beauty of providing both PAM and NSS is that you do not have to modify a single line of code anywhere. In this respect, IndiMail is probably the most flexible messaging server available at the moment.

Dovecot is an open source IMAP and POP3 server for Linux/UNIX-like systems, written with security primarily in mind. Dovecot is an excellent choice for both small and large installations. It's fast, simple to set up, requires no special administration and it uses very little memory. Though I do not use dovecot, I have heard excellent reviews from users about dovecot. It took me less than 20 minutes to download dovecot today and have it working with IndiMail with all existing mails intact and accessible. So at the moment, my IndiMail installation is working with both courier-imap and dovecot simultaneously (with different IMAP/POP3 ports assigned to courier-imap and dovecot).

Like most of imap/pop3 servers, dovecot is configurable and can use multiple methods to authenticate and as well get other information about the user such as home directory, user id, etc.

IndiMail provides pam-multi(8) as a flexible Password Authentication Module. For providing the userdb information using the standard passwd mechanism, IndiMail provides the pwdlookup service. The pwdlookup service uses nssd(8) daemon which provides Name Service Switch. NSS provides a mechanism by which standard functions, which look into /etc/passwd, /etc/shadow, can be extended to look into external sources. nssd provides IndiMail's database as an alternate UNIX configuration database for /etc/passwd, /etc/shadow and /etc/group. The additional source for passwd database can be enabled by adding 'nssd' in /etc/nsswitch.conf as an alternate source for passwd database.

```
$ egrep "passwd|shadow" /etc/nsswitch.conf
#     passwd: sss files
#     passwd: files
#     passwd: sss files # from profile
passwd:     sss files systemd nssd
# passwd:    db files
# shadow:    db files
shadow:     files sss nssd
```

pam-multi along with pwdlookup services makes it easy to have dovecot work with IndiMail without modifying a single line of code of dovecot. You just need to configure 3 additonal config files - /etc/indimail/nssd.conf, /etc/pam.d/pam-multi and /etc/dovecot.conf. Here is what is required

**File /etc/indimail/nssd.conf**

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

**File /etc/pam.d/pam-multi**
```
auth     required  pam-multi.so args -s /usr/lib/indimail/modules/iauth.so
account  required  pam-multi.so args -s /usr/lib/indimail/modules/iauth.so
#pam_selinux.so close should be the first session rule
session  required  pam_selinux.so close
#pam_selinux.so  open should only be followed by sessions to be executed in the user context
session  required  pam_selinux.so open env_params
session  optional  pam_keyinit.so force revoke
```

The above is for fedora. You may have to change the configuration for your OS. Consult your OS pam documentation

If you have installed IndiMail using RPM, you will be having pwdlookup service configured and running. Ensure that pwdlookup service is running

```
$ sudo /usr/bin/svstat /service/pwdlookup
/service/pwdlookup/: up (pid 8397) 1091 seconds
```

To improve passwd lookup performance, you may want to have nscd(8) daemon started.

```
$ /etc/init.d/nscd start
Starting nscd: [ OK ]
```

Finally, the following configuration will be needed for dovecot
**File /etc/dovecot.conf**

```
# User to use for the login process. Create a completely new user for this,
# and don't use it anywhere else. The user must also belong to a group where
# only it has access, it's used to control access for authentication process.
# Note that this user is NOT used to access mails.
login_user = qmaill

#
mail_location = maildir:~/Maildir

# System user and group used to access mails. If you use multiple, userdb
# can override these by returning uid or gid fields. You can use either numbers
# or names.
mail_uid = 555
mail_gid = 555

passdb pam {
# PAM authentication. Preferred nowadays by most systems.
# Note that PAM can only be used to verify if user's password is correct,
# so it can't be used as userdb. If you don't want to use a separate user
# database (passwd usually), you can use static userdb.
# REMEMBER: You'll need /etc/pam.d/dovecot file created for PAM
# authentication to actually work.
# [session=yes] [setcred=yes] [failure_show_msg=yes] [max_requests=]
# [cache_key=] []
args = session=yes pam-multi
}
```

Restart/start dovecot and your user's should be able to access their Maildirs using dovecot using POP3, IMAP, POP3S or IMAPS

Note: IndiMail's pam-multi is installed in /lib/security, lib64/security or /usr/lib/pam depending on your OS.

## NOTE

This version has been packaged as part of [indimail-auth package](https://github.com/mbhangui/indimail-virtualdomains)

Send all bug reports to indimail-auth@indimail.org 
