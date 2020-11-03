# iwebadmin-hooks

Note: Only the add/del/moduser and add/del/mailinglist hooks have been implemented.  Parameters to add/del/modmaillist will be different than those used for add/del/mod user.

Create a file named `.iwebadmin-hooks` according to the following syntax. iwebadmin will first look in the directory for the current domain, and then look for `iwebadmin-hooks` in /etc/indimail.


```
adduser     /path/to/hook/software
deluser     /path/to/hook/software
moduser     /path/to/hook/software
addmaillist /path/to/hook/software
delmaillist /path/to/hook/software
modmaillist /path/to/hook/software # not implemented
listadduser /path/to/hook/software # not implemented
listdeluser /path/to/hook/software # not implemented
# comments are ok using the "#" character as in this line
```

The following parameters will be passed to the specified program:

adduser/moduser:

```
$1: username
$2: domain name
$3: password
$4: GECOS (user's full name)
```

deluser:

```
$1: username
$2: domain name
$3: "forwardto" email address
$4: empty string
```

addmailinglist:

```
$1: listuser
$2: listdomain
$3: listowner or ""
$4: replyto   or ""
```

delmailinglist:

```
$1: listuser
$2: listdomain
$3: ""
$4: ""
```

You can use various environment variables in your script too. e.g.

```
HTTP_ACCEPT_ENCODING=gzip, deflate, br
SERVER_NAME=127.0.0.1
HTTP_ORIGIN=http://127.0.0.1
UNIQUE_ID=X6D5jkmdkM@0hhkILXZX1QAAAAI
SCRIPT_NAME=/cgi-bin/iwebadmin
GATEWAY_INTERFACE=CGI/1.1
SERVER_SOFTWARE=Apache/2.4.46 (Fedora) OpenSSL/1.1.1g
PATH_INFO=/com/delmailinglistnow
DOCUMENT_ROOT=/var/www/html
HTTP_UPGRADE_INSECURE_REQUESTS=1
PWD=/var/indimail/domains/example.com
REQUEST_URI=/cgi-bin/iwebadmin/com/delmailinglistnow?user=postmaster&dom=example.com&time=1604382165&
PATH_TRANSLATED=/var/www/html/com/delmailinglistnow
SERVER_SIGNATURE=
REQUEST_SCHEME=http
QUERY_STRING=user=postmaster&dom=example.com&time=1604382165&
HTTP_ACCEPT_LANGUAGE=en-US,en;q=0.9
HTTP_SEC_FETCH_DEST=document
CONTEXT_DOCUMENT_ROOT=/var/www/cgi-bin/
HTTP_ACCEPT=text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
HTTP_SEC_CH_UA="Chromium";v="86", "\"Not\\A;Brand";v="99", "Google Chrome";v="86"
REMOTE_PORT=36006
SERVER_ADMIN=root@localhost
HTTP_HOST=127.0.0.1
HTTP_SEC_FETCH_USER=?1
HTTP_SEC_FETCH_SITE=same-origin
HTTP_CONNECTION=keep-alive
SERVER_ADDR=127.0.0.1
HTTP_DNT=1
HTTP_USER_AGENT=Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.111 Safari/537.36
CONTEXT_PREFIX=/cgi-bin/
SHLVL=1
CONTENT_LENGTH=10
HTTP_SEC_CH_UA_MOBILE=?0
HTTP_SEC_FETCH_MODE=navigate
HTTP_REFERER=http://127.0.0.1/cgi-bin/iwebadmin/com/delmailinglist?user=postmaster&dom=example.com&time=1604382165&modu=test2
SERVER_PROTOCOL=HTTP/1.1
SERVER_PORT=80
SCRIPT_FILENAME=/var/www/cgi-bin/iwebadmin
REMOTE_ADDR=127.0.0.1
HTTP_CACHE_CONTROL=max-age=0
PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin
CONTENT_TYPE=application/x-www-form-urlencoded
REQUEST_METHOD=POST
```

So, this is a very nice way for the system owner to track down what is happening for billing purposes or whatever.

Author of the hooks feature is Michael Boman, former at wizoffice, Singapore.

Hooks were introduced 12 MAY 2000, with their release: qmailadmin-0.26h-WizOffice-FROZEN-12-MAY-2000.tgz
