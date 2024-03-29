## Introduction

This is fortune-1.0. It has been resurrected from fortune-mod 9708 below.  It has been changed so to use the standard ./configure and make, make install to install the package. Read the original README below for better understanding of this wonderful program.

The potentially offensive fortunes are not installed by default. If you're absolutely, *positively*, without-a-shadow-of-a-doubt sure that your user community wants them installed, specify --enable-offensive as an argument to configure and do "make install".

## Getting this Software

You will get this software as part of [indimail-virtualdomains](https://github.com/indimail/indimail-virtualdomains/tree/master/fortune-x) software

To install 

```
$ cd /usr/local/src
$ git clone https://github.com/indimail/indimail-virtualdomains.git
$ cd /usr/local/src/indimail-virtualdomains/fortune-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## fortune-mod README

This is fortune-mod 9708.  It is basically the same as the fortune-mod released in October 1995, with some portability improvements, bug fixes and documentation cleanup.

The most significant fix was for the reported bug about the `-a' option of fortune with database names that appear in both the inoffensive and offensive directories. This was solved by allowing the user to append '-o' to a fortune name while `-a' is in effect to force selection of the offensive version of a database.

Other changes/fixes: Fortune is now consistant in how it determines a fortune's length (for -s and -l).  The -m can now be used together with -s or -l: only fortunes which match _BOTH_ the pattern and the length requirement will be printed.

Most of the other features over the usual BSD fortune are summarised in Amy's README.Linux, included below, and in the accompanying manual pages for fortune(6) and strfile(1).

The changes in fortune.c and fortune.man are copyrighted by me but freely distributable: see the source files for details.  All other changes are in the public domain: you may do what you like with them.

[Dennis L. Clark](mailto:dbugger@progsoc.uts.edu.au) Thu, 28 Aug 1997 11:42:15 -0400

## Getting this Software

You can get this software [here](https://github.com/indimail/indimail-virtualdomains/tree/master/fortune-x)


## Amy's README.Linux

This version of fortune is a modification of the NetBSD fortune, as tweaked by Florian La Roche (see below, and many thanks to Florian for starting the update), and then massively hacked on by Amy Lewis.

I (Amy) hacked on this because it was broken; the BSD source itself is broken (I looked at it). Specifically, if you are using an old version of fortune, then it accesses *only* the two files "fortunes" and "fortunes-o", even though 'fortune -[ao]f' will tell you differently.  That was my original reason to start working with the code.

Bug fixes: fortune now reads the same file list that it reports with -f. strfile now really sorts and randomizes, instead of just setting the 'sorted' and 'randomized' flags.  strfile does not lose the pointer to a fortune that follows a null fortune.

### Enhancements
fortune -f now prints percentages, whether specified on the command line or not.

fortune -m now prints filenames to stderr; the fortunes printed to stdout can be redirected into a file which is valid strfile format.

fortune -l|s can be modified with -n *number* to specify the number of characters in a short fortune (default 160, as before).  The means of distinguishing between offensive and inoffensive fortunes is changed: offensive fortunes are put in a separate subdirectory. The contents of the fortunes databases have been extensively reviewed, and broken into smaller, more manageable [hopefully] files.

strfile is not notably enhanced, though it received the most significant bug fixes.  unstr now accepts a command line parameter -c *char* which globally changes the delimiter character.  unstr now accepts an output file as the second file parameter, and can tell if a file has a '.dat' extension.

An example of the use of fortune-style databases for other purposes, called randstr, has been added. See util/README.randstr

The Makefiles have been extensively hacked upon.

## Bugs

combining -a with xx% filename, when *filename* is found in both the offensive and the inoffensive directories, causes fortune to exit without an error message.  I think it's confused as to which file gets the xx%.  I should fix this.  Don't hold your breath, though.

For more information, see the files ChangeLog, Offensive, README.install, and cookie-files in the top-level directory, and the comments in the various \*.c source files.

[Amy A. Lewis](mailto: alewis@email.unc.edu) October, 1995

## Florian's README.LINUX

I have looked at sunsite and tsx and found one very old fortune program and one in the debian Linux distribution. But comparing that one with the version in NetBSD-current showed me, that NetBSD-current has fixed so many speeling-bugs that I just had to repackage everything for the Linux community.

In the source package are all changes for Linux in the file LINUX.DIF. (Rewriting the Makefiles and some trivial small fixes.)

I expect this "fortune.tar.gz" to show up under /pub/Linux/games.

Not only the kernel needs speeling-corrections,

[Florian La Roche](mailto: florian@jurix.jura.uni-sb.de) April 1995


PS. The following is the README from the originating NetBSD fortune:

```
#	$NetBSD: README,v 1.2 1995/03/23 08:28:29 cgd Exp $
#	@(#)README	8.1 (Berkeley) 5/31/93

The potentially offensive fortunes are not installed by default on BSD
systems. If you're absolutely, *positively*, without-a-shadow-of-a-doubt
sure that your user community wants them installed, whack the Makefile
in the subdirectory datfiles, and do "make all install".

```
Some years ago, my neighbor Avery said to me:

"There has not been an adequate jokebook published since Joe_Miller,
which came out in 1739 and which, incidentally, was the most miserable no-good ...
jokebook in the history of the printed word."

In a subsequent conversation, Avery said: "A funny story is a funny
story, no matter who is in it - whether it's about Catholics or Protestants,
Jews or Gentiles, blacks or whites, browns or yellows. If a story is genuinely
funny it makes no difference how dirty it is.  Shout it from the rooftops.
Let the chips fall all over the prairie and let the bonehead wowsers yelp.
... on them."

It is a nice thing to have a neighbor of Avery's grain.  He has
believed in the aforestated principles all his life.  A great many other
people nowadays are casting aside the pietistic attitude that has led them
to plug up their ears against the facts of life.  We of The Brotherhood
believe as Avery believes; we have never been intimidated by the pharisaical
meddlers who have been smelling up the American landscape since the time of
the bundling board.  Neither has any one of our members ever been called a
racist.  Still, we have been in unremitting revolt against the ignorant
propensity which ordains, in effect, that "The Green Pastures" should never
have been written; the idiot attitude which compelled Arthur Kober to abandon
his delightful Bella Gross, and Octavius Roy Cohen to quit writing about the
splendiferous Florian Slappey; the moronic frame of mind which, if carried
to its logical end, would have forbidden Ring Lardner from writing in the
language of the masses.

-- H. Allen Smith, "Rude Jokes"

... let us keep in mind the basic governing philosophy of The
Brotherhood, as handsomely summarized in these words: we believe in
healthy, hearty laughter -- at the expense of the whole human race, if
needs be.
Needs be.
-- H. Allen Smith, "Rude Jokes"
```

## NOTE

This version has been packaged as part of [indimail-utils package](https://github.com/indimail/indimail-virtualdomains)

Send all bug reports to indimail-utils@indimail.org 
