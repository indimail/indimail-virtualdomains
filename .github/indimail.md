# Installing Indimail using DNF/YUM/APT Repository

**TERMINOLOGY used for commands**

TERMINOLOGY|Description
-----------|-----------
$ command|command `command` was executed by a non-privileged user
# command|command `command` was executed by the `root` user
$ sudo command|command `command` requires root privilege to run. sudo was used to gain root privileges

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

# Using Docker Engine to Run IndiMail / IndiMail-MTA

IndiMail now has docker images. You can read about installing Docker here. Once you have installed docker-engine, you need to start it. Typically it would be

`$ sudo service docker start`

To avoid having to use sudo when you use the docker command, create a Unix group called docker and add users to it. When the docker daemon starts, it makes the ownership of the Unix socket read/writable by the docker group.
NOTE: Warning: The docker group is equivalent to the root user; For details on how this impacts security in your system, see [Docker Daemon attack surface](https://docs.docker.com/engine/security/security/#docker-daemon-attack-surface)

```
$ sudo groupadd docker 
$ sudo usermod -aG docker your\_username
```
Log out and login again to ensure your user is running with the correct permissions. You can run the unix id command to confirm that you have the docker group privileges. e.g.

```
$ id -a
uid=1000(mbhangui) gid=1000(mbhangui) groups=1000(mbhangui),10(wheel),545(docker) context=unconfined\_u:unconfined\_r:unconfined\_t:s0-s0:c0.c1023
```

Now we need to pull the docker image for IndiMail. use the **docker pull** command. The values for tag can be fedora-23, centos7, debian8, ubuntu-15.10, ubuntu-14.03. If your favourite OS is missing, let me know. You can find the list of all images here.

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
# /service/fetchmail: down 32 seconds
# /service/greylist.1999: up (pid 203) 32 seconds
# /service/indisrvr.4000: up (pid 178) 32 seconds
# /service/inlookup.infifo: up (pid 192) 32 seconds
# /service/mysql.3306: up (pid 181) 32 seconds
# /service/proxy-imapd.4143: up (pid 191) 32 seconds
# /service/proxy-imapd-ssl.9143: up (pid 188) 32 seconds
# /service/proxy-pop3d.4110: up (pid 197) 32 seconds
# /service/proxy-pop3d-ssl.9110: up (pid 179) 32 seconds
# /service/pwdlookup: up (pid 195) 32 seconds
# /service/qmail-imapd.143: up (pid 222) 32 seconds
# /service/qmail-imapd-ssl.993: up (pid 200) 32 seconds
# /service/qmail-pop3d.110: up (pid 212) 32 seconds
# /service/qmail-pop3d-ssl.995: up (pid 184) 32 seconds
# /service/qmail-poppass.106: up (pid 216) 32 seconds
# /service/qmail-qmqpd.628: down 32 seconds
# /service/qmail-qmtpd.209: up (pid 153) 32 seconds
# /service/qmail-send.25: up (pid 182) 32 seconds
# /service/qmail-smtpd.25: up (pid 187) 32 seconds
# /service/qmail-smtpd.366: up (pid 208) 32 seconds
# /service/qmail-smtpd.465: up (pid 194) 32 seconds
# /service/qmail-smtpd.587: up (pid 196) 32 seconds
# /service/qmail-spamlog: up (pid 221) 32 seconds
# /service/qscanq: up (pid 213) 32 seconds
# /service/udplogger.3000: up (pid 211) 32 seconds
```

You now have a fully functional mail server with a pre-configured virtual domain indimail.org and a pre-configured virtual user testuser01@indimail.org. You can use IMAP/POP3/SMTP to your heart's content. If not satisfied, try out the ssl enabled services IMAPS/POP3S/SMTPS or STARTTLS command. If still not satisfied, read the man pages in /var/indimail/man/\*. You can stop the container by executing the docker stop command.

`$ docker stop fd09c7ca75be`

You can make your changes to the container and commit changes by using the docker commit command. Learning how to use docker is not difficult. Just follow the Docker Documentation. If you are lazy like me, just read the Getting Started guide.
I am also a newbie as far as docker is concerned. Do let me know your experience with network settings and other advanced docker topics, that you may be familiar with. Do send few bottles of beer my way if you can.

NOTE: There are few defaults for the indimail docker container image

* root password is passxxx@xxx 
* mysql user, password for indimail is indimail, ssh-1.5- 
* mysql privileged user, password is mysql, 4-57343- 
* password for postmaster@indimail.org virtual imap/pop3 account is passxxx 
* password for testuser01@indimail.org virtual imap/pop3 account is passxxx 

# IndiMail Queue Mechanism

IndiMail has multiple queues and the queue destination directories are also configurable. You can have one IndiMail installation cater to multiple instances having different properties / configuration.  To set up a new IndiMail instance requires you to just set few environment variables. Unlike qmail/netqmail, IndiMail doesn't force you to recompile each time you require a new instance.  Multiple queues eliminates what is known as ['the silly qmail syndrome'](https://qmail.jms1.net/silly-qmail.shtml "silly-qmail-syndrome") and gives IndiMail the capability to perform better than a stock qmail installation. IndiMail's multiple queue architecture gives allows it to achieve tremendous inject rates using commodity hardware as can be read [here](http://groups.google.co.in/group/indimail/browse_thread/thread/f9e0b6214d88ca6d#). Here is a pictorial representation of the IndiMail queue ![Pictorial](indimail_queue.png).

When you have massive injecting rates, your software may place multiple files in a single directory. This drastically reduces file system performance. IndiMail avoids this by injecting your email in a queue consisting of multiple directories and mails distributed as evenly as possible across these directories.

Balancing of emails across multiple queues is achieved by the program qmail-multi(8), which is actually just a qmail-queue(8) replacement. Any qmail-queue frontend can use qmail-multi. The list of qmail-queue frontends in IndiMail are

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

The diagram below shows how qmail-multi(8) works ![qmail-multi](qmail_multi.png)

You just need to configure the following environment variables to have the qmail-queue(8) frontends using qmail-multi(8)

1. QUEUE_BASE – Base directory where all queues will be placed
2. QUEUE_COUNT – number of queues
3. QUEUE_START – numeric prefix of the first queue

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

# Using systemd to start IndiMail

[systemd](http://en.wikipedia.org/wiki/Systemd) is a system and service manager for Linux, compatible with SysV and LSB init scripts. systemd provides aggressive parallelization capabilities, uses socket and D-Bus activation for starting services, offers on-demand starting of daemons, keeps track of processes using Linux cgroups, supports snapshots and restoring of the system state, maintains mount and automount points and implements an elaborate transactional dependency-based service control logic. It can work as a drop-in replacement for sysvinit.
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

From Fedora 15 onwards, upstart has been replaced by a service called systemd. Due to improper rpm package upgrade scripts, some system services previously enabled in Fedora 14, may not be enabled after upgrading to Fedora 15. To determine if a service is impacted, run the systemctl status command as shown below.

```
# systemctl is-enabled svscan.service && echo "Enabled on boot" || echo "Disabled on boot"
```

To enable indimail service on boot, run the following systemctl command

`# systemctl enable svscan.service`

Now to start IndiMail you can use the usual service command

```
# service indimail start    (to start indimail)
# service indimail stop     (to stop indimail)
```

You can automate the above service creation for systemd by running the initsvc(1) command

```
# /usr/sbin/initsvc -on  (to enable indimail service)
# /usr/sbin/initsvc -off   (to disable indimail service)
```

You can now also query the status of the running IndiMail service by using the systemctl command

```
# systemctl status indimail.service
● svscan.service - IndiMail Messaging Platform
     Loaded: loaded (/usr/lib/systemd/system/svscan.service; enabled; vendor preset: disabled)
     Active: active (running) since Wed 2020-06-10 07:15:06 IST; 3h 36min ago
   Main PID: 1354 (svscan)
      Tasks: 215 (limit: 9342)
     Memory: 766.5M
     CGroup: /system.slice/svscan.service
             ├─ 1354 /usr/sbin/svscan /service
             ├─ 1446 supervise log
             ├─ 1448 supervise qmail-qmqpd.628
             ├─ 1449 supervise log
             ├─ 1450 supervise qmail-imapd.143
             ├─ 1451 supervise log
             ├─ 1452 supervise qmail-poppass.106
             ├─ 1453 supervise log
             ├─ 1454 supervise qmail-daned.1998
             ├─ 1455 supervise log
             ├─ 1456 supervise qmail-imapd-ssl.993
             ├─ 1457 supervise log
             ├─ 1458 supervise qmail-smtpd.25
             ├─ 1459 supervise log
             ├─ 1460 supervise indisrvr.4000
             ├─ 1461 supervise log
             ├─ 1462 supervise qmail-pop3d.110
             ├─ 1463 supervise log
             ├─ 1464 supervise qmail-smtpd.366
             ├─ 1465 supervise log
             ├─ 1466 supervise freshclam
             ├─ 1467 supervise log
             ├─ 1468 supervise qmail-pop3d-ssl.995
             ├─ 1469 supervise log
             ├─ 1470 supervise qmail-qmtpd.209
             ├─ 1471 supervise log
             ├─ 1472 supervise mrtg
             ├─ 1473 supervise log
             ├─ 1476 supervise qmail-logfifo
             ├─ 1478 supervise log
             ├─ 1479 supervise qscanq
             ├─ 1480 supervise log
             ├─ 1481 supervise mpdwatch
             ├─ 1482 supervise log
             ├─ 1483 supervise qmail-send.25
             ├─ 1484 supervise log
             ├─ 1485 supervise greylist.1999
             ├─ 1486 supervise log
             ├─ 1487 supervise qmail-smtpd.465
             ├─ 1488 supervise log
             ├─ 1489 supervise proxy-imapd.4143
             ├─ 1490 supervise log
             ├─ 1491 supervise fetchmail
             ├─ 1492 supervise log
             ├─ 1493 supervise qmail-smtpd.587
             ├─ 1494 supervise log
             ├─ 1495 supervise pwdlookup
             ├─ 1496 supervise log
             ├─ 1497 supervise proxy-imapd-ssl.9143
             ├─ 1498 supervise log
             ├─ 1499 supervise dietpi
             ├─ 1500 supervise log
             ├─ 1501 supervise libwatch
             ├─ 1502 supervise log
             ├─ 1503 supervise dnscache
             ├─ 1504 supervise log
             ├─ 1505 supervise proxy-pop3d-ssl.9110
             ├─ 1506 supervise log
             ├─ 1507 supervise inlookup.infifo
             ├─ 1508 supervise log
             ├─ 1509 supervise clamd
             ├─ 1510 supervise log
             ├─ 1511 supervise proxy-pop3d.4110
             ├─ 1512 supervise log
             ├─ 1513 supervise mysql.3306
             ├─ 1514 supervise log
             ├─ 1515 supervise udplogger.3000
             ├─ 1516 supervise log
             ├─ 1517 /usr/sbin/multilog t /var/log/svc/indisrvr.4000
             ├─ 1518 /usr/sbin/multilog t /var/log/svc/smtpd.25
             ├─ 1519 /usr/sbin/multilog t /var/log/svc/svscan
             ├─ 1520 /usr/sbin/mysqld --defaults-file=/etc/indimail/indimail.cnf --port=3307 --basedir=/usr --datadir=/var/indimail/mysqldb/data --memlock --ssl --require-secure-transport --skip-external-locking --delay-key-write=all --skip-name-resolve --sql-mode=NO_ENGINE_SUBSTITUTION,NO_ZERO_IN_DATE,ERROR_FOR_DIVISION_BY_ZERO,STRICT_TRANS_TABLES --explicit-defaults-for-timestamp=TRUE --general-log=1 --general-log-file=/var/indimail/mysqldb/logs/general-log --slow-query-log=1 --slow-query-log-file=/var/indimail/mysqldb/logs/slowquery-log --log-queries-not-using-indexes --log-error-verbosity=3 --pid-file=/var/run/mysqld/mysqld.3306.pid
             ├─ 1521 /usr/sbin/multilog t /var/log/svc/proxyPOP3.4110
             ├─ 1522 /usr/sbin/multilog t /var/log/svc/mrtg
             ├─ 1523 /usr/bin/tcpserver -v -c /service/proxy-pop3d.4110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 4110 /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 1524 /usr/sbin/qmail-daned -w /etc/indimail/control/tlsa.white -t 30 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/tlsa.context
             ├─ 1525 /usr/sbin/multilog t /var/log/svc/freshclam
             ├─ 1526 /usr/bin/tcpserver -v -c /service/qmail-imapd-ssl.993/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 993 /usr/bin/couriertls -server -tcpd /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authshadow /usr/bin/imapd Maildir
             ├─ 1527 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp/tcp.smtp.cdb -c /service/qmail-smtpd.366/variables/MAXDAEMONS -o -b 150 -u 555 -g 555 0 366 /usr/sbin/qmail-smtpd
             ├─ 1528 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp/tcp.poppass.cdb -X -c /service/qmail-poppass.106/variables/MAXDAEMONS -C 25 -o -b 40 -n /etc/indimail/certs/servercert.pem -u 555 -g 555 0 106 /usr/sbin/qmail-poppass argos.indimail.org /usr/sbin/vchkpass /bin/false
             ├─ 1529 /usr/sbin/multilog t /var/log/svc/pop3d.110
             ├─ 1530 /usr/sbin/multilog t /var/log/svc/smtpd.366
             ├─ 1532 /usr/bin/qmail-cat /run/indimail/logfifo
             ├─ 1533 /usr/bin/tcpserver -v -c /service/qmail-pop3d-ssl.995/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 995 /usr/bin/couriertls -server -tcpd /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authshadow /usr/bin/pop3d Maildir
             ├─ 1534 /usr/sbin/indisrvr -i 0 -p 4000 -b 40 -n /etc/indimail/certs/servercert.pem
             ├─ 1535 /usr/sbin/multilog t /var/log/svc/logfifo
             ├─ 1549 /usr/sbin/multilog t /var/log/svc/qmqpd.628
             ├─ 1551 /usr/sbin/multilog t /var/log/svc/dietpi
             ├─ 1565 /usr/bin/mpdwatch -n
             ├─ 1578 /usr/sbin/multilog t /var/log/svc/imapd.143
             ├─ 1579 /usr/sbin/multilog t /var/log/svc/proxyIMAP.4143
             ├─ 1581 /usr/sbin/multilog t /var/log/svc/poppass.106
             ├─ 1582 /usr/bin/freshclam -v --stdout --datadir=/var/indimail/clamd -d -c 2 --config-file=/etc/freshclam.conf
             ├─ 1583 /usr/sbin/multilog t /var/log/svc/imapd-ssl.993
             ├─ 1584 /usr/sbin/multilog t /var/log/svc/mpdwatch
             ├─ 1585 /usr/bin/tcpserver -v -h -R -l 0 -x /etc/indimail/tcp/tcp.smtp.cdb -c /service/qmail-smtpd.465/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 465 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so argos.indimail.org /bin/false
             ├─ 1586 /usr/bin/tcpserver -v -c /service/qmail-pop3d.110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 110 /usr/sbin/pop3login /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authshadow /usr/bin/pop3d Maildir
             ├─ 1587 /usr/sbin/multilog t /var/log/svc/udplogger.3000
             ├─ 1588 multilog t ./main
             ├─ 1589 /usr/sbin/multilog t /var/log/svc/daned.1998
             ├─ 1590 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp/tcp.smtp.cdb -c /service/qmail-smtpd.587/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 587 /usr/lib/indimail/plugins/qmail_smtpd.so
             ├─ 1611 /usr/sbin/multilog t /var/log/svc/qscanq
             ├─ 1612 /usr/sbin/multilog t /var/log/svc/pop3d-ssl.995
             ├─ 1614 /usr/sbin/multilog t /var/log/svc/smtpd.465
             ├─ 1615 /usr/sbin/udplogger -p 3000 -t 10 0
             ├─ 1616 /usr/sbin/multilog t /var/log/svc/smtpd.587
             ├─ 1617 /usr/bin/tcpserver -v -c /service/proxy-imapd.4143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 4143 /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 1618 /usr/sbin/multilog t /var/log/svc/greylist.1999
             ├─ 1619 /usr/sbin/multilog t /var/log/svc/inlookup.infifo
             ├─ 1621 /usr/bin/tcpserver -v -h -R -l 0 -x /etc/indimail/tcp/tcp.smtp.cdb -c /service/qmail-smtpd.25/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 25 /usr/lib/indimail/plugins/rblsmtpd.so -rdnsbl-1.uceprotect.net -rzen.spamhaus.org /usr/lib/indimail/plugins/qmail_smtpd.so
             ├─ 1622 /usr/sbin/cleanq -l -s 200 /var/indimail/qscanq/root/scanq
             ├─ 1623 /usr/bin/dnscache
             ├─ 1624 /usr/sbin/qmail-daemon ./Maildir/
             ├─ 1625 /usr/sbin/multilog t /var/log/svc/proxyPOP3.9110
             ├─ 1626 /usr/sbin/multilog t /var/log/svc/pwdlookup
             ├─ 1627 /usr/sbin/multilog t /var/log/svc/proxyIMAP.9143
             ├─ 1628 /usr/sbin/qmail-greyd -w /etc/indimail/control/greylist.white -t 30 -g 24 -m 2 -s 5 -h 65535 127.0.0.1 /etc/indimail/control/greylist.context
             ├─ 1629 /usr/sbin/multilog t /var/log/svc/deliver.25
             ├─ 1630 /usr/bin/tcpserver -v -H -R -l 0 -x /etc/indimail/tcp/tcp.qmtp.cdb -c /service/qmail-qmtpd.209/variables/MAXDAEMONS -o -b 75 -u 555 -g 555 0 209 /usr/sbin/qmail-qmtpd
             ├─ 1631 /usr/bin/tcpserver -v -c /service/qmail-imapd.143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 143 /usr/sbin/imaplogin /usr/libexec/indimail/imapmodules/authindi /usr/libexec/indimail/imapmodules/authshadow /usr/bin/imapd Maildir
             ├─ 1632 /usr/bin/tcpserver -v -c /service/proxy-imapd-ssl.9143/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.imap.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 9143 /usr/bin/couriertls -server -tcpd /usr/bin/proxyimap /usr/bin/imapd Maildir
             ├─ 1633 /usr/sbin/nssd -d notice
             ├─ 1634 /bin/sh ./run
             ├─ 1635 /usr/sbin/multilog t /var/log/svc/qmtpd.209
             ├─ 1636 /usr/sbin/multilog t /var/log/svc/mysql.3306
             ├─ 1637 /usr/sbin/multilog t /var/log/svc/clamd
             ├─ 1640 /usr/bin/tcpserver -v -c /service/proxy-pop3d-ssl.9110/variables/MAXDAEMONS -C 25 -x /etc/indimail/tcp/tcp.pop3.cdb -X -o -b 40 -H -l 0 -R -u 555 -g 555 0 9110 /usr/bin/couriertls -server -tcpd /usr/bin/proxypop3 /usr/bin/pop3d Maildir
             ├─ 1659 /usr/sbin/multilog t /var/log/svc/fetchmail
             ├─ 1673 /usr/sbin/multilog t /var/log/svc/libwatch
             ├─ 1689 qmail-send
             ├─ 1691 qmail-send
             ├─ 1692 qmail-lspawn ./Maildir/
             ├─ 1693 qmail-rspawn
             ├─ 1694 qmail-clean
             ├─ 1695 qmail-todo
             ├─ 1696 qmail-clean
             ├─ 1697 qmail-lspawn ./Maildir/
             ├─ 1698 qmail-rspawn
             ├─ 1699 qmail-clean
             ├─ 1700 qmail-todo
             ├─ 1701 qmail-clean
             ├─ 1702 qmail-send
             ├─ 1703 qmail-lspawn ./Maildir/
             ├─ 1704 qmail-rspawn
             ├─ 1705 qmail-clean
             ├─ 1706 qmail-todo
             ├─ 1707 qmail-clean
             ├─ 1708 qmail-send
             ├─ 1709 qmail-lspawn ./Maildir/
             ├─ 1710 qmail-rspawn
             ├─ 1711 qmail-clean
             ├─ 1712 qmail-todo
             ├─ 1713 qmail-clean
             ├─ 1714 qmail-send
             ├─ 1715 qmail-lspawn ./Maildir/
             ├─ 1716 qmail-rspawn
             ├─ 1717 qmail-clean
             ├─ 1718 qmail-todo
             ├─ 1719 qmail-clean
             ├─ 2387 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2389 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2390 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2391 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2392 /usr/sbin/inlookup -i 5 -c 5184000
             ├─ 2393 /usr/sbin/inlookup -i 5 -c 5184000
             ├─59137 sleep 60
             └─59139 sleep 300

Jun 10 07:15:06 argos.indimail.org systemd[1]: Started IndiMail Messaging Platform.
```

# Eliminating Duplicate Emails during local delivery

Often you will find program like MS outlook, notorious for sending duplicate emails, flooding your inbox. IndiMail allows you to quickly deal with this proprietary nonsense by turning on duplicate eliminator in **vdelivermail**(8) - the default MDA. To turn on the duplicate eliminator in vdelivermail, you need to set **ELIMINATE_DUPS** and **MAKE_SEEKABLE** environment variables.

```
$ sudo /bin/bash
# echo 1>/service/qmail-send.25/variables/ELIMINATE_DUPS
# echo 1>/service/qmail-send.25/variables/MAKE_SEEKABLE
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
```

If you do not use vdelivermail and want to use your own delivery agent? Fear not by using ismaildup(1). ismaildup expects the email on standard input and is easily scriptable like the example below in a .qmail file.

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

You can replace maildirdeliver in the last line with vdelivermail(8)

`| /usr/sbin/vdelivermail '' bounce-no-mailbox`

# Writing Filters for IndiMail

IndiMail provides multiple methods by which you can intercept an email in transit and modify the email headers or the email body. A filter is a simple program that expects the raw email on standard input and outputs the message text back on standard output. The program /bin/cat can be used as a filter which simply copies the standard input to standard output without modifying anything. Some methods can be used before the mail gets queued and some methods can be used before the execution of local / remote delivery.

It is not necessary for a filter to modify the email. You can have a filter just to extract the headers or body and use that information for some purpose. IndiMail also provides the following programs - 822addr(1), 822headerfilter(1), 822bodyfilter(1), 822field(1), 822fields(1), 822header(1), 822body(1), 822headerok(1), 822received(1), 822date(1), 822fields(1) to help in processing emails.

Let us say that we have written a script /usr/local/bin/myfilter. The myfilter program expects the raw email on stdin and outputs the email back (maybe modiying it) on stdout.

## 1.1 Filtering during SMTP (before mail gets queued)

### 1.1.1 Using FILTERARGS environment variable

The below configuration causes all inbound SMTP email to be fed through the filter /usr/local/bin/myfilter. You can use the programs 822header(1), 822body(1) inside myfilter to get and manipulate the headers and body (See 1.5.1).

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-smtpd.25/variables/FILTERARGS
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25

NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).
```

### 1.1.2 Using QMAILQUEUE with qmail-qfilter

You can use qmail-qfilter(1). qmail-qfilter allows you to run multiple filters passed as command line arguments to qmail-qfilter. Since QMAILQUEUE doesn't allow you to specify multiple arguments, you can write a shell script which calls qmail-qfilter and have the shell script defined as QMAILQUEUE environment variable.

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

### 1.1.3 Using QMAILQUEUE with your own program

When you want to use your own program as QMAILQUEUE, then your program is responsible for queuing the email. It is trivial to queue the email by calling qmail-multi(8). You script can read the stdin for the raw message (headers + body) and pipe the output (maybe after modifications) to qmail-multi(8). If you are doing DK/DKIM signing, you can execute qmail-dk(8) instead of qmail-multi(8). You can have qmail-dk(8) call qmail-dkim(8) and qmail-dkim(8) calls qmail-multi(8). Assuming you want to do DK/DKIM signing, and myfilter calls qmail-dk(8), you can do the following

```
$ sudo /bin/bash
# echo /usr/local/bin/myfilter > /service/qmail-smtpd.25/variables/QMAILQUEUE
# echo /usr/bin/qmail-dkim > /service/qmail-smtpd.25/variables/DKQUEUE
# echo /usr/bin/qmail-multi > /service/qmail-smtpd.25/variables/DKIMQUEUE
# svc -d /service/qmail-smtpd.25
# svc -u /service/qmail-smtpd.25
```

NOTE: You can set the environment variable NULLQUEUE before calling qmail-multi to discard the mail completely (blackhole).

## 1.2 Filtering during local / remote delivery

### 1.2.1 Using FILTERARGS environment variable

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

### 1.2.2 Using control file filterargs

The control file filterargs gives you control to run filters individually for local or remote deliveries. It also allows you to run your filter for both local and remote deliveries. See spawn-filter(8) for full description on this control file. 
e.g. The following entry in /var/indimail/control/filterargs causes all mails to yahoo.com be fed through the filter dk-filter(8) for DK/DKIM signing.
yahoo.com:remote:/usr/bin/dk-filter
NOTE: If the program myfilter returns 100, the message will be bounced. If it returns 2, the message will be discarded (blackholed).

### 1.2.3 Using QMAILLOCAL or QMAILREMOTE environment variables

If you define QMAILLOCAL, indimail will execute the program/script defined by the QMAILLOCAL variable for all local deliveries. The arguments passed to this program/script will be the same as that for qmail-local(8).
Similarly, if you define QMAILREMOTE, indimail will execute the program/script defined by the QMAILREMOTE variable for all remote deliveries. The argument passed to this program/script are the same as that for qmail-remote(8).
The raw email (header + body) is available on stdin. You can use 822header(8), 822body(8) for getting the headers and body. After your program is through with filtering, the output should be piped to qmail-local(8) for local deliveries and qmail-remote(8) for remote deliveries. You need to also call qmail-local / qmail-remote with the same arguments. i.e

```
exec qmail-local  "$@"     #(for local deliveries)
exec qmail-remote "$@"     #(for remote deliveries)
```

NOTE: You can exit with value 0 instead of calling qmail-local / qmail-remote to discard the mail completely (blackhole)

## 1.3 Using dot-qmail(5) or valias(1)

Both .qmail files and valias mechanism allows you to execute your own programs for local deliveries. See the man pages for dot-qmail(5) and valias(1) for more details. After manipulating the original raw email on stdin, you can pipe the out to the program maildirdeliver(1) for the final delivery.
Assuming you write the program myscript to call maildirdeliver program, you can use the valias command to add the following alias

`$ valias -i "|/usr/local/bin/myfilter" testuser01@example.com`

Now any mail sent to testuser01@example.com will be given to the program /usr/local/bin/myfilter as standard input.
NOTE: you can exit with value 0 instead of calling the maildirdeliver program to discard the mail completely (blackhole).

## 1.4 Using IndiMail rule based filter - vfilter

IndiMail's vfilter(8) mechanism allows you to create rule based filter based on any keyword in the message headers or message body. You can create a vfilter by calling the vcfilter(1) program.

```
$ vcfilter -i -t myfilter -h 2 -c 0 -k "failure notice" -f /NoDeliver -b "2|/usr/local/bin/myfilter" testuser01@example.com
```

NOTE: you can exit with value 0 instead of putting anything on standard output to discard the mail completely (blackhole).

## 1.5 Examples Filters

e.g. the below filter looks for emails having "failure notice" in the subject line and inserts the line "sorry about that" in the first line of the message body and puts the line “sent by IndiMail Messaging platform” in the last line

### 1.5.1 FILTERARGS script

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

### 1.5.2 QMAILQUEUE script

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

### 1.5.3 QMAILREMOTE script

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

### 1.5.4 QMAILLOCAL script

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

Any email that needs to be delivered needs to be put into a queue before it can be taken up for delivery. Email can be submitted to the queue using qmail-queue command or qmail_open() function. The following programs use the qmail_open() API -
condredirect, dot-forward, fastforward, filterto, forward, maildirserial, new-inject, ofmipd, qmail-inject, qmail-local, qmail-qmqpd, qmail-qmtpd, qmail-queue, qmail-send, qreceipt, replier, rrforward, qmail-smtpd.
Of these, qmail-smtpd and qmail-qmtpd accept an email for a domain only if the domain is listed in rcpthosts. Once an email is accepted into the queue, qmail-send(8) decides if the mail is to be delivered locally or to a remote address. If the email address corresponds to a domain listed in locals or virtualdomains control file, steps are taken to have the email delivered locally.

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
IndiMail further uses qmail-users mechanism to deliver the email for users in a virtual domain. This is explained  below

## qmail-users

The file /etc/indimail/users/assign assigns addresses to users.
A simple assignment is a line of the form

`=local:user:uid:gid:homedir:dash:ext:`

Here local is an address; user, uid, and gid are the account name, uid, and gid of the user in charge of local; and messages to local will be controlled by

`homedir/.qmaildashext`

If there are several assignments for the same local address, qmail-lspawn will use the first one. local is interpreted without regard to case.

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

As stated earlier,  any email addressed to user@example.com will be delivered to local user example.com-user@example.com because of virtualdomains control file. The above address can be looked as

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
In the following description, qmail-local is handling a message addressed to local@domain, where local is controlled by .qmail-ext. Here is what it does.
If .qmail-ext is completely empty, qmail-local follows the defaultdelivery instructions set by your system administrator.
If .qmail-ext doesn't exist, qmail-local will try some default .qmail files. For example, if ext is foo-bar, qmail-local will try first .qmail-foo-bar, then .qmail-foo-default, and finally .qmail-default. If none of these exist, qmail-local will bounce the message. (Exception: for the basic user address, qmail-local treats a nonexistent .qmail the same as an empty .qmail.)
The vadddomain command creates the file .qmail-default in /var/domain/domains/domain\_name. Hence any email addressed to user@example.com gets controlled by /var/indimail/domains/example.com/.qmail-default.

WARNING: For security, qmail-local replaces any dots in ext with colons before checking .qmail-ext. For convenience, qmail-local converts any uppercase letters in ext to lowercase.

# Distributing your outgoing mails from Multiple IP addresses

Some mail providers like hotmail, yahoo restrict the number of connections from a single IP and the number of mails that can be delivered in an hour from a single IP. To increase your ability to deliver large number of genuine emails from your users to such sites, you may want to send out mails from multiple IP addresses.

IndiMail has the ability to call a custom program instead of qmail-local(8) or qmail-remote(8). This is done by defining the environment variable QMAILLOCAL or QMAILREMOTE. qmail-remote(8) can use the environment variable OUTGOINGIP to set the IP address of the local interface when making outgoing connections. By writing a simple script and setting QMAILREMOTE environment variable pointing to this script, one can randomly chose an IP address from the control file

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

IndiMail allows a mechanism by which you can use your own script/program to handle bounces. All bounces in IndiMail is generated by qmail-send. qmail-send generates a bounce when qmail-lspawn or qmail-rspawn reports a permanent failed delivery. A bounce is generated by qmail-send by injecting a new mail in the queue using qmail-queue. This bounce generation by qmail-send can be modified in three ways

## 1. Using environment variable BOUNCEPROCESSOR

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

## 2 Using environment variable BOUNCERULES or control files bounce.envrules.

Using envrules, you can set specific environment variables only for bounced recipients. The format of this file is of the form

`pat:envar1=val,envar2=val,...]`

where pat is a regular expression which matches a bounce recipient.  envar1, envar2 are list of environment variables to be set. If var is omitted, the environment variable is unset.

e.g.

```
support@indimail.org:CONTROLDIR=control2,QMAILQUEUE=/usr/bin/qmail-nullqueue
```

causes all bounces generated for the sender support@indimail.org to be discarded.

## 3. Using BOUNCEQUEUE environment variable to queue bounces

qmail-send uses qmail-queue to queue bounces and aliases/forwards. This can be changed by using QMAILQUEUE environment variable. If a different queue program is desired for bounces, it can be set by using BOUNCEQUEUE environment variable.
e.g

```
$ sudo /bin/bash
# echo /usr/bin/qmail-nullqueue > /service/qmail-send.25/variables/BOUNCEQUEUE
# svc -d /service/qmail-send.25
# svc -u /service/qmail-send.25
```

disables bounces system-wide. Though disabling bounces may not be the right thing to do but in some situations where bounces are not at all needed, disabling bounces will surely result in performance improvements of your system, especially so if your system does mass-mailing.

# Delivery Instructions for a virtual domain

IndiMail uses a modified version of qmail as the MTA. For local deliveries, qmail-lspawn reads a series of local delivery commands from descriptor 0, invokes qmail-local to perform the deliveries. qmail-local reads a mail message and delivers to to a user by the procedure described in dot-qmail(5). IndiMail uses vdelivermail as the local delivery agent.
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

Just like filterargs control file, the environment variable FILTERARGS allows you to set any custom filter before your mail gets deposited into the queue by qmail-queue(8).

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

## 1. using environment variable EXTRAQUEUE

If EXTRAQUEUE environment variable is set to any environment variable, qmail-queue will deposit an extra copy of the email which it receives for putting it in the queue. Normally you would set EXTRAQUEUE variable in any of the clients which use qmail-queue. e.g. qmail-smtpd, qmail-inject, sendmail, etc. If you have setup IndiMail as per the official instructions, you can set EXTRAQUEUE for incoming and outgoing mails as given below

```
$ sudo /bin/bash
# echo "archive@example.com" > /service/qmail-smtpd.25/variables/EXTRAQUEUE
# svc -d /service/qmail-smtpd.25 /service/qmail-smtpd.587
# svc -u /service/qmail-smtpd.25 /service/qmail-smtpd.587
```

Now all your emails coming in and going out of the system, a copy will be sent to archive@example.com. If archive@example.com lies on IndiMail Messaging Platform, you can set filters (using vfilter(1)) to automatically deposit the mails in different folders. The folders can be decided on various criteria like date, sender, recipient, domain, etc.

## 2. using control file mailarchive

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

IndiMail allows you to configure most of its functionality through set of environment variables. In fact there more more than 200 features that can be controlled just by setting or un-setting environment variables. envrules is applicable to qmail-smtpd, qmail-inject, qmail-local, qmail-remote as well. It can also be used to control programs called by the above programs (e.g qmail-queue). IndiMail allows you to configure quite many things using environment variables. Just set the environment variable CONTROLDIR=control2 and all qmail components of IndiMail start looking for control files in /var/indimail/control2. You can set CONTROLDIR=/etc/indimail and all control files can be conveniently placed in /etc/indimail.
Some of these environment variables can be set during the startup of various services. IndiMail has all its services configured as directories in the /service directory. As an example, if you want to force authenticated SMTP on all your users, setting the environment variable REQUIREAUTH allows you to do so.

```
$ sudo /bin/bash
# echo 1 > /service/qmail-smtpd.587/variables/REQUIREAUTH
# svc -d /service/qmail-smtpd.587
# svc -u /service/qmail-smtpd.587
```

sets the qmail-smtpd running on port 587 to force authentication.

Setting environment variables in your startup script, in your .profile or your shell forces you to permanently set the environment variable to a specific value. Using **envrules**, IndiMail allows you to set these environment variables specific to different *senders* or *recipients* envrules allows IndiMail platform to be tuned differently for different users. No other messaging platform, to the best of my knowledge, is capable of doing that. Another way of saying is that envrules allows your IndiMail platform to dynamically change its behavior for each and every user.

For the SMTP service, you can set different environment variables for different *senders*. All that is required is to define the following in the control file /etc/indimail/control/from.envrules. The format of this file is of the form

`pattern:envar1=val,envar2=val,...]`

where pattern is a regular expression which matches a sender. envar1, envar2 are list of environment variables to be set. If val is omitted, the environment variable is unset. The name of the control file can be overridden by the environment variable **FROMRULES**. e.g. having the following in from.envrules

`*consultant:REQUIREAUTH=1,NORELAY=1`

forces all users whose email ids end with 'consultant' to authenticate while sending mails. Also such users will be prevented from sending mails to outside your domain.

`ceo@example.com:DATASIZE=`

Removes all message size restrictions for the user whose email address is ceo@example.com, by unsetting the environment variable **DATASIZE**.

You can also set envrules on per recipient basis. This gets set for qmail-local & qmail-remote. The control file to be used in this case is /etc/indimail/control/rcpt.envrules. The filename can be overridden by **RCPTRULES** environment variable.

.e.g

`*.yahoo.com:OUTGOINGIP=192.168.2.100`

The **OUTGOINGIP** environment variable is used by qmail-remote to bind on a specific IP address when connecting to the remote SMTP server. The above envrule forces qmail-remote to use 192.168.2.100 as the outgoing IP address when sending mails to any recipient at yahoo.com.

For SMTP service the following the following list of environment variables can be modified using envrules

```
REQUIREAUTH, QREGEX, ENFORCE_FQDN_HELO, DATABYTES, BADHELOCHECK, BADHELO, BADHOST, BADHOSTCHECK, TCPPARANOID, NODNSCHECK, VIRUSCHECK, VIRUSFORWARD, REMOVEHEADERS, ENVHEADERS, LOGHEADERS, LOGHEADERFD, SIGNATURES, BODYCHECK, BADMAILFROM, BADMAILFROMPATTERNS, BOUNCEMAIL, CUGMAIL, MASQUERADE, BADRCPTTO, BADRCPTPATTERNS, GOODRCPTTO, GOODRCPTPATTERNS, GREYIP, GREETDELAY, CLIENTCA, TLSCIPHERS, SERVERCERT, BLACKHOLERCPT, BLACKHOLERCPTPATTERNS, SIGNKEY, SIGNKEYSTALE, SPFBEHAVIOR, TMPDIR, TARPITCOUNT, TARPITDELAY, MAXRECIPIENTS, MAX_RCPT_ERRCOUNT, AUTH_ALL, CHECKRELAY, CONTROLDIR, ANTISPOOFING, CHECKRECIPIENT, SPAMFILTER, LOGFILTER, SPAMFILTERARGS, SPAMEXITCODE, REJECTSPAM, SPAMREDIRECT, SPAMIGNORE, SPAMIGNOREPATTERNS, FILTERARGS, QUEUEDIR, QUEUE_BASE, QUEUE_START, QUEUE_COUNT, QMAILQUEUE, QUEUEPROG, RELAYCLIENT, QQEH, BADEXT, BADEXTPATTERNS, ACCESSLIST, EXTRAQUEUE, QUARANTINE, QHPSI, QHPSIMINSIZE, QHPSIMAXSIZE, QHPSIRC, QHPSIRN, USE_FSYNC, SCANCMD, PLUGINDIR, QUEUE_PLUGIN, PASSWORD_HASH, MAKESEEKABLE, MIN_FREE, ERROR_FD, DKSIGN, DKVERIFY, DKSIGNOPTIONS, DKQUEUE, DKEXCLUDEHEADERS, DKIMSIGN, DKIMVERIFY, DKIMPRACTICE, DKIMIDENTITY, DKIMEXPIRE, SIGN_PRACTICE DKIMQUEUE, SIGNATUREDOMAINS, and NOSIGNATUREDOMAINS
```

The following list of environment variables can be modified using envrules if QMAILLOCAL and QMAILREMOTE is set to /var/indimail/bin/spawn-filter.

```
QREGEX, SPAMFILTER, LOGFILTER, SPAMFILTERARGS, FILTERARGS, SPAMEXITCODE, HAMEXITCODE, UNSUREEXITCODE, REJECTSPAM, SPAMREDIRECT, SPAMIGNORE, SPAMIGNOREPATTERNS, DATABYTES, MDA, MYSQL_INIT_COMMAND, MYSQL_READ_DEFAULT_FILE, MYSQL_READ_DEFAULT_GROUP, MYSQL_OPT_CONNECT_TIMEOUT, MYSQL_OPT_READ_TIMEOUT, MYSQL_OPT_WRITE_TIMEOUT, QUEUEDIR, QUEUE_BASE, QUEUE_START, QUEUE_COUNT, and TMPDIR
```

The following list of environment variables which can be modified using envrules are specfic to qmail-remote.

```
CONTROLDIR, SMTPROUTE, SIGNKEY, OUTGOINGIP, DOMAINBINDINGS, AUTH_SMTP, MIN_PENALTY, and MAX_TOLERANCE
```

The following list of environment variables which can be modified using envrules are specfic to qmail-local. 

`USE_SYNCDIR, USE_FSYNC, and LOCALDOMAINS`

Do man qmail-smtpd(8), spawn-filter(8) to know the full list of environment variables that can be controlled using envrules.

# Setting up QMQP services

QMQP is faster than SMTP. You can use QMQP to send mails from your relay servers to a server running QMQP service. The QMQP service can deliver mails to your local mailboxes or/and relay mails to the outside world.

## Client Setup

QMQP provides a centralized mail queue within a cluster of hosts. QMQP clients do not require local queue for queueing messages.

For a minimal QMQP client installation, you need to have the following

* forward, qmail-inject, rmail, sendmail, predate, datemail, mailsubj, qmail-showctl, qmaildirmake, maildir2mbox, maildirwatch in /usr/bin;
* shared libs libsrs2-1.0\*  from /usr/lib64
* a symbolic link to qmail-qmqpc from /usr/sbin/qmail-queue; 
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail; 
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and 
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision. 

Everything can be shared across hosts except for /etc/indimail/control/idhost. 

Remember that users won't be able to send mail if all the QMQP servers are down. Most sites have two or three independent QMQP servers. 

Note that users can still use all the qmail-inject environment variables to control the appearance of their outgoing messages. This will include environment variables in $HOME/.defaultqueue directory.

If you want to setup a SMTP service, it might be easier to install the entire indimail-mta package and remove the services qmail-send.25. You can use svctool to remove the service e.g.

`$ sudo /usr/sbin/svctool --rmsvc qmail-send.25`

In case the mails generated by the client is to be relayed to the outside world, you should set the SMTP service and have /usr/sbin/sendmail, /usr/lib/sendmail linked to /usr/bin/sendmail. This is to ensure that tasks like virus scanning, dk, dkim signing happen at the client end. You can also choose not to have these tasks done at the client end, but rather have it carried out by the QMQP service.

## QMQP Service

IndiMail runs a QMQP service which handles incoming QMQP connections on port 628 using tcpserver. It uses multilog to store log messages under /var/log/svc/qmqpd.628

If you have installed IndiMail using the RPM, QMQP service is installed by default. However, you need to enable it.

```
$ sudo /bin/bash
# /bin/rm /service/qmail-qmqpd.628/down
# /usr/bin/svc -u /service/qmail-qmqpd.628
```

If you have installed IndiMail using the source, you may create the QMQP service using the following command

```
$ sudo /usr/sbin/svctool --qmqp=628 --servicedir=/service \
  --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
  --cntrldir=control --localip=0 --maxdaemons=75 --maxperip=25 \
  --fsync --syncdir --memory=104857600 --min-free=52428800
