#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf; trg=${0##*''/}; trg=${trg%.c}; test ! -e "$trg" || rm "$trg"
 WARN="-Wall -Wextra -Wstrict-prototypes -Wformat=2" # -pedantic
 WARN="$WARN -Wcast-qual -Wpointer-arith" # -Wfloat-equal #-Werror
 WARN="$WARN -Wcast-align -Wwrite-strings -Wshadow" # -Wconversion
 WARN="$WARN -Waggregate-return -Wold-style-definition -Wredundant-decls"
 WARN="$WARN -Wbad-function-cast -Wnested-externs -Wmissing-include-dirs"
 WARN="$WARN -Wmissing-prototypes -Wmissing-declarations -Wlogical-op"
 WARN="$WARN -Woverlength-strings -Winline -Wundef -Wvla -Wpadded"
 case ${1-} in '') set x -O2; shift; esac
 #case ${1-} in '') set x -ggdb; shift; esac
 set -x; exec ${CC:-cc} $WARN -Wno-unused-result "$@" -o "$trg" "$0"
 exit $?
 */
#endif
/*
 * Some approximation how long it takes to execute (a set of) syscalls...
 */

// SPDX-License-Identifier: BSD Zero Clause License (0BSD)

#define _GNU_SOURCE // at least sched_getcpu() needs this

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <time.h>
#include <sys/resource.h>

static const char * nsp(unsigned int u)
{
     /**/ if (u >= 100000000) return ".";
     else if (u >= 10000000) return ".0";
     else if (u >= 1000000) return ".00";
     else if (u >= 100000) return ".000";
     else if (u >= 10000) return ".0000";
     else if (u >= 1000) return ".00000";
     else if (u >= 100) return ".000000";
     else if (u >= 10) return ".0000000";
     else /* ---- */  return ".00000000";
}

static void print_nsd(long ns1, long ns2)
{
    char buf[128];
    int nsd = (int)(ns2 - ns1);
    if (nsd < 0) nsd += 1e9;
    int v = snprintf(buf, sizeof buf, "%ld %ld %s%d",
                     ns1, ns2, nsp(nsd), nsd);
    //write(1, buf, v); write(1, "\n", 1);
    int n = v - 1;
    buf[n + 3] = '\n';
    buf[n + 2] = buf[n]; n--;
    buf[n + 2] = buf[n]; n--;
    buf[n + 2] = buf[n]; n--;
    buf[n + 2] = ' ';
    buf[n + 1] = buf[n]; n--;
    buf[n + 1] = buf[n]; n--;
    buf[n + 1] = buf[n]; n--;
    buf[n + 1] = ' ';
    (void)write(1, buf, v + 3);
}

int main(void)
{
    char buf[1024];

    for (int i = 0; i < 4; i++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);

        // spend some time -- use strace to see which are syscalls...
#if 1
        getpid();
        getppid(); // 1000 - 2000 ns per "trivial" syscall (usually)
        getsid(0);
        getpgrp();
        getcwd(buf, sizeof buf);
#endif
        //sched_getcpu(); // ~30-100ns...
        struct rusage rusage; (void)getrusage(RUSAGE_SELF, &rusage);
        //for (int j = 0; j < 1000000; j++) write(20, buf, 3);
        //write(2, buf, 512); // remember 2>/dev/null
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        print_nsd(tv1.tv_nsec, tv2.tv_nsec);
    }
    write(1, "\n", 1);
    for (int i = 0; i < 4; i++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        //if (fork() == 0) exit(0); // ~100_000ns...
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        //clock_gettime(CLOCK_REALTIME, &tv2); // ~50-100ns...
        print_nsd(tv1.tv_nsec, tv2.tv_nsec);
    }
    return 0;
}
