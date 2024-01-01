<!-- # vim: wrap
-->
# What is it?

iwebadmin is a cgi program for administering indimail. This version of iwebadmin is a hack of QmailAdmin https://www.inter7.com/qmailadmin-project

Requirements

- A UNIX computer (Linux/Solaris/BSD) and C compiler (gcc)

- A web server [Apache](http://www.apache.org/)

- [indimail-virtualdomains](https://github.com/indimail/indimail-virtualdomains)

- ezmlm / ezmlm-idx installed in /usr/bin
    - http://cr.yp.to/ezmlm.html
    - http://www.ezmlm.org/
    - https://github.com/indimail/ezmlm-idx

# Updates?

Latest version is available at https://github.com/indimail/indimail-virtualdomains/tree/master/iwebadmin-x. Please refer your suggestions/bug reports to iwebadmin@indimail.org

# Install guide 

iwebadmin 1.3 and later requires indimail 1.7 or later. iwebadmin 1.4 and later requires indimail 2.x or later.

If you are installing on x86\_64 platform (64-bit Intel/AMD processor), or if configure exits with an "Invalid configuration" error, you will need to run `libtoolize --force` in the IwebAdmin source directory before following any other instructions in this guide.

iwebadmin needs root to install into directories

- /usr/share/iwebadmin - lang and html directory,
- /var/www/cg-bin      - the web server's cgi-bin directory
- /var/www/html/image  - web server's image directory.

Since iwebadmin is a suid binary, make sure it's installed on a volume that isn't mounted with the 'nosuid' option in /etc/fstab.

Be root before you follow the rest of the instructions or your installation will fail. 

If you have problems using 'make', try using 'gmake' instead.

Note to people who did not read the above paragraph: When you give up on your installation, try reading the above information then starting from scratch.

1.  fast install guide.. 

    ```
    $ ./configure --prefix=/usr --sysconfdir=/etc/indimail \
        --disable-ipauth --disable-trivial-password
        --enable-ezmlmdir=/usr/bin --enable-domain-autofill \
        --enable-modify-quota --enable-htmldir=/var/www/html
        --enable-cgibindir=/var/www/cgi-bin \
        --enable-imagedir=/var/www/html/images/iwebadmin \
        --enable-htmllibdir=/usr/share/iwebadmin
    ```
    or you can run `./default.configure` instead of the above configure command

    ```
    $ make
    $ sudo make install-strip
    ```

    If it works, you are done!
    If it doesn't.. read below.

2.  Before we can make and install there are a few things to consider..

    a. do you have a indimail user and indimail group?
    b. where is your cgi bin directory?
    c. where is your ezmlm directory? default /usr/bin/ezmlm
    d. where is your autorespond directory? /usr/bin

    If you are missing any one of the above, stop reading this now and
    go install the missing software. 

3.  Below is a list of possible configuration options 

    Note: Decide now which features you will want/need for your
    `./configure`

    *Use this if your cgi bin is not in a standard location
    --enable-cgibindir={dir}   HTTP server's cgi-bin directory.

    Since iwebadmin is a suid binary, make sure the cgi-bin directory is on a volume that isn't mounted with the 'nosuid' option in /etc/fstab.

    Use this if your don't want the HTML templates to be in /usr/share/iwebadmin

       --enable-htmllibdir={dir}  iwebadmin HTML library directory.

    Use this if your URL PATH for cgi-bin is elsewhere   

       --enable-cgipath={/cgi-bin/iwebadmin}   URL path for cgi.

    Use this to set the indimail user if it is not indimail.

       --enable-indimailuser={indimail}   user indimail was installed as.

    Use this to set the indimail group if it is not indimail.

       --enable-indimailgroup={indimail}   group indimail was installed as.
   
    Use this if ezmlm is not in /usr/bin/ezmlm

       --enable-ezmlmdir={dir}  Directory that holds the ezmlm package.

    If you have different domains accessing the same iwebadmin, you may want to automatically fill the domain field with the domain the user accessed the iwebadmin cgi with:

       --enable-domain-autofill

    With autofill enabled, iwebadmin will search the file /etc/indimail/control/virtualdomains for an entry that matches the hostname of the HTTP request.  So, if test.com appears in your virtualdomains file, <http://www.test.com/cgi-bin/iwebadmin> will pre-fill the domain field with "test.com".

    Note that with or without autofill enabled, you can pass parameters to iwebadmin to pre-fill the "User Account" and "Domain" fields.  <http://www.test.com/cgi-bin/iwebadmin?dom=xyz.net&user=john> will prefill "Domain" with xyz.net and "User Account" with john.

4.  Ok, now configure, using any options that you need to. For example:

    ```
    ./configure  --enable-cgibindir=/my/wierd/cgi-bin/dir ....
    ```

5.  make

6.  make install or 
    make install-strip (for a smaller binary)

    To run it, type into your webrowser:

    http://yourdomain/cgi-bin/iwebadmin

    Now some fine tuning

7.  Other things

- If you want to set per domain limits on the number of

	- pop accounts  
	- aliases
	- forwards
	- mailing lists 
	- autoresponders

  Then create a .iwebadmin-limits file in the virtual domain directory for the domain you wish to limit. The syntax of the .iwebadmin-limits file is as follows

    ```
    maxpopaccounts          X
    maxaliases              X
    maxforwards             X
    maxmailinglists         X
    maxautoresponders       X
    ```

   Where X is the maximum number you wish. Be sure the indimail user has read permissions to this file. The default is unlimited.
   
   If you set any of the above values to 0 it will effectually disable that part of the menu and that feature.

   In addition, you can use iwebadmin-limits to disable services on a per domain basis for the creation of new users.  If you choose to disable services on a domain that has existing user accounts, you will need to modify the existing user accounts manually with vmoduser.
    
   You may disable these services:

    - POP Access
    - IMAP Access
    - Roaming Users (External Relaying)
    - Webmail Access
    - Dialup Access
    - Password Changing

   The syntax of the .iwebadmin-limits file for disabling the above services, respectively, is:

    ```
    disable_pop
    disable_imap
    disable_external_relay
    disable_webmail
    disable_dialup
    disable_password_changing
    ```

   These services are enabled by default, unless manually changed via `vmoduser`

   Lastly, you can set default quotas on a per domain basis.  Just include this line in your .iwebadmin-limits file:

    ```
    default_quota <quota>
    ```

   The format of \<quota> is the same used for other command line tools like vadduser, vsetuserquota, and vmoduser.
   
   NOTE: the same restrictions apply as the "disable" options above: this only applies to *new* users.  Any existing users will need to me manually changed via vmoduser.

- If you want to modify the "look" of qmail admin:

   Edit the html template files in /usr/share/iwebadmin/ or if you changed the location with the `--enable-htmllibdir={dir}` then edit the files in that directory. The HTML elements for some not-very-often used features are included in comments in the template files. Remove the comment tags to see the relevant bits.

- If you want more than one administrator of a domain:

   You can issue `vmoduser -a <user@domain>` to grant iwebadmin administrator privileges to non-postmaster users for a domain. To remove the these privileges, just clear the gid flags for that user with `vmoduser -x <user@domain>`

- To log into the interface you will first need to create a domain using the vadddomain program.

    ```
    $ sudo /usr/bin/vadddomain "your new domain name" "pick a postmaster password"
    ```

    Then you can log into iwebadmin with "your new domain name" and the password you set with the vadddomain command.

- There are a number of things you can pass to iwebadmin when you run it. I believe they can be sent via either post or get. 

    domain = set the domain name in the login page.

    user = set the user name in the login page.

    returnhttp and returntext (both must be used) = create a link to returnhttp on all iwebadmin pages with returntext for its label.

- For using nginx, you can create /etc/nginx/default.d/iwebadmin.conf

    a) Install nginx, fcgiwrap

    ```
    # dnf/yum install fcgiwrap nginx
    or
    # apt-get install nginx fcgiwrap libbg1-dev ssl-cert
    ```

    b) create iwebadmin.conf

    ```
    location /iwebadmin/ {
        root /var/www/;
        # Fastcgi socket
        fastcgi_pass unix:/run/fcgiwrap/fcgiwrap-nginx.sock;
        # Fastcgi parameters, include the standard ones
        include fastcgi_params;
 
        # Adjust non standard parameters (SCRIPT_FILENAME)
        fastcgi_split_path_info (^/cgi-bin/iwebadmin[^/]*)(.*)$;
        fastcgi_param SCRIPT_FILENAME /var/www/cgi-bin/iwebadmin;
        fastcgi_param PATH_INFO $fastcgi_path_info;
        fastcgi_param HTTPS on;
    }

    location /cgi-bin/iwebadmin/ {
        root /var/www/;
        # Fastcgi socket
        fastcgi_pass unix:/run/fcgiwrap/fcgiwrap-nginx.sock;
        # Fastcgi parameters, include the standard ones
        include fastcgi_params;

        # Adjust non standard parameters (SCRIPT_FILENAME)
        fastcgi_split_path_info (^/cgi-bin/iwebadmin[^/]*)(.*)$;
        fastcgi_param SCRIPT_FILENAME /var/www/cgi-bin/iwebadmin;
        fastcgi_param PATH_INFO $fastcgi_path_info;
        fastcgi_param HTTPS on;
    }
    ```

 c) start nginx

    ```
    # systemctl enable fcgiwrap@nginx.socket
    # systemctl start  fcgiwrap@nginx.socket
    # systemctl start nginx
    ```

8 .  Enjoy

    If you have any questions or comments please email indimail-support@lists.sourceforge.net or join the [mailing list](http://groups.google.com/group/indimail)

$Id: INSTALL,v 1.8 2017-03-18 14:20:57+05:30 Cprogrammer Exp mbhangui $

# Testing

You can use curl to login, logout or do any activity. Three examples below

```
# login as normal user
curl http://localhost/cgi-bin/iwebadmin -d "username=postmaster&domain=example.com&password=pass%40%40123&returnhttp=&returntext="

# login as admin user
curl http://localhost/cgi-bin/iwebadmin -d "username=postmaster&domain=example.com&password=pass&returnhttp=&returntext="
curl -X GET "http://localhost/cgi-bin/iwebadmin/com/logout?user=testuser01&dom=example.com&time=1690551326"
```