```

The above command will create a supervised service which runs qmail-qmqpd under tcpserver. In case you are setting up this service to relay mails to outside world, you might want to also specify --dkfilter, --qhpsi, --virus-filter, etc arguments to svctool(8) so that tasks like virus scanning, dk, domainkey signing, etc is done by the QMQP service.

A QMQP server shouldn't even have to glance at incoming messages; its only job is to queue them for qmail-send(8). Hence you should allow access to QMQP service only from your authorized clients. You can edit the file /etc/indimail/tcp.qmqp to grant specific access to clients. If you make changes to tcp.qmqp, don't forget to run the qmailctl command

`$ sudo /usr/bin/qmailctl cdb`

Note: Some of the tasks like virus/spam filtering, dk, dkim signing, etc can be done either by the client (if `QMAILQUEUE=/usr/sbin/qmail-multi`), or can be performed by QMQP service if **QMAILQUEUE** is defined as qmail-multi in the service's variable directory.

# Mini IndiMail Installation

IndiMail 1.7.3 onwards comes with option in svctool to install QMQP service. IndiMail 1.7.5 onwards comes with RPM package indimail-mini which will allow you to install a mini indimail installation. A mini indimail installation comes up with a bare minimum list of programs to enable you to send out mails. A indimail-mini installation doesn't have a mail queue. Instead it gives each new message to a central server through QMQP.

Many of my friends run web servers which need to send out emails. If you already have an installation of IndiMail messaging server on your network, you can quickly setup a mini indimail installation on your web server, without impacting the performance by using QMQP. To use QMQP service, you need to have QMQP service running on your IndiMail messaging server. All other servers (including your webservers) can have a indimail-mini installation.

![image](indimail_mini.png)

### How do I set up a QMQP service?

You need to have at least one host on your network offering QMQP service to your clients. IndiMail includes a QMQP server, qmail-qmqpd. Here's how to set up QMQP service to authorized client hosts on your IndiMail messaging server.
first create /etc/indimail/tcp.qmqp in tcprules format to allow queueing from the authorized hosts. make sure to deny connections from unauthorized hosts. for example, if queueing is allowed from 1.2.3.\*:

```
1.2.3.:allow
:deny
```

Then create /etc/indimail//tcp/tcp.qmqp.cdb:

```
$ sudo /usr/bin/tcprules /etc/indimail/tcp/tcp.qmqp.cdb \
    /etc/indimail/tcp/qmqp.tmp < /etc/indimail/tcp/tcp.qmqp
