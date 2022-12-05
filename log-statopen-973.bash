#!/bin/bash
# -*- mode: shell-script; sh-basic-offset: 8; tab-width: 8 -*-
# $ log-statopen.bash $
#
# This is bash script as bash can do  exec 973>"$logfile"
# (perl, python, ruby, ..., c would be next options...)
#
# Author: Tomi Ollila -- too ät iki piste fi
#
#	Copyright (c) 2022 Tomi Ollila
#	    All rights reserved
#
# Created: Wed 26 Jan 2022 19:06:28 +0200 too
# L.st modified: Mon 05 Dec 2022 18:31:13 +0200 too
# Last modified: Mon 05 Dec 2022 20:48:16 +0200 too

# SPDX-License-Identifier: BSD-2-Clause

set -euf
#set -x

# this script uses some bash features. therefore attempt to ensure it is
case ${BASH_VERSION-} in '') echo 'Not BASH!' >&2; exit 1; esac

die () { printf '%s\n' '' "$@" ''; exit 1; } >&2

case ${1-} in -x) setx='set -x'; shift ;; *) setx= ;; esac

if test $# -lt 3
then	exec >&2; case $0 in /*) b0=${0##*/} ;; *) b0=$0 ;; esac
	echo
	echo Usage: "$b0 [-x] LOGFILE '.' [NAME=VALUE]... [COMMAND [ARG]...]"
	echo
	exit 1
fi

saved_IFS=$IFS; readonly saved_IFS

if test "$2" != .
then
	die "Separator arg #2 '.' (between '$1' and '$2') missing from command line"
fi

logfile=$1
shift 2

#export var=val ... like LD_DEBUG=all (so that this bash itself is not affected)
for arg
do    case $arg in *[!a-zA-Z0-9_]*=*) break;
		;; *=*) export $arg; shift
		;; *) break
      esac
done
unset arg
test $# = 0 && die 'command [arg]... not given'

case $1
in */*)
	test -x "$1" || die "'$1': no such executable"
;; *)
	IFS=:
	for d in $PATH
	do	test -x "$d/$1" || continue
		IFS=$saved_IFS
		break
	done
	case $IFS in :) die "'$1': command not found"; esac
esac

if test -L "$0"
then
	lwd=`readlink -f "$0"`
	lwd=${lwd%/*}
else
	case $0 in /*) lwd=${0%/*}
		;; */*/*) lwd=${0%/*}; lwd=`readlink -f "$lwd"`
		;; ./*) lwd=$PWD
		;; */*) lwd=${0%/*}; lwd=`readlink -f "$lwd"`
		;; *) lwd=$PWD
	esac
fi

ld_preload=$lwd/ldpreload-log-statopen-973.so

test -f "$ld_preload" || die "'$ld_preload' does not exist" '' \
	"Execute  sh ldpreload-log-dlproc-life.c 973  to create it"

case $logfile
	in [0-9])  exec 973>&$1
	;; *)	   exec 973>"$logfile"
esac

#st='strace -o ttt'
st=

$setx
#exec /usr/bin/env -i "LD_PRELOAD=$ld_preload${LD_PRELOAD:+:$LD_PRELOAD}" \
#	"PATH=$PATH" LANG=C.UTF-8 LC_ALL=C.UTF-8 "$@"
LD_PRELOAD="$ld_preload${LD_PRELOAD:+:$LD_PRELOAD}" exec $st "$@"
