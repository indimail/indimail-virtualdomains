# indimail
Messaging Platform based on qmail for MTA Virtual Domains, Courier-IMAP for IMAP/POP3

Look at doc/README for details

Look at doc/INSTALL for Source Installation instructions

Compile libqmail-0.1

 $ cd /usr/local/src
 
 $ git clone https://github.com/mbhangui/libqmail
 
 $ cd libqmail-0.1
 
 $ ./default.configure
 
 $ make
 
 $ sudo make install-strip
   
Download indimail, indimail-mta and components

 $ git clone https://github.com/mbhangui/indimail.git

Compile libdkim-1.4 (with dynamic libaries)

 $ cd /usr/local/src/libdkim-1.4
 
 $ ./default.configure
 
 $ make
 
 $ sudo make -s install-strip

Compile libsrs2-1.0.18 (with dynamic libaries)

 $ cd /usr/local/src/libsrs2-1.0.18
 
 $ ./defaualt.configure
 
 $ make
 
 $ sudo make install-strip
 

Build ucspi-tcp-0.88 (with all patches applied)

 $ cd /usr/local/src/ucspi-tcp-0.88
 
 $ make
 
 $ sudo make install-strip
  
Build indimail-mta-2.7

 $ cd /usr/local/src/indimail-mta-2.7
 
 $ make
 
 $ sudo make install-strip

Setup & Configuration

 Setup (this uses svctool a general purpose utility to configure indimail-mta
 services. The create_services is a shell script which uses svctool to setup
 indimail-mta. It will also put a systemd unit file indimail.service in
 /lib/systemd/system

 $ cd /usr/local/src/indimail-mta-2.7
 
 $ sudo sh ./create_services --servicedir=/services --qbase=/var/indimail/queue

 $ sudo service indimail start
 
     or
     
 $ /etc/init.d/indimail start
 
     or
     
 $ /usr/bin/qmailctl start


Some Notes on directory structure
==========================
indimail-mta has files in standard unix directories. You can change
the locationsby editing the following files in indimail-mta source
directory

conf-prefix       - this is where bin, sbin go
conf-shared       - this is where boot, doc go (conf-prefix/share/indimail)
conf-sysconfdir   - this is where etc, control, users go
conf-libexec      - this is where private scripts/executables go
conf-qmail        - domains, alias, queue, autoturn, qscanq, symlinks
                    for control, users, bin and sbin

You can have the old non-fhs behaviour by having /var/indimail in the
above 4 files. In addition to the above, indimail uses the hardcoded
directory /usr/lib/indimail in build scripts

/usr/lib/indimail - plugins, modules (architecture-dependent files)

Some settings
===========

conf-shared       - /usr/share/indimail
conf-prefix       - /usr
conf-sysconfdir   - /etc/indimail
conf-libexec      - /usr/libexec/indimail
conf-qmail        - /var/indimail
