# indimail
Messaging Platform based on qmail for MTA Virtual Domains, Courier-IMAP for IMAP/POP3

Look at indimail-3.x/doc/README for details. indimail needs indimail-mta to be installed. Look at 
https://github.com/mbhangui/indimail-mta
for details on installing indimail-mta

Look at indimail-3.x/doc/INSTALL for Source Installation instructions

# Compile libqmail-0.1

 $ cd /usr/local/src
 
 $ git clone https://github.com/mbhangui/libqmail-0.1
 
 $ cd /usr/local/src/libqmail-0.1
 
 $ ./default.configure
 
 $ make
 
 $ sudo make install-strip
   
# Download indimail, indimail-mta and components

 $ cd /usr/local/src

 $ git clone https://github.com/mbhangui/indimail-virtualdomains.git

 $ git clone https://github.com/mbhangui/indimail-mta.git

# Compile libdkim-1.4 (with dynamic libaries)

 $ cd /usr/local/src/indimail-mta/libdkim-1.4
 
 $ ./default.configure
 
 $ make
 
 $ sudo make -s install-strip

# Compile libsrs2-1.0.18 (with dynamic libaries)

 $ cd /usr/local/src/indimail-mta/libsrs2-1.0.18
 
 $ ./defaualt.configure
 
 $ make
 
 $ sudo make install-strip
 

# Build ucspi-tcp-0.88 (with all patches applied)

 $ cd /usr/local/src/indimail-mta/ucspi-tcp-0.88
 
 $ make
 
 $ sudo make install-strip
  
# Build indimail-mta-2.8

 $ cd /usr/local/src/indimail-mta/indimail-mta-2.8
 
 $ make
 
 $ sudo make install-strip

# Build indimail-3.x

 $ cd /usr/local/src/indimail-virtualdomains/indimail-3.0

 $ ./default.configure

 $ make

 $ sudo make install-strip

# Setup & Configuration

 Setup (this uses svctool a general purpose utility to configure indimail-mta
 services. The create_services is a shell script which uses svctool to setup
 indimail-mta. It will also put a systemd unit file indimail.service in
 /lib/systemd/system

 $ cd /usr/local/src/indimail-mta/indimail-mta-2.7
 
 $ sudo sh ./create_services --servicedir=/services --qbase=/var/indimail/queue

 $ sudo service indimail start
 
 or
     
 $ /etc/init.d/indimail start
 
 or
 
 $ /usr/bin/qmailctl start
