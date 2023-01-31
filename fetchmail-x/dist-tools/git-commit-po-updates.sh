#!/bin/sh
# dist-tools/git-commit-po-updates.sh
# A helper script to commit translation updates into Git.
# Assumes translation updates are visible to git,
# and one directory above.
#
# © Copyright 2019 - 2020 by Matthias Andree.
# Licensed under the GNU General Public License V2 or,
# at your choice, any later version.
#
# Supported modes:
# -n:   dry-run, only print commands, but do not run them.
# -c:   commit, print commands and run them.

# Exit codes:
# 0: success, no new po/*.po files.
# 1: error
# 2: usage was printed, nothing was done
# 3: new po/*.po files detected
set -eu

unset IFS
cd "$(realpath $(dirname $0))/.."

# see if Perl has Carp::Always available,
# and implicitly fail (set -e) if it hasn't.
perl -MCarp::Always -e ''

usage() {
	printf 'Usage: %s {-n|-c}\n-n: dry-run; -c: commit\n' "$0"
	exit $1
}

# print and potentially run a shell command
run() {
	cmd="$1 '$2'"
	printf '+ %s\n' "$cmd"
	$dryrun_pfx eval "$cmd"
	return $?
}

# try to parse features of a po file and build a git command
handle_po() {
	pofile="$1"
	logmsg1="$2" # "Update", or "Add new"
	logmsg2="$3" # "to", or "for"
	export logmsg1 logmsg2
	if ! cmd="$(perl -WT - "$pofile" <<'_EOF'
use Encode::Locale;
use Encode;
use strict;
use Carp::Always ();
use warnings FATAL => 'uninitialized';
my ($ver, $dat, $translator, $lang, $lcod, $cset, $found);

while(<>)
{
	if (/^"Project-Id-Version: (.+)\\n"/)	{ $ver=$1; };
	if (/^"PO-Revision-Date: (.+)\\n"/)	{ $dat=$1; };
	if (/^"Last-Translator: (.+)\\n"/)	{ $translator=$1; };
	if (/^"Language-Team: ([^<]+?)\s+<.*>\\n"/)
						{ $lang=$1; };
	if (/^"Language: (.+)\\n"/)		{ $lcod=$1; };
	if (/^"Content-Type: text\/plain; charset=(.+)\\n"/)
						{ $cset = $1; };
	if ($ver and $dat and $translator and $lang and $lcod and $cset) {
		$found = 1;
		last;
	}
}

$translator = Encode::decode($cset, $translator);

if ($found) {
	print Encode::encode(locale => "git commit --author '$translator' --date '$dat' -m '$ENV{logmsg1} <$lcod> $lang translation $ENV{logmsg2} $ver'", Encode::FB_CROAK);
} else {
	exit(1);
}
_EOF
)"
	then
		echo >&2 "Parsing $pofile failed, skipping."
		return 23
	fi
	run "$cmd" "$pofile"
	return $?
}

dryrun_pfx=
docommit=
while getopts 'nc' opt ; do
  case $opt in
  n) dryrun_pfx=: ;;
  c) docommit=y ;;
  ?)
     usage 2 ;
  esac
done

rc=0

if [ -z "$dryrun_pfx" -a -z "$docommit" ] ; then usage 2 ; fi

new_po_files=$(git status --porcelain=v1 po/*.po | egrep '^(\?|.\?|A)' | cut -c4-)
for nfile in $new_po_files ; do
	run "git add" "$nfile"
	r=$?
	handle_po "$nfile" "Add new" "for"
	if [ $r -ne 0 -o $? -ne 0 ] ; then
		echo "There were errors adding $nfile" >&2 ; rc=1
	fi
done


git diff -G '^"(Project-Id-Version|PO-Revision-Date):' --name-only po/*.po \
| while read pofile ; do
	if ! handle_po "$pofile" "Update" "to" ; then
		echo "There were errors updating $pofile" >&2 ; rc=1
	fi
done

if [ -n "$new_po_files" ] ; then
	printf "Remember to add these codes to po/LINGUAS:"
	for i in $new_po_files ; do
		j=${i%.po}
		j=${j#po/}
		printf " %s" "$j"
	done
	printf '\n'
	if [ $rc -eq 0 ] ; then rc=3 ; fi
fi

exit $rc
