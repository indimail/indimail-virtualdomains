## Introduction

flash is an attempt to create a secure menu-driver shell for UNIX-derived OSes, while providing user-friendliness and easy configurability. An ideal situation requiring the use of flash would be a student-run telnet server which needs to:

* shelter the users from some of the nastinesses of UNIX
* shelter the system from nasty users
* provide an easy way to launch applications
* support multitasking/job control as elegantly as possible
* support easy-to-get-right configuration by administrators

flash offers the following features:

* uses NCURSES menus driven by cursor keys 
* offers 'hotkey' functionality for program invocation
* loadable modules for "NEW MAIL" and clock functions
* password lockable screen saver support

It currently compiles nicely under RedHat 5.x & 6.x, SuSe 6.x (x86), RedHat Alpha, and Solaris 5.5 upwards. However for Solaris please read the Makefile, and you will need to use GNU make for this version.  If you manage to get it to compile elsewhere, please let us know.

## History

flash was developed by Stephen Fegan of the UCD Internet Society (steve@netsoc.ucd.ie) when the UCD Netsoc were just beginning. It was used with great success to shelter non-computer-literate users from the complicated operating system as well as protecting our setup from hackers of all kinds. Unfortunately the original developer no longer responds to our emails about the program so we have decided to release the program as open source to ensure it will be developed. The original developer did actually intend to do this at some point (the GPL declarations are his) but just didn't get around to it. Kudos to Steve for his work, which built on a very old menu driven shell called 'flin'.  Steve almost totally rewrote it during the course of the development.

* why did you call your program "Flash" ..aren't there enough programs out
* there with the same name already???  Ever hear of Macromedia Flash?

From Niall R. Murphy:

```
There was actually a naming conflict from the beginning, since Steve already had another program called flash (which did nasty things to world writable ttys). I persuaded him that flash was the right name since the menus and backgrounds and so on and forth were... well... flashy :)

This flash was around in 1996. I don't know if that's before Macromedia. I don't think either of us had heard of it by then anyway.
```

## Installation:

The creation of the binaries should be as simple as "./configure;make" on a Linux system.

```
$ cd /usr/local/src
$ git clone https://github.com/indimail/indimail-virtualdomains.git
$ cd /usr/local/src/indimail-virtualdomains/flash-x
$ ./default.configure
$ make
$ sudo make install-strip
```

There are a few important things to remember: 

* system.flashlogin is the file executed by flash upon login.
  You can set environment variables, check the return codes from
  executed programs and so on.

* system.menu is the menu definition file. In here you can define what
  programs are executed with what arguments. It should be fairly easy
  to figure out what's going on here. Better documentation would be 
  a help. You can also include other definition files; we include
  module configuration files here.

* This can't be stressed enough - when you are adding programs to
  the system.menu AUDIT THEM FOR SHELL ESCAPES FIRST. It is /no/
  use giving people flash as a shell and then allowing them access
  to vi, tin or even pine. These programs can all be manipulated
  into giving the user a full shell, some trivially, some not so
  trivially. Heavy source code review (removing all system() and
  most exec() calls for example) is the minimum necessary for peace
  of mind.      

There is also a man page included in the distribution but it's of
dubious usefulness. Knock up a better one...

## Known bugs:

* Flash doesn't like being straced under Linux (ncurses?).
* You have to be quite careful with your terminal type sometimes.

We expect this list to grow larger :-)

## FAQ

**What is `Flash'?**

```
Flash is a Un*x/ncurses menu interface.
I originally wrote it when I tried to make a shell script to write a
backup/restoration tool. I found that shell scripts are completely 
impossible to write in a decent way and just plain ugly to look at. 
So I had my hand at writing my own.
```

**How do I write menus?**

This is a short primer, please see the file Manual for further details.

Flash menus are very simple to write and read. They are laid out in a very logical way much like the menus to 'fvwm' (1.xx anyway).  Unfortunately, at this point the parser is a bit crude, but it has nice verbose output so you can trace down the problem in the menu file really easily.

A menu starts with the word **Menu** followed by the menu name (not the title), and ends with the word **EndMenu**. Each menu consists of a variety of tokens and their respective arguments, tokens and arguments are split by the character `:`. At this point there is no escape character, so you cannot use `:` in menu text. Every token has at least one argument, which is what will appear on the screen.

The tokens currently recognized by Flash are as follows:

* **Title**: denotes the title of the menu (you can have as many as you like).

  `Title:Main Menu:`
* **Exec** denotes an executable item and has two arguments, the second is the command line to execute.

  `Exec:Read Mail:pine:`
* **Args** denotes an executable with user-inputed command line arguments. The optional **Prompt** field contains the title for the dialog box. USE WITH CARE, this could be a whopping security whole when used improperly.

  `Args:Send a file (ZMODEM):sz:Prompt:`
* **SubMenu** denotes a link to another menu and has two arguments, the second is the name of the Menu to link to.

  SubMenu:File Utilities:File-Util:
* **Exit** quits the current menu, if the current menu is the first menu, you will be logged out.

  `Exit:Back to Main Menu:`
* **Quit** Log out of Flash.

  `Quit:Logout:`
* **Nop** is for comments

  `Nop:This is a comment:`

**Putting it together**

To execute the menu, you can either put the line `#!/usr/bin/flash` at the first line in the menu, or type `flash <menu-name>`.

A sample menu is provided with this distribution, please examine it.


## NOTE

The latest stable version of flash should always be available [here](https://github.com/indimail/indimail-virtualdomains/tree/master/flash-x)

This version has been packaged as part of [indimail-utils package](https://github.com/indimail/indimail-virtualdomains)

Send all bug reports to indimail-utils@indimail.org 
