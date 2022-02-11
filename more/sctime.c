#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf; trg=${0##*''/}; trg=${trg%.c}; test ! -e "$trg" || rm "$trg"
 case ${1-} in '') set x -O2; shift; esac
 #case ${1-} in '') set x -ggdb; shift; esac
 set -x
 exec ${CC:-cc} -std=c99 -Wno-unused-result "$@" -o "$trg" "$0" -lrt
 exit $?
 */
#endif
/*
 * Some approximation how long it takes to execute (a set of) syscalls...
 * note: clock_gettime() and sched_getcpu() are (apparently) not syscalls
 *    -- also time spent in clock_gettime() is not substracted --
 */

// SPDX-License-Identifier: BSD Zero Clause License (0BSD)

#if defined(__linux__) && __linux__
// on linux: man feature_test_macros -- try ftm.c at the end of it
#define _DEFAULT_SOURCE 1
// for older glibc's on linux (< 2.19 -- e.g. rhel6 uses 2.12...)
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1
#define _POSIX_C_SOURCE 200809L
#define _ATFILE_SOURCE 1
// more extensions (less portability?)
#define _GNU_SOURCE // at least sched_getcpu() needs this
#endif

// hint: gcc -dM -E -xc /dev/null | grep -i gnuc
// also: clang -dM -E -xc /dev/null | grep -i gnuc
#if defined (__GNUC__)

#if 1 // use of -Wpadded may get complicated, 32 vs 64 bit systems
#pragma GCC diagnostic warning "-Wpadded"
#endif

// to relax, change 'error' to 'warning' -- or even 'ignored'
// selectively. use #pragma GCC diagnostic push/pop to change
// the rules temporarily

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

#if __GNUC__ >= 8 // impractically strict in gccs 5, 6 and 7
#pragma GCC diagnostic error "-Wpedantic"
#endif

#if __GNUC__ >= 7 || defined (__clang__) && __clang_major__ >= 12

// gcc manual says all kind of /* fall.*through */ regexp's work too
// but perhaps only when cpp does not filter comments out. thus...
#define FALL_THROUGH __attribute__ ((fallthrough))
#else
#define FALL_THROUGH ((void)0)
#endif

#ifndef __cplusplus
#pragma GCC diagnostic error "-Wstrict-prototypes"
#pragma GCC diagnostic error "-Wbad-function-cast"
#pragma GCC diagnostic error "-Wold-style-definition"
#pragma GCC diagnostic error "-Wmissing-prototypes"
#pragma GCC diagnostic error "-Wnested-externs"
#endif

// -Wformat=2 ¡currently! (2020-11-11) equivalent of the following 4
#pragma GCC diagnostic error "-Wformat"
#pragma GCC diagnostic error "-Wformat-nonliteral"
#pragma GCC diagnostic error "-Wformat-security"
#pragma GCC diagnostic error "-Wformat-y2k"

#pragma GCC diagnostic error "-Winit-self"
#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic error "-Wpointer-arith"
#pragma GCC diagnostic error "-Wwrite-strings"
#pragma GCC diagnostic error "-Wcast-qual"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic error "-Wmissing-include-dirs"
#pragma GCC diagnostic error "-Wundef"

#ifndef __clang__ // XXX revisit -- tried with clang 3.8.0
#pragma GCC diagnostic error "-Wlogical-op"
#endif

#ifndef __cplusplus // supported by c++ compiler but perhaps not worth having
#pragma GCC diagnostic error "-Waggregate-return"
#endif

#pragma GCC diagnostic error "-Wmissing-declarations"
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Winline"
#pragma GCC diagnostic error "-Wvla"
#pragma GCC diagnostic error "-Woverlength-strings"
#pragma GCC diagnostic error "-Wuninitialized"

//ragma GCC diagnostic error "-Wfloat-equal"
//ragma GCC diagnostic error "-Werror"
//ragma GCC diagnostic error "-Wconversion"

// avoiding known problems (turning some errors set above to warnings)...
#if __GNUC__ == 4
#ifndef __clang__
#pragma GCC diagnostic warning "-Winline" // gcc 4.4.6 ...
#pragma GCC diagnostic warning "-Wuninitialized" // gcc 4.4.6, 4.8.5 ...
#endif
#endif

#endif /* defined (__GNUC__) */

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

static void print_nsd(const char * pfx, int nsd)
{
    char buf[64];
    //printf("%d\n", nsd);
    int v = snprintf(buf, sizeof buf, "%s: 0%s%d", pfx, nsp(nsd), nsd);
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
    fwrite(buf, v+2, 1, stdout);
}

#define LC 10

static void print_nsds(const char * ppfx, int * nsdiff)
{
    printf("%-10s", ppfx);
    print_nsd("first", nsdiff[0]);
    long long sum = 0;
    for (int lc = 1; lc < LC; lc++) {
        sum = sum + nsdiff[lc];
    }
    print_nsd(", rest avg", (int)(sum / (LC - 1)));
    printf("\n");
}

static inline int nsdiff_val(int ns1, int ns2) {
    int nsd = ns2 - ns1; if (nsd < 0) nsd += 1e9;
    return nsd;
}

int main(void)
{
    printf("view %s - run this multiple times - compare results\n",
           __FILE__);
    char buf[1024];
    int nsdiff[LC];
    for (int lc = 0; lc < LC; lc++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        nsdiff[lc] = nsdiff_val(tv1.tv_nsec, tv2.tv_nsec);
    }
    print_nsds("no-calls:", nsdiff);

    for (int lc = 0; lc < LC; lc++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        sched_getcpu();
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        nsdiff[lc] = nsdiff_val(tv1.tv_nsec, tv2.tv_nsec);
    }
    print_nsds("*_getcpu:", nsdiff);


    for (int lc = 0; lc < LC; lc++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        getpid();
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        nsdiff[lc] = nsdiff_val(tv1.tv_nsec, tv2.tv_nsec);
    }
    print_nsds("getpid:", nsdiff);

    for (int lc = 0; lc < LC; lc++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        getpid();
        getppid();
        getsid(0);
        getpgrp();
        getcwd(buf, sizeof buf);
        struct rusage rusage; (void)getrusage(RUSAGE_SELF, &rusage);
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        nsdiff[lc] = nsdiff_val(tv1.tv_nsec, tv2.tv_nsec);
    }
    print_nsds("6 get*:", nsdiff);

    for (int lc = 0; lc < LC; lc++) {
        struct timespec tv1; clock_gettime(CLOCK_REALTIME, &tv1);
        if (fork() == 0) exit(0);
        struct timespec tv2; clock_gettime(CLOCK_REALTIME, &tv2);
        nsdiff[lc] = nsdiff_val(tv1.tv_nsec, tv2.tv_nsec);
    }
    print_nsds("fork:", nsdiff);
    return 0;
}
