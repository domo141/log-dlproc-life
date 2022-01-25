#!/bin/sh
#
# $ minenv.sh $
#
# Created: Sat 08 Jan 2022 22:04:48 EET too
# Last modified: Fri 14 Jan 2022 23:23:02 +0200 too

# SPDX-License-Identifier: BSD Zero Clause License (0BSD)

case ${BASH_VERSION-} in *.*) set -o posix; shopt -s xpg_echo; esac
case ${ZSH_VERSION-} in *.*) emulate ksh; esac

set -euf  # hint: sh -x thisfile [args] to trace execution

die () { printf '%s\n' "$@"; exit 1; } >&2
x_exec () { printf '+ %s\n\n' "$*" >&2; exec "$@"; die "exec '$*' failed"; }

test $# -gt 0 || die "Usage: ${0##*/} command [args]"

# undocumented options (in usage) (practically unusable when both used)

# note also, that in both 2 cases below, the program executed will get pid 1,
# and it should be able to reap grandchild processes which parents have
# exited before the grandchild process themselves.

if test "$1" = --podman-run
then
	test $# -gt 2 || die "Usage: ${0##*/} $1 {image} command [args]"
	cntr=$2
	shift 2
	x_exec  podman run --pull=never --rm -it --privileged \
		-v "$PWD:$PWD" -w "$PWD" "$cntr" "$0" "$@"
	die 'not reached'
fi

if test "$1" = --unshare-pid
then
	test $# -gt 1 || die "Usage: ${0##*/} $1 command [args]"
	shift
	pcl='unshare -r --pid --fork --mount-proc'
else
	pcl=
fi

# LC_* and XDG_*... well perhaps the list below could be made even shorter...

for arg
do case $arg in *=*) export $arg; shift ;; *) break ;; esac
done

#for e in XAUTHORITY USER USERNAME TERM PWD PATH LOGNAME LANG HOME
for e in USER PWD PATH LANG HOME
do
	eval v=\${$e-:::}
	test "$v" = ::: && continue
	set -- "$e=$v" "$@"
done

x_exec $pcl env -i "$@"

# Local variables:
# mode: shell-script
# sh-basic-offset: 8
# tab-width: 8
# End:
# vi: set sw=8 ts=8