```

You can change /var/indimail/etc/tcp.qmqp and run tcprules again at any time. Finally qmail-qmqpd to be run under supervise:
NOTE: 628 is the TCP port for QMQP.

```
$ sudo /usr/sbin/svctool --qmqp=628 --servicedir=/service \
  --qbase=/var/indimail/queue --qcount=5 --qstart=1 \
  --cntrldir=control --localip=0 \
  --maxdaemons=75 --maxperip=25 --fsync --syncdir \
  --memory=104857600 --min-free=52428800
```


### How do I install indimail-mini?

A indimail-mini installation is just like a indimail installation, except that it's much easier to set up:

* You don't need MySQL
* You don't need /var/indimail/alias. A indimail-mini installation doesn't do any local delivery.
* You don't need indimail entries in /etc/group or /etc/passwd. indimail-mini runs with the same privileges as the user sending mail; it doesn't have any of its own files.
* You don't need to start anything from your boot scripts. indimail-mini doesn't have a queue, so it doesn't need a long-running queue manager.
* You don't need to add anything to inetd.conf. A null client doesn't receive incoming mail.

Here's what you do need:

* forward, qmail-inject, sendmail, rmail predate, datemail, mailsubj, qmail-showctl, maildirmake, maildir2mbox, and maildirwatch in your path
* shared libs libsrs2-1.0\* from /usr/lib64 (/usr/lib on 32 bit systems)
* a symbolic link to /usr/sbin/qmail-qmqpc from /usr/sbin/qmail-queue; 
* symbolic links to /usr/bin/sendmail from /usr/sbin/sendmail and /usr/lib/sendmail; 
* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers;
* a copy of /etc/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and 
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision. 
* All manual pages.

You can install all the above by manually copying the binaries and man pages from a host having standard IndiMail installation or you can install and setup just by using the indimail-mini RPM

`$ sudo rpm -ivh indimail-mini`

Apart from the binaries, you need to do the following

* a list of IP addresses of QMQP servers, one per line, in /etc/indimail/control/qmqpservers
* a copy of /var/indimail/control/me, /etc/indimail/control/defaultdomain, and /etc/indimail/control/plusdomain from your central server, so that qmail-inject uses appropriate host names in outgoing mail; and
* this host's name in /etc/indimail/control/idhost, so that qmail-inject generates Message-ID without any risk of collision.

Everything can be shared across hosts except for /etc/indimail/control/idhost.

Remember that users won't be able to send mail if all the QMQP servers are down. Most sites have two or three independent QMQP servers.

Note that users can still use all the qmail-inject environment variables to control the appearance of their outgoing messages. Also you can setup environment variables in $HOME/.defaultqueue

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

IndiMail also provides you authenticated SMTP providing **AUTH PLAIN**, **AUTH LOGIN**, **AUTH CRAM-MD5**, **CRAM-SHA1**, **CRAM-SHA256**, **CRAM-SHA512**, **CRAM-RIPEMD**, **DIGEST_MD5** methods. Whenever a user successfully authenticates through SMTP, the **RELAYCLIENT** environment variable gets set. qmail-smtpd uses the **RELAYCLIENT** environment variable to allow relaying.

Most of the email clients like thunderbird, evolution, outlook, outlook express have options to use authenticated SMTP.
For a tutorial on authenticated SMTP, you can refer to this [tutorial](http://indimail.blogspot.com/2010/03/authenticated-smtp-tutorial.html)

## Using control file relaydomains

Host and domain names allowed to relay mail through this host. Each  address should be followed by a colon and an (optional) string that should be appended to each incoming recipient address, just as with the **RELAYCLIENT** environment variable. Nearly always, the optional string should be null. Addresses in relaydomains may be wildcarded:

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

If the environment variable MAX\_RCPT\_ERRCOUNT is set qmail-smtpd will reject an email if in a SMTP session, the number of such recipients who do not exist, exceed MAX\_RCPT\_ERRCOUNT.

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

The mechanism is quite simple. For example, if you have the control file badmailfrom, qmail-smtpd will use badmailfrom. If you have the file badmailfrom.cdb, qmail-smtpd will first do cdb lookup in badmailfrom.cdb. To create badmailfrom.cdb, you need to run the command.

`$ sudo /usr/bin/qmail-cdb badmailfrom`

You can also have your entries in a MySQL table. Let say you have a MySQL server on the server localhost, a database named 'indimail' with user 'indimail' having password 'ssh-1.5-'. To enable the control file in MySQL you need to create the control file with a .sql extension. The following enables the badmailfrom in MySQL

```
# echo "localhost:indimail:ssh-1.5-:indimail:badmailfrom" > badmailfrom.sql
```

Once you have created a file badmailfrom.sql, qmail-smtpd will connect to the MySQL server on localhost and look for entry in the column 'email' in the table badmailfrom. If this table does not exist, qmail-smtpd will create an empty table using the following SQL create statement

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

Rather than making individual connections to MySQL for extracting information from the 'indimail' table, IndiMail programs use the service of the inlookup(8) server. Programs use an API function inquery() to request service. InLookup is a connection pooling server to serve requests for inquery() function. It is implemented over two fifos. One fixed fifo for reading the query and reading the path of a randomly generated fifo. The randomly generated fifo is used for writing the result of the query back. inlookup(8) creates a read FIFO determined by the environment variable **INFIFO**. If **INFIFO** is not defined, the default FIFO used is /var/indimail/inquery/infifo. inlookup(8) then goes into an infinite loop reading this FIFO. If **INFIFO** is not  an absolute path, inlookup(8) uses environment variable FIFODIR to look for fifo named by **INFIFO** variable. Inlookup(8) can be configured by setting environment variables in /service/inlookup.info/variables

* inlookup helps in optimizing connection to MySQL(1), by keeping the connections persistent.
* It also maintains the query result in a double link list.
* It uses binary tree algorithm to search the cache before actually sending the query to the database.
* IndiMail clients send requests for MySQL(1) queries to inlookup through the function inquery() using a fifo.
* The inquery() API uses the InLookup service only if the environment variable QUERY_CACHE is set. If this environment variable is not set, the inquery() function makes a direct connecton to MySQL. 
* Clients which are currently using inquery are qmail-smtpd(1), proxyimap(8), proxypop3(8), vchkpass(8) and authindi(8). 
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

IndiMail has multiple methods to insert your script anywhere before the queue, after the queue, before local delivery, before remote deliver or call a script to do local or remote delivery. Refer to the chapter **Writing Filters for IndiMail** for more details.

# SPAM Control using bogofilter

If you have installed indimail-spamfilter package, you will have [bogofilter](https://bogofilter.sourceforge.io/) providing a bayesian spam filter.

On of the easiest method to enable bogofilter is to set few environment variable for indimail's qmail-multi(8) the frontend for qmail-queue(8) program. e.g. to enable spam filter on the incoming SMTP on port 25:

bogofilter requires training to work. You can refer to Section 3, Step 8 in the document [INSTALL](INSTALL-indimail.md). You can also have a pre-trained database installed by installing the **bogofilter-wordlist** package.

```
$ sudo /bin/bash
# echo "/usr/bin/bogofilter -p -d /etc/indimail" > /service/qmail-smtpd.25/variables/SPAMFILTER
# echo "0" > /service/qmail-smtpd.25/variables/SPAMEXITCODE
# echo "0" > /service/qmail-smtpd.25/variables/REJECTSPAM
# echo "1" > /service/qmail-smtpd.25/variables/MAKESEEKABLE
```

Now qmail-multi(8) will pass every mail will pass through bogofilter before it passes to qmail-queue(8). You can refer to Chapter **IndiMail Queue Mechanism**, look at the picture to understand how it works. bogofilter(1) will add X-Bogosity in each and every mail. A spam mail will have the value `Yes` along with a probabality number (e.g. 0.999616 below). You can configure bogofilter in /etc/indimail/bogofilter.cf. The SMTP logs will also have lines having this X-Bogosity field. A detailed mechanism is depicted pictorially in the chapter **Virus Scanning using QHPSI**

```
X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The method describe above is a global SPAM filter. It will happen for all users, unless you use something like **envrules** to unset **SPAMFILTER** environment variable. You can use **envrules** to set **SPAMFILTER** for few specific email addresses. You can refer to the chapter on **Envrules** for more details.

