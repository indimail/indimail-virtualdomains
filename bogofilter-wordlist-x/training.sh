#!/bin/sh
if [ ! -d training ] ; then
	echo "Training folder not found!!" 1>&2
	exit 1
fi
if [ ! -x /usr/bin/bogofilter ] ; then
	echo "bogofilter not found" 1>&2
	exit 1
fi
cd training
for i in *ham*
do
	echo "bogofilter -d . -B -n $i"
	bogofilter -d . -B -n $i
done
for i in *spam*
do
	echo "bogofilter -d . -B -s $i"
	bogofilter -d . -B -s $i
done
echo "Created wordlist.db"
/bin/ls -l wordlist.db
echo "training database for bogofilter created from spamassasin corpus at"
echo "http://spamassassin.apache.org/old/publiccorpus/ in the training directory"
echo "You can updated easy_ham_2 and spam_2 subfolders in the training directory"
echo "using your own samples. Keep in mind that a good training requires the spam"
echo "database to be around 30% of the ham database"
echo "Every time you updated easy_ham_2 or spam_2 folder, don't forget to run"
echo "the training.sh script and copy wordlist.db to /etc/indimail with correct"
echo "Permissions."
exec mv wordlist.db ..
