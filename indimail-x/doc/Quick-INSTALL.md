# Quick Install
   IndiMail supports two types of virtual domains

   1. Non-Clustered domain - A domain existing on a single server
   2. Clustered domain - A domain extended across multiple servers, with each
      server having its own set of users.
   
   You must decide if you want Non-clustered setup or a clustered setup. If you have millions
   of users, then you must chose clustered setup. Even if you chose to install a non-clustered
   setup, you can always migrate to a clustered setup later.

   IndiMail has yum / RPM / Debian repositories for most of the Linux Distros at

   http://download.opensuse.org/repositories/home:/indimail/

   The latest RPM (for yet to be released version) can be found at

   http://download.opensuse.org/repositories/home:/mbhangui/

   The corresponding install instructions for the two repositories are
   https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail
   &
   https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail

   You can also Download the RPM for either of the two above repository and proceed below

   --------------------------------------------------------------------------------------

   Creating a non-clustered Domain
   -------------------------------

   Install IndiMail from RPM / Debian. Follow instrucations at

   https://software.opensuse.org/download.html?project=home%3Aindimail&package=indimail
    or
   https://software.opensuse.org/download.html?project=home%3Ambhangui&package=indimail
    or
   % sudo rpm -ivh filename.rpm (for RPM)
    or
   % sudo dpkg -i filename.deb (for Debian)

   Create a basic configuration file required by IndiMail
   % cd /etc/indimail/control
   % su
   # echo "localhost:indimail:ssh-1.5-:/tmp/mysql.sock" > host.mysql

   Start IndiMail
   # /usr/sbin/initsvc -on
    or
   # service svscan start
    or
   # systemctl start svscan
    or
   # /etc/init.d/svscan start
   # exit

   x--------------------------------------------------------------------------------------x
   | NOTE: replace localhost, indimail, ssh-1.5-, /var/run/mysqld/mysqld.sock as per your |
   | MySQL installation. You can use 3306 instead of /var/run/mysqld/mysqld.sock in case  |
   | your MySQL database is on another host.                                              |
   | you must use host:user:password:socket or host:user:password:port format             |
   | for host.mysql (for IndiMail 1.6.9 and above).                                       |
   | If you change the socket/port then you have to edit /etc/indimailcontrol/host.*      |
   | and /etc/indimail/indimail.cnf to change the socket path / port                      |
   |                                                                                      |
   | Instead of the above initsvc command, you can also do (a more portable way)          |
   |                                                                                      |
   | % sudo service indimail start                                                        |
   x--------------------------------------------------------------------------------------x

   Ensure MySQL is running

   The RPM installation creates MySQL service in down state. The MySQL service
   will not come up unless you use the command svc -u. Also to ensure that the
   service comes up automatically after system reboot, remove the file 'down'
   Also check the MySQl configuration file /etc/indimail/indimail.cnf
   for the port and socket parameter.
    [client]
    port   = 3306
    socket = /var/run/mysqld/mysqld.sock

   % sudo /bin/rm /service/mysql.3306/down
   % sudo /usr/bin/svc -u /service/mysql.3306

   % sudo /usr/bin/svstat /service/mysql.3306
   /service/mysql.3306: up (pid 11936) 5 seconds

   Create a virtual domain and do various operations
   % sudo /usr/bin/vadddomain example.com pass
   % /usr/bin/vuserinfo postmaster@example.com
   % sudo /usr/bin/vadduser testuser1@example.com pass
   % /usr/bin/vuserinfo testuser1@example.com
   % /usr/bin/vpasswd testuser1@example.com newpass
   % /usr/bin/vmoduser -q 50000000 testuser1@example.com

   --------------------------------------------------------------------------------------

   Creating a Clustered Domain
   ---------------------------

   Install IndiMail from RPM / Debian
   % sudo rpm -ivh filename.rpm
    or
   % sudo dpkg -i filename.deb (for Debian)

   Create basic configuration files required by IndiMail
   % cd /etc/indimail/control
   % su
   # echo "localhost:indimail:ssh-1.5-:/var/run/mysqld/mysqld.sock" > host.cntrl
   # ln -s host.cntrl host.master
   # echo 192.168.1.100 > hostip (replace 192.168.1.100 with your mailserver IP)

   Start IndiMail
   # /usr/sbin/initsvc -on
    or
   # service svscan start
    or
   # systemctl start svscan
    or
   # /etc/init.d/svscan start
   # exit

   x------------------------------------------------------------------------------------x
   | NOTE: replace localhost, indimail, ssh-1.5-, /tmp/mysql.sock as relevant to your   |
   | MySQL installation. You can use 3306 instead of /tmp/mysql.sock in case your MySQL |
   | database is on another host.                                                       |
   | you must use host:user:password:socket or host:user:password:port format           |
   | for host.cntrl (for IndiMail 1.6.9 and above).                                     |
   | If you change the socket/port then you have to edit /etc/indimailcontrol/host.*    |
   | and /etc/indimail/indimail.cnf to change the socket path / port                    |
   |                                                                                    |
   x------------------------------------------------------------------------------------x

   Ensure MySQL is running

   The RPM installation creates MySQL service in down state. The service will
   not come up unless you use the command svc -u. Also to ensure that the
   service comes up automatically after system reboot, remove the file 'down'

   % sudo /bin/rm /service/mysql.3306/down
   % sudo /usr/bin/svc -u /service/mysql.3306

   % sudo /usr/bin/svstat /service/mysql.3306
   /service/mysql.3306: up (pid 11936) 7 seconds

   Create a virtual domain and do various operations
   % sudo /usr/bin/vadddomain -D indimail -S localhost \
       -U indimail -P ssh-1.5- -p 3306 -c example.com pass
   % /usr/bin/vuserinfo postmaster@example.com
   % sudo @iprefix@/bin/vadduser testuser1@example.com pass
   % /usr/bin/vuserinfo testuser1@example.com
   % sudo /usr/bin/vpasswd testuser1@example.com newpass
   % sudo /usr/bin/vmoduser -q 50000000 testuser1@example.com

   Query the cluster definition
   % /usr/bin/dbinfo -s

   --------------------------------------------------------------------------------------

   Send / Receive Mails
   --------------------

   At this stage, your setup is ready to send mails to the outside world. To receive
   mails, you need to create your actual domain (instead of example.com) using vadddomain
   and setup a mail exchanger record for your domain (MX record).
   To send mails, you can either use SMTP or use a sendmail (IndiMail's sendmail replacement
   /usr/bin/sendmail).

   % ( echo 'First M. Last'; uname -a) | \
      mail -s "IndiMail Installation" indimail-virtualdomains@indimail.org

   Replace First M. Last with your name.

   --- * --- * ---- * --- * --- THE END --- * --- * --- * --- * ---

   x------------------------------------------------------------------------------------x
   |NOTE: You can email me at indimail-virtualdomains@indimail.org, if you find this    |
   |confusing or need any help/clarification.                                           |
   x------------------------------------------------------------------------------------x