There is another way you can do spam filtering - during local delivery (you could do for remote delivery, but what would be the point?). IndiMail allows you to call an external program during local/remote delivery by settting **QMAILLOCAL** / **QMAILREMOTE** environment variable. You could use any method to call bogofilter (either directly in *filterargs* control file, or your own script). You can see a Pictorial representation of how this happens. ![LocalFilter](indimail_spamfilter_local.png)

You can also use vcfilter(1) to set up a filter that will place your spam emails in a designated folder for SPAM. Refer to the chapter **Writing Filters for IndiMail** for more details.

# SPAM Control using badip control file

IndiMail has many methods to help deal with spam. For detecting spam, IndiMail uses bogofilter a fast bayesian spam filter. IndiMail's qmail-smtpd which provides SMTP protocol is neatly integrated with bogofilter. When bogofilter detects spam, qmail-smtpd prints the X-Bogosity header as part of SMTP transaction log

```
$ grep "X-Bogosity, Yes" /var/log/svc/smtpd.25/current
@400000004bc8183f01fcbc54 qmail-smtpd: pid 16158 from ::ffff:88.191.35.203 HELO X-Bogosity: Yes, spamicity=0.999616, cutoff=9.90e-01, ham_cutoff=0.00e+00, queueID=6cs66604wfk,
```

