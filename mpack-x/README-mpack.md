# mpack/munpack version 1.6 for unix

mpack and munpack are utilities for encoding and decoding (respectively) binary files in MIME (Multipurpose Internet Mail Extensions) format mail messages.  For compatibility with older forms of transferring binary files, the munpack program can also decode messages in split-uuencoded format. The Macintosh version can also decode messages in split-BinHex format.

The canonical [FTP site for this software](ftp.andrew.cmu.edu:pub/mpack/) is dead. Binaries are no longer provided. The pc, os2, amiga and archimedes ports have been removed. The mac version probably doesn't compile anymore, but is still included (MacOS X users can use the unix version...). You can however download mpack [here](https://github.com/indimail/indimail-virtualdomains/tree/master/mpack-x). mpack also comes as part of indimail-utils package from [indimail-virtualdomains](https://github.com/indimail/indimail-virtualdomains)

This MIME implementation is intended to be as simple and portable as possible.  For a slightly more sophisticated MIME implementation, see the program MetaMail, available via anonymous FTP to thumper.bellcore.com, in directory pub/nsb


# Decoding MIME messages:

First, you have to compile the munpack program.  See the instructions in the section "Compilation" below.  If, after reading the instructions, you are still unsure as to how to compile munpack, please try to find someone locally to help you.

To decode a MIME message, first save it to a text file.  If possible, save it with all headers included.  Munpack can decode some MIME files when the headers are missing or incomplete, other files it cannot decode without having the information in the headers.  In general, messages which have a statement at the beginning that they are in MIME format can be decoded without the headers.  Messages which have been split into multiple parts generally require all headers in order to be reassembled and decoded.

Some LAN-based mail systems and some mail providers (including America Online, as of the writing of this document) place the mail headers at the bottom of the message, instead of at the top of the message.  If you are having problems decoding a MIME message on such a system, you need to convert the mail back into the standard format by removing the system's nonstandard headers and moving the standard Internet headers to the top of the message (separated from the message body with a blank line).

There must be exactly one message per file. Munpack cannot deal with multiple messages in a single file, to decode things correctly it must know when one message ends and the next one begins.

To decode a message, run the command:

`munpack file`

where "file" is the name of the file containing the message. More than one filename may be specified, munpack will try to decode the message in each file.  For more information on ways to run munpack, see the section "Using munpack" below.

## Reporting bugs:

Bugs and comments should be reported to indimail-utils@indimail.org. When reporting bugs or other problems, please include the following information:

  * The version number of mpack
  * The platform (Unix, PC, OS/2, Mac, Amiga, Archimedes)
  * The EXACT output of any unsuccessful attempts.
  * If having a problem decoding, the first couple of lines of the input file.


## Compilation:

mpack uses autoconf and automake on unix.  refer to INSTALL for more information

```
$ cd /usr/local/src
$ git clone https://github.com/indimail/indimail-virtualdomains.git
$ cd /usr/local/src/indimail-virtualdomains/mpack-x
$ ./default.configure
$ make
$ sudo make install-strip
```

## Using mpack

mpack is used to encode a file into one or more MIME format messages.  The program is invoked with:

``
$ mpack [options] -o outputfile file

or 

$ mpack [options] file address...

or

$ mpack [options] -n newsgroups file
```

Where "[options]" is one or more optional switches described below.
"-o outputfile" is also described below. "file" is the name of the
file to encode, "address..." is one or more e-mail address to mail the
resulting messages to and "newsgroups" is a comma-separated list of
newsgroups to post the resulting messages to.

The possible options are:

```
-s subject
   Set the Subject header field to Subject.   By default,
   mpack will prompt for the contents of the subject
   header.

-d descriptionfile
   Include the contents of the file descriptionfile in an
   introductory section at the beginning of the first
   generated message.

-m maxsize
   Split the message (if necessary) into partial messages,
   each not exceeding maxsize characters.  The default
   limit is the value of the SPLITSIZE environment 
   variable, or no limit if the environment variable
   does not exist.  Specifying a maxsize of 0 means there
   is no limit to the size of the generated message.

-c content-type
   Label the included file as being of MIME type
   content-type, which must be a subtype of application,
   audio, image, or video.  If this switch is not given,
   mpack examines the file to determine its type.

-o outputfile
   Write the generated message to the file outputfile.  If
   the message has to be split, the partial messages will
   instead be written to the files outputfile.01,
   outputfile.02, etc.
```

The environment variables which control mpack's behavior are:


```
SPLITSIZE
Default value of the -m switch.  Default "0".

TMPDIR
Directory to store temporary files.  Default "/tmp".
```


## Using munpack:

Munpack is used to decode one or more messages in MIME or
split-uuencoded format and extract the embedded files.  The program is
invoked with:

`munpack [options] filename...`

which reads the messages in the files "filename...".  Munpack may also
be invoked with just:

`munpack [options]`

which reads a message from the standard input.

If the message suggests a file name to use for the imbedded part, that
name is cleaned of potential problem characters and used for the
output file.  If the suggested filename includes subdirectories, they
will be created as necessary.  If the message does not suggest a file
name, the names "part1", "part2", etc are used in sequence.

If the imbedded part was preceded with textual information, that
information is also written to a file. The file is named the same as
the imbedded part, with any filename extension replaced with
".desc"

The possible options are:

```
-f
   Forces the overwriting of existing files.  If a message
   suggests a file name of an existing file, the file will be
   overwritten.  Without this flag, munpack appends ".1", ".2",
   etc to find a nonexistent file.

-t
```

Also unpack the text parts of multipart messages to files. By default, text parts that do not have a filename parameter do not get unpacked.

```
-q
   Be quiet--suppress messages about saving partial messages.

-C directory
   Change the current directory to "directory" before reading
   any files.  This is useful when invoking munpack
   from a mail or news reader.
```

The environment variables which control munpack's behavior are:

```
TMPDIR
Root of directory to store partial messages awaiting 
reassembly.  Default is "/usr/tmp".   Partial messages
are stored in subdirectories of $TMPDIR/m-prts-$USER/
```


## Acknowledgements:

Written by John G. Myers, jgm+@cmu.edu

The mac version was written by Christopher J. Newman, chrisn+@cmu.edu

This version has been packaged as part of [indimail-utils package](https://github.com/indimail/indimail-virtualdomains)

Send all bug reports to indimail-utils@indimail.org 

Thanks to Nathaniel Borenstein for testing early versions of mpack and for making many helpful suggestions.

## PGP signature:

The mpack 1.6 distribution is not pgp signed.

## Legalese:

```
(C) Copyright 1993,1994 by Carnegie Mellon University
All Rights Reserved.

Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Carnegie Mellon
University not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior
permission.  Carnegie Mellon University makes no representations about
the suitability of this software for any purpose.  It is provided "as
is" without express or implied warranty.

CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

Portions of this software are derived from code written by Bell
Communications Research, Inc. (Bellcore) and by RSA Data Security,
Inc. and bear similar copyrights and disclaimers of warranty.
```
