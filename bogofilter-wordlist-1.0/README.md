# Training from SpamAssassin Corpus

Welcome to the SpamAssassin public mail corpus.  This is a selection of mail
messages, suitable for use in testing spam filtering systems.  Pertinent
points:

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

Note: if you write a paper or similar using this corpus, and it's available for download, we'd love to hear about it!  Mail users AT spamassassin dot apache dot org.  cheers!

UPDATE: Jan 31 2006 jm: I've received a mail saying 'I'm seeing 41 messages [from the ham corpus](www.countermoon.com) hit on SURBL. Looks like the domain changed may have changed hands at some point.' So again, live lookups will probably now produce different results from what would have been seen at time of first email receipt; be warned.