The value "Yes" in X-Bogosity indicates spam. You can tell qmail-smtpd to reject such mails at SMTP just by doing

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

When you use IndiMail, it is ultimately qmail-queue which is responsible for queueing your messages. qmail-queue stores the message component of queued mails (captured duing DATA phase of the SMTP conversation) under the mess subdirectory.

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

The above lines indicates that indimail-mta has received a new message, and its queue ID is 660188. What this means is that is qmail-queue has created a file named /var/indimail/queue/mess/NN/660188. The i-node number of the file is 660188. This is the queue file that contains the message. The queue ID is guaranteed to be unique as long as the message remains in the queue (you can't have two files with the same i-node in a filesystem).

To perform virus scanning, it would be trivial to do virus scanning on the mess file above in qmail-queue itself. That is exactly what IndiMail does by using a feature called Qmail High Performance Virus Scanner (**QHPSI**). **QHPSI** was conceptualized by Erwin Hoffman. You can read here for more details.

IndiMail takes **QHPSI** forward by adding the ability to add plugins. The **QHPSI** extension for qmail-queue allows to call an arbitary virus scanner directly, scanning the incoming data-stream on STDIN. Alternatively, it allows plugins to be loaded from the /usr/lib/indimail/plugins directory. This directory can be changed by defining PLUGINDIR environment variable. **QHPSI** can be advised to pass multiple arguments to the virus scanner for customization. To run external scanner or load scanner plugins, qmail-queue calls qhpsi, a program setuid to qscand. By default, qhpsi looks for the symbol virusscan to invoke the scanner. The symbol can be changed by setting the environment variable QUEUE_PLUGIN to the desired symbol.

Today’s virus scanner -- in particluar Clam AV -- work in resource efficient client/server mode (clamd/clamdscan) and include the feature to detect virii/worms in the base64 encoded data stream. Thus, there is no necessity to call additional programs (like reformime or ripmime) except for the virus scanner itself.

You can see how the scanning works ![Pictorially](indimail_spamfilter_global.png)

To enable virus scanning in IndiMail during the SMTP data phase, you can implement either of the two methods below

## 1. Using tcprules

Define QHPSI in tcp.smtp and rebuild tcp.smtp.cdb using tcprules.

`:allow,QHPSI=’/usr/bin/clamdscan %s --quiet --no-summary’`

## 2. Using envdir for SMTP service under supervise(8)

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

Once you have **QHPSI** enabled, qmail-queue will add the header **X-QHPSI** in the mail. You will have the following header

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

One of the feature that IndiMail adds to qmail-smtpd is accesslist between senders and recipients. Accesslist can be enabled by creating a control file /etc/indimail/control/accesslist. A line in accesslist is of the form

`type:sender:recipient`

where *type* is either the word 'from' or 'rcpt'. *sender* and *recipient* can be the actual *sender*, *recipient*, a wildcard or a regular expression (uses regex(3))

The accesslist happens during SMTP session and mails which get restricted get rejected with permanent 5xx code.

To give some examples

```
rcpt:ceo@indimail.org:country_distribution_list@indimail.org
rcpt:md@indimail.org:country_distribution_list@indimail.org
from:recruiter@gmail.com:hr_manager@indimail.org
```

* The above accesslist implies that only the users with email ceo@indimail.org and md@indimail.org can send a mail to the email country_distribution_list@indimail.org
* The 3rd line implies that all outside mails from the sender recruiter@gmail.com will be rejected at SMTP unless the recipient is hr_manager@indimail.org

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

Since IndiMail uses envdir program to set environment variable, a simple way would be to set SPAMFILTER, SPAMEXITCODE is to do the following

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

IndiMail 1.6 onwards implements greylisting using qmail-greyd daemon. You additionally need to have the environment variable GREYIP defined for the qmail-smtpd process. The environment variable GREYIP specifies on which IP and port, qmail-greyd is accepting greylisting requests. qmail-smtpd uses UDP to send a triplet (IP+RETURN\_PATH+RECIPIENT) to the greylisting server and waits for an answer which tells qmail-smtpd to proceed ahead or to temporarily reject the mail. qmail-greyd also accepts a list of whitelisted IP addresses for which greylisting should not be done.

## 1. Enabling qmail-greyd greylisting server

```
$ sudo svctool --greylist=1999 --servicedir=/service --min-resend-min=2 \
   --resend-win-hr=24 --timeout-days=30 --context-file=greylist.context \
   --save-interval=5 --whitelist=greylist.whitelist
```

NOTE: The above service has already been setup for you, if you have done a binary installation of IndiMail/indimail-mta

## 2. Enabling greylisting in SMTP

Assuming you've setup your qmail-smtpd service with tcpserver with the -x option (as in LWQ), you just need to update the cdb file referenced by this -x option. The source for this file is typically /etc/indimail/tcp.smtp. For example,

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
      
If you've setup qmail-greyd on a non-default address (perhaps you're running qmail-greyd on a separate machine), you'll also need to specify the address it's listening on - adjust the above to include GREYIP="192.168.5.5@1999", for example. 
Finally, don't forget to update the cdb file corresponding to the source file you've just edited. If you have a LWQ setup that's

