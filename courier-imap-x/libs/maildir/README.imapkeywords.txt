                   Courier-IMAP: IMAP keywords implementation

   This white paper describes how Courier-IMAP implements IMAP keywords. This
   document is provided for informational purposes only.

Background

   Courier-IMAP is a maildir-based IMAP server. The reader is presumed to
   know how maildirs work.

   There are five pre-defined flags that may be set on each message in an
   IMAP folder: \Seen, \Answered, \Draft, \Deleted, and \Flagged. An IMAP
   server may also optionally offer the ability to set arbitrary
   client-defined flags for any message.

Implementation Requirements

     • Maintain the high-performance, lock-free nature of maildir-based mail
       stores.
     • The current version of Courier-IMAP offers an option to use light,
       dot-lock based locking to minimize undesirable side-effects brought by
       concurrent folder updates by multiple IMAP clients. Keyword usage
       should not rely on locking being enabled.
     • Reading and saving keywords should be reasonably fast, even with large
       folders.
     • Obtaining a list of all keywords set for a given message should be a
       fast operation.
     • Obtaining a list of all messages with a given keyword set should also
       be a fast operation.
     • Updating keywords should be a reasonably fast operation.
     • Should not have any noticable overhead unless keywords are actually
       used.

Implementation Details

   The rest of this document describes the technical keyword implementation.
   This is a short summary of the implement issues that should be understood
   when using IMAP keywords with Courier-IMAP.

     • On systems that impose a fixed upper limit on the maximum number of
       files in a directory, the number of messages whose keywords may be
       adjusted within a 15-20 minute window may not exceed 1/3rd of the
       upper limit. For example, Linux ext2 filesystem directories can hold
       about 30,000 files, maximum. On Linux systems, no more than 10,000
       messages (in the same folder, of course) may have their keywords
       changed within any 15-20 minute window.
     • The atomicity is on a per-message basis. All keywords set for a
       particular message are saved as an atomic unit. A client adjusts the
       keywords that are set for a particular message by reading the existing
       set of keywords, and then replacing them with a new set of keywords.
       This means that when multiple clients update the keyword set of the
       same message, the last update wins. Changes made by the losing client
       are lost. Moral of the story: do not allow multiple clients to mess
       with the same message, at the same time.

Data storage

   A new subdirectory, courierimapkeywords, is created in the maildir. It
   stores keyword-related data.

   The file courierimapkeywords/:list contains a "stable, known list" of all
   keywords sets for all messages. It is, essentially, a list of the base
   filenames of each message in the cur directory that has keywords, without
   the ":2," suffix, and any message flags. Messages without any set keywords
   are not listed in this file.

   Additional files may also exist in this subdirectory, named either
   .N.file, or file. file is the base filename of a message, while "N" is a
   numeric value.

   The list of keywords set for all messages is obtained by reading the
   contents of courierimapkeywords according to the process described below.

Updating keywords

   A keywords set for a message may be updated as follows:

     • Create a file in tmp, containing the new keywords that are set for the
       message. To remove all existing keywords, the file should be empty.
     • Rename the file as courierimapkeywords/file, with file matching the
       message's base filename.

Reading keywords

   First, a list of all messages present in new and cur is obtained. Then:

     • Read courierimapkeywords/:list. Ignore non-existent base filenames
       read from the :list file.
     • Divide the current time, in seconds, by 300. Call the result T.
     • Read the contents of the courierimapkeywords directory. Ignore :list,
       the remaining files in the directory will be named either "file", or
       ".N.file" where N is a number. When encountering a file that cannot be
       found in the current list of messages present in new and cur, stat the
       file, and remove it if its ctime is at least fifteen minutes old
       (prevents removal of keywords for a message that's just been added to
       the folder, and the scan for messages in new and cur just missed it).
     • When encountering ".N.file" after another ".N.file" was encountered
       earlier, remove the file with the lesser N, unless the larger of the
       two Ns is greater than or equals to T. Keep track of the largest N
       seen that's less than T, and the largest N that's seen that's greater
       than or equals to T. When encountering a "file", add it a list of all
       "file"s that were encountered, and process this entry as if it were
       .X.file, where X=T+1.
     • After reading the entire directory, apply the following changes to the
       keywords read from the :list file: the contents of every file seen; if
       file was not seen but a .N.file was seen, then the contents of the
       file with the largest N. If an attempt to open an update file failed
       with ENOENT, restart everything from step 1.
     • Write the new set of keywords to a temporary file in tmp, then rename
       it as courierimapkeywords/:list. Afterwords go through the list of all
       "file"s that were encountered two steps ago, and rename each "file" to
       ".X.file". This step should be omitted unless at least one nonexistent
       file was skipped in the old :list file, or the contents of at least
       one .N.file was updated to :list.
     • If exactly one .N.file was seen, and N<T, remove the lone .N.file.
