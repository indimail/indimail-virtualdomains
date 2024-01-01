# Introduction

[pam-multi](https://github.com/indimail/indimail-virtualdomains/tree/master/pam-multi-x) helps you to extend authentication of an existing pam-module (e.g. courier-imap, dovecout, cyrus, etc) to transparently authenticate against [IndiMail's](https://github.com/indimail/indimail-virtualdomains) own MySQL database. The primary goal of pam-multi was to allow SMTP/IMAP/POP# servers to authenticate against IndiMail's MySQL database. In short, pam-multi provides a pam service, which is configurable to look at any datasource. IndiMail uses pam-multi to make possible usage of any SMTP server with IndiMail's database, without modifying a single line of code of the SMTP/IMAP/POP3 server. See the man page pam-multi(8). pam-multi package also contains pam-checkpwd(8) which provides a [checkpassword interface](http://cr.yp.to/checkpwd/interface.html). pam-checkpwd can use pam-multi or any PAM config/module authentication. See the examples below how it can be used to provided authenticated SMTP, IMAP/POP3 login. pam-checkpwd can be used for courier-imap, dovecot authentication too. pam-multi does chdir to the user's home directory and sets the uid, gid before returning.

pam-multi doesn't depend on the user being in /etc/passwd. But your application may need fields like uid, gid, home directory from /et/passwd. If your datasource happens to be in MySQL, you can use nssd(8) to transparently extend your MySQL db into /etc/passwd. If you use nssd, all calls like getpwnam(3), getpwent(3), getspnam(3), getspent(3), etc will read data from your MySQL database with some simple configuration of /etc/indimail/nssd.conf. See the man page of nssd(8) for details. Both pam-multi and nssd are part of the indimail-auth package. In most cases, using nssd is enough make your application authenticate against IndiMail's MySQL db, and you many not need pam-multi.

pam-multi supports four methods for password hashing schemes.

`crypt (DES), MD5, SHA256, SHA512.`

To use pam-multi for your application (which currently uses PAM), you need to modify the PAM configuration file for that application. The following options are supported in configuration file in /etc/pam.d.  See man pam-multi(8) for more details.

All that is needed is for you is to configure the appropriate command which returns an encrypted password for the user. pam-multi will encrypt the plain text password given by the user and compare it with the result fetched by one of the three configured method. If the encrypted password matches, access will be granted. The comparision will utilize one of the four hashing schemes.

* $1$ .. MD5
* $5$ .. SHA256
* $6$ .. SHA512

anything else than above, the hashing scheme will be DES

Three methods are supported for fetching the encrypted password from your database

```
-m sql_statement
   MySQL mode. You will need to specify -u, -p, -d, -H and -P options.
   It is expected that the sql_statement will return a row containing
   the encrypted password for the user.

-c command
   Command mode. pam-multi will do sh -c "command". It is expected that
   the output of the command will be an encrypted password.

-s shared_library
   Library Mode. pam-multi will dynamically load the shared library. It is
   expected for the library to provide the function

   `char *iauth(char *email, char *service)`

   The function iauth() will be passed arguments username, service
   argument denoting the name of service. The service argument will be
   used only for identification purpose. It is expected for the function
   to return a string containing the encrypted password. An example shared
   library iauth.so has been provided in the package. authenticate.so has
   a iauth() function to authenticate against IndiMail's database.
```

The following tokens if present in command string or the sql string (-m or -c options),
will be replaced as follows

* %u - Username
* %d - Domain

A checkpassword compatible program **pam-checkpwd** has been provided in the package. pam-checkpwd can use pam-multi as well as any existing pam service on your system. See man pam-checkpwd(8) for more details.

## Examples PAM Configuration

### System PAM Configuration

If you want an existing application on your system which uses PAM to use pam-multi, you will neeed to modify the configuration file which your application uses to have any one of the three options below.

```
1. auth sufficient pam-multi.so args -m [select pw\_passwd from indimail where pw\_name=’%u’ and pw\_domain=’%d’] -u indimail -p ssh-1.5- -d indimail -H localhost -P 3306
2. auth sufficient pam-multi.so args -c [grep "^%u:" /etc/passwd | awk -F: ’{print $2}’]
3. auth sufficient pam-multi.so args -s /usr/lib/indimail/modules/iauth.so
```
NOTE: Read the **pam**(8) man page to cconfigure your PAM

#### Config /etc/pam.d/pam-multi

```
#
# auth     sufficient  pam-multi.so argv0 -m [select pw_passwd from indimail where pw_name='%u' and pw_domain='%d'] -u indimail -p ssh-1.5- -D indimail -H localhost -P 3306 -d
# auth     sufficient  pam-multi.so argv0 -s /usr/lib/indimail/modules/iauth.so -d
# account  sufficient  pam-multi.so argv0 -s /usr/lib/indimail/modules/iauth.so -d
#
#%PAM-1.0
auth     include     system-auth
auth     sufficient  pam-multi.so pam-multi -s /usr/lib/indimail/modules/iauth.so
account  sufficient  pam-multi.so pam-multi -s /usr/lib/indimail/modules/iauth.so
```

#### Config /etc/pam.d/imap
```
#
# $Log: imap.in,v $
#
# $Id: imap.in,v 1.2 2020-09-29 10:59:22+05:30 Cprogrammer Exp mbhangui $
#
# auth     required  pam-multi.so argv0 -m [select pw_passwd from indimail where pw_name='%u' and pw_domain='%d'] -u indimail -p ssh-1.5- -D indimail -H localhost -P 3306 -d
# auth     required  pam-multi.so argv0 -s /usr/lib/indimail/modules/iauth.so
# account  required  pam-multi.so argv0 -i imap -s /usr/lib/indimail/modules/iauth.so
# add -d argument to debug pam-multi lines
#
#%PAM-1.0
auth     include     system-auth
auth     sufficient  pam-multi.so pam-multi -s /usr/lib/indimail/modules/iauth.so
account  sufficient  pam-multi.so pam-multi -i imap -s /usr/lib/indimail/modules/iauth.so
```

#### Config /etc/pam.d/pop3
```
#
# $Log: pop3.in,v $
#
# $Id: pop3.in,v 1.2 2020-09-29 10:59:22+05:30 Cprogrammer Exp mbhangui $
#
# auth     required  pam-multi.so argv0 -m [select pw_passwd from indimail where pw_name='%u' and pw_domain='%d'] -u indimail -p ssh-1.5- -D indimail -H localhost -P 3306 -d
# auth     required  pam-multi.so argv0 -s /usr/lib/indimail/modules/iauth.so
# account  required  pam-multi.so argv0 -i pop3 -s /usr/lib/indimail/modules/iauth.so
# add -d argument to debug pam-multi lines
#
#%PAM-1.0
auth     include     system-auth
auth     sufficient  pam-multi.so pam-multi -s /usr/lib/indimail/modules/iauth.so
account  sufficient  pam-multi.so pam-multi -i pop3 -s /usr/lib/indimail/modules/iauth.so
```

NOTE: Any line starting with `#` is a comment

## Examples AUTHMODULE configuration

### IndiMail courier-imap

This is a crazy example of using checkpassword compatible auth module with courier-imap (because courier-imap has its own auth modules).

```
# cat > /usr/libexec/indimail/imapmodules/authcheckpassword <<EOF
#!/bin/sh
if [ -n "$POSTAUTH" ] ; then
  exec env "AUTHENTICATED=$1" "MAILDIR=$HOME/Maildir" "AUTHADDR=$1" \
    /usr/local/bin/imapd Maildir
fi
POSTAUTH=/usr/local/libexec/indimail/imapmodules/authcheckpassword
AUTHSERVICE=$(basename $AUTHUSER | cut -c1-4)
export POSTAUTH
out=`mktemp -t authpXXXXXXXXXX`
sed -e '1,2d' 0<&3 | tr '\n' '\0' > $out
exec 3<$out
/bin/rm -f $out
exec /usr/local/sbin/pam-checkpwd -s pam-multi -i $AUTHSERVICE -- $*
EOF
# chmod +x /usr/libexec/indimail/imapmodules/authcheckpassword
```

### indimail-mta Authenticated SMTP

The following will use pam-checkpwd sys-checkpwd as the first, pam-checkpwd as the second authmodule, and vchkpass as the last module.

```
# cat > /service/qmail-smtpd.587/variables/AUTHMODULES <<EOF
/usr/sbin/sys-checkpwd pam-checkwd -s pam-multi -e -- /usr/sbin/vchkpass
EOF
```

## TESTING

You can use pam-checkpwd to test pam-multi. You can also use pamtester Written by Moriyoshi Koizumi <moriyoshi@users.sourceforge.net>. 

```
printf "user01@example.com\0pass\0\0" | pam-checkpwd -s pam-multi -e -- /usr/bin/id 3<&0
pamtester imap user01@example.com authenticate
pamtester pop3 user01@example.com authenticate
pamtester pam-multi user01@example.com authenticate
```

pam-multi is experimental at this stage. It is covered under GNU GPL V3 license and no warranty is implied. Suggestions to improve it are welcome.
