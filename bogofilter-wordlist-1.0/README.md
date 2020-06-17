# Training from SpamAssassin Corpus

Welcome to the SpamAssassin public mail corpus.  This is a selection of mail messages, suitable for use in testing spam filtering systems.  Pertinent points:

* All headers are reproduced in full.  Some address obfuscation has taken place, and hostnames in some cases have been replaced with "spamassassin.taint.org" (which has a valid MX record).  In most cases though, the headers appear as they were received.
* All of these messages were posted to public fora, were sent to me in the knowledge that they may be made public, were sent by me, or originated as newsletters from public news web sites.
* relying on data from public networked blacklists like DNSBLs, Razor, DCC or Pyzor for identification of these messages is not recommended, as a previous downloader of this corpus might have reported them!
* Copyright for the text in the messages remains with the original senders.


OK, now onto the corpus description.  It's split into three parts, as follows:

* spam: 500 spam messages, all received from non-spam-trap sources.
* easy_ham: 2500 non-spam messages.  These are typically quite easy to differentiate from spam, since they frequently do not contain any spammish signatures (like HTML etc).
* hard_ham: 250 non-spam messages which are closer in many respects to typical spam: use of HTML, unusual HTML markup, coloured text, "spammish-sounding" phrases etc.
* easy_ham2: 1400 non-spam messages.  A more recent addition to the set.
* spam_2: 1396 spam messages.  Again, more recent.
* spam_3: 393 spam messages.  Most recent.

Total count: 6439 messages, with about a 35% spam ratio.

The corpora are prefixed with the date they were assembled.  The messages are named by a message number and their MD5 checksum.

This corpus lives [here](http://spamassassin.apache.org/publiccorpus/).  Mail jm - public - corpus AT jmason dot org if you have questions.

NOTE: the training folder is missing in the git repository to avoid cluttering it with 1000s of spam messages. You need to recreate the folder from this [site](https://spamassassin.apache.org/old/publiccorpus), extract the files.

You can run the script download.sh to download the corpus. After extraction your training folder should look like this

```
$ sh ./download.sh
$ ls -l training
total 460
drwx--x--x. 2 mbhangui mbhangui 172032 Jun 29  2014 easy_ham
drwx--x--x. 2 mbhangui mbhangui  94208 Jun 29  2014 easy_ham_2
drwx--x--x. 2 mbhangui mbhangui  20480 Jun 29  2014 hard_ham
drwxr-xr-x. 2 mbhangui mbhangui  36864 Jun 29  2014 spam
drwxr-xr-x. 2 mbhangui mbhangui  98304 Mar  3  2009 spam_2
drwxr-xr-x. 2 mbhangui mbhangui  32768 Mar 16  2017 spam_3
```

The `easy_ham2` and `spam_3` folder above should be a collection of your own ham and spam emails