```
$ sudo /bin/bash
# /usr/bin/qmailctl cdb
```
      
Alternatively (and particularly if you're not using the -x option to tcpserver) you can enable greylisting for all SMTP connections by setting GREYIP in the environment in which qmail-smtpd is started - for example your variables directory for qmail-smtpd can contain a file with the name GREYIP

```
$ sudo /bin/bash
# echo GREYIP=\"127.0.0.1@1999\" > /service/qmail-smtpd.25/variables/GREYIP
```
      
NOTE: The above instructions are for IndiMail/indimail-mta 2.x and above. For 1.x releases, use /var/indimail/etc for the location of tcp.smtp and tcp.smtp.cdb

# Configuring DKIM

**What is DKIM**

DomainKeys Identified Mail (DKIM) lets an organization take responsibility for a message while it is in transit. DKIM has been approved as a Proposed Standard by IETF and published it as RFC 4871. There are number of vendors/software available which provide DKIM signing. IndiMail is one of them. You can see the full list here.
DKIM uses public-key cryptography to allow the sender to electronically sign legitimate emails in a way that can be verified by recipients. Prominent email service providers implementing DKIM (or its slightly different predecessor, DomainKeys) include Yahoo and Gmail. Any mail from these domains should carry a DKIM signature, and if the recipient knows this, they can discard mail that hasn't been signed, or has an invalid signature.

IndiMail from version 1.5 onwards, comes with a drop-in replacement for qmail-queue for DKIM signature signing and verification (see qmail-dkim(8) for more details). You need the following steps to enable DKIM. IndiMail from version 1.5.1 onwards comes with a filter dk-filter, which can be enabled before mail is handed over to qmail-local or qmail-remote (see spawn-filter(8) for more details).

You may want to look at an excellent [setup instructions](http://notes.sagredo.eu/node/92) by Roberto Puzzanghera for configuring dkim for qmail

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

qmail-dkim uses openssl libraries and there is some amount of memory allocation that happens. You may want to increase your softlimit (if any) in your qmail-smtpd run script.

```
$ sudo /bin/bash
# cd /service/qmail-smtpd.25/variables
# echo "/usr/bin/qmail-dkim" > QMAILQUEUE
# echo "/etc/indimail/control/domainkeys/default" > DKIMSIGN
# svc -d /service/qmail-smtpd.25; svc -u /service/qmail-smtpd.25
```


## Set SMTP to verify DKIM signatures

You can setup qmail-stmpd for verification by setting
DKIMIVERIFY environment variable instead of DKIMSIGN environment variable.

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

You may not want to do DKIM signing/verificaton by SMTP. In that case, you have the choice of using the QMAILREMOTE, QMAILLOCAL environment variables which allows IndiMail to run any script before it gets passed to qmail-remote, qmail-local respectively.
Setting qmail-remote to sign with DKIM signatures On your host which sends out outgoing mails, it only make sense to do DKIM signing and not verification.

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

On your host which serves as your incoming gateway for your local domains, it only makes sense to do DKIM verification with qmail-local

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

If you experience problems, consult the qmail-dkim man page or post a comment below and I’ll try to help.
You can also use the following for testing.

* dktest@temporary.com, is Yahoo!'s testing server. When you send a message to this address, it will send you back a message telling you whether or not the domainkeys signature was valid. 
* sa-test@sendmail.net is a free service from the sendmail people. It's very similar to the Yahoo! address, but it also shows you the results of an SPF check as well. 

All the above was quite easy. If you don't think so, you can always use the magic options --dkverify (for verification) or `--dksign --private_key=domain_key_private_key_file` to svctool (svctool --help for all options) to create supervice run script for qmail-smtpd, qmail-send.

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
/service/mrtg/: up (pid 2443) 35254 seconds
```

Point your browser to /var/www/html/mailmrtg and you should see the graphs.

# RoundCube Installation for IndiMail

These instructions will work on CentOS, RHEL, Fedora. For Debian/Ubuntu and other distros, please use your knowledge to make changes accordingly. In this guide, replace indimail.org with your own hostname.
Non SSL Version Install/Configuration 
(look below for SSL config)

    1. Install RoundCube. On older systems, use the yum command
       `$ sudo dnf -y install roundcubemail php-mysqlnd`

    2. Connect to MySQL using a privileged user. IndiMail installation creates a privileged mysql user 'mysql'. It does not have the user 'root'. Look at the variable PRIV_PASS in /usr/sbin/svctool to know the password.

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
       For some reason pdo_mysql uses wrong mysql socket on some systems. Uses /var/lib/mysql/mysql.sock instead of /var/run/mysqld/mysqld.sock. You need to edit the file /etc/php.ini and define pdo_mysql.default_socket

       `pdo_mysql.default_socket= /var/run/mysqld/mysqld.sock`

       You can verify if the path has been correctly entered by executing the below command. The command should return without any error

       `$ php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"`

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

        2. Connect to MySQL using a privileged user. IndiMail installation creates a privileged mysql user 'mysql'. It does not have the user 'root'. Look at the variable PRIV_PASS in /usr/sbin/svctool to know the password.

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
       
        6. Change sauserprefs_db_dsnw in /usr/share/roundcubemail/sauserprefs/config.inc.php
           `$rcmail_config['sauserprefs_db_dsnw'] = 'mysql://roundcube:subscribed@localhost/RoundCube_db';`
       
        7. Restore indimail plugins for roundcube

           ```
           $ cd /tmp
           $ wget http://downloads.sourceforge.net/indimail/indimail-roundcube-ssl-1.0.tar.gz # This file
           $ cd /
           $ sudo tar xvfz /tmp/indimail-roundcube-ssl-1.0.tar.gz usr/share/roundcubemail/plugins
           $ /usr/bin/mysql -u mysql -p RoundCube_db < /usr/share/roundcubemail/sauserprefs/sauserprefs.sql
           ```

        8. Change pdo_mysql.default_socket /etc/php.ini
           For some reason pdo_mysql uses wrong mysql socket on some systems. Uses /var/lib/mysql/mysql.sock instead of /var/run/mysqld/mysqld.sock. You need to edit the file /etc/php.ini and define pdo_mysql.default_socket
           `pdo_mysql.default_socket= /var/run/mysqld/mysqld.sock`

           You can verifiy if the path has been correctly entered by executing the below command. The command should return without any error
           `php -r "new PDO('mysql:host=localhost;dbname=RoundCube_db', 'roundcube', 'subscribed');"`

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

It is advisable not to store the MySQL passwords in scripts. You can use mysql_config_editor to store encrypted credentials in .mylogin.cnf file. This file is read by MySQL clients during startup, hence avoiding the need of passing passwords on the command line. Remember passing passwords on the command line is insecure. With the --password option, mysql_config_editor will prompt you for the password to store

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

## 1. Database Initialization
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

Create a directory whose location can be provided to the secure_file_priv system variable, which limits import/export operations to that specific directory: 

```
$ mkdir -p /var/indimail/mysqldb/data
$ mkdir -p var/indimail/mysqldb/logs
$ chown -R mysql:mysql /var/indimail/mysqldb/data
$ chmod 750 /var/indimail/mysqldb/data
```

Initialize the data directory, including the mysql database containing the initial MySQL grant tables that determine how users are permitted to connect to the server. 

Typically, data directory initialization need be done only after you first installed MySQL. If you are upgrading an existing installation, you should run mysql_upgrade instead (see mysql_upgrade — Check and Upgrade MySQL Tables). However, the command that initializes the data directory does not overwrite any existing privilege tables, so it should be safe to run in any circumstances. Use the server to initialize the data directory; for example: 

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

For more information, see mysql_ssl_rsa_setup — Create SSL/RSA Files. 
Now using the password obtained from var/log/mysqld.log, we will connect to MySQL and create users

1. If the plugin directory (the directory named by the plugin_dir system variable) is writable by the server, it may be possible for a user to write executable code to a file in the directory using SELECT ... INTO DUMPFILE. This can be prevented by making the plugin directory read only to the server or by setting the secure_file_priv system variable at server startup to a directory where SELECT writes can be performed safely. (For example, set it to the mysql-files directory created earlier.) 
2. To specify options that the MySQL server should use at startup, put them in a /etc/my.cnf or /etc/mysql/my.cnf file. You can use such a file to set, for example, the secure_file_priv system variable. See Server Configuration Defaults. If you do not do this, the server starts with its default settings. You should set the datadir variable to /var/indimail/mysqldb/data  in my.cnf.
3. If you want MySQL to start automatically when you boot your machine, see Section 9.5, “Starting and Stopping MySQL Automatically”. 

Data directory initialization creates time zone tables in the mysql database but does not populate them. To do so, use the instructions in MySQL Server Time Zone Support. 

## 2. MySQL Startup

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

NOTE: You don’t need the supervise supervise service if you use the system installation default. If you have created this service, you can have supervise not start up mysqld by issuing the following command

`$ sudo touch /service/mysql.3306/down`

If you are doing to have supervise startup MySQL, then you need to disable the service from getting started up at boot. On modern systems, the command will be

```
$ sudo systemctl disable mysqld.service # MySQL from Oracle (it could also be mysql.service
$ sudo systemctl disable mariadb.service # MariaDB
```

## 3. Creating MySQL Users
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

NOTE: for MariaDB, you will have to execute few additional MySQL statements. The statements below also achieves what the script /bin/mysql_secure_installation does for MariaDB installations.

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

## 4. Creating MySQL Configuration

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

## 5. MySQL Control File

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

# 6. Configuring MySQL/MariaDB to use SSL/TLS

MySQL/MariaDB can encrypt the connections between itself and its clients. To do that you need to create certificates. There are differences betweeen how you configure MySQL server and MariaDB server for SSL/TLS

**MySQL community server**

`$ sudo mysql_ssl_rsa_setup –user=mysql –datadir=/var/indimail/mysqldb/data`

You will now find the following files in /var/indimail/mysqldb/data.

ca.pem, ca-key.pem, client-cert.pem, client-key.pem, server-cert.pem, server-key.pem, public_key.pem, private_key.pem.  If you are going to have multiple clients who need to connect to this server, you need to copy ca.pem and ca-key.pem to /var/indimail/mysqldb/data directory on the client host.

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
ca.pem, ca-key.pem, client-cert.pem, client-key.pem, server-cert.pem, server-key.pem, public_key.pem, private_key.pem.  If you are going to have multiple clients who need to connect to this server, you need to copy ca.pem and ca-key.pem to /var/indimail/mysqldb/ssl directory on the client host.

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