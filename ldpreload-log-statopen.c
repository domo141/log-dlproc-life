#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf
 trg=${0##*''/}; trg=${trg%.c}-973.so; test ! -e "$trg" || rm "$trg"
 case ${1-} in '') set x -O2; shift; esac
 #case ${1-} in '') set x -ggdb; shift; esac
 x_exec () { printf %s\\n "$*" >&2; exec "$@"; }
 x_exec ${CC:-gcc} -std=c11 -shared -fPIC -o "$trg" "$0" $@ -ldl
 exit $?
 */
#endif
/*
 * $ ldpreload-statopen.c $
 *
 * Created: Thu 14 Dec 2017 08:48:34 EET too (ldpreload-vsfa.c in ioiomxtx)
 * L.st modified: Sat 02 Apr 2022 16:51:21 +0300 too
 * Last modified: Thu 19 Jan 2023 21:06:14 +0200 too
 */

/* log some *stat*() and *open*() calls
 * -- to see which files were accessed; inotify (and fanotify)
 * \-  don't "see" stat() calls
 * wrap close() too, often avoid closing fd 973 when called...
*/

// gcc -dM -E -xc /dev/null | grep -i gnuc
// clang -dM -E -xc /dev/null | grep -i gnuc
#if defined (__GNUC__)

// to relax, change 'error' to 'warning' -- or even 'ignored'
// selectively. use #pragma GCC diagnostic push/pop to change
// the rules temporarily

#if 0 // use of -Wpadded gets complicated, 32 vs 64 bit systems
#pragma GCC diagnostic warning "-Wpadded"
#endif

#pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wextra"

#if __GNUC__ >= 8 // impractically strict in gccs 5, 6 and 7
#pragma GCC diagnostic error "-Wpedantic"
#endif

#if __GNUC__ >= 7 || defined (__clang__) && __clang_major__ >= 12

// gcc manual says all kind of /* fall.*through * / regexp's work too
// but perhaps only when cpp does not filter comments out. thus...
#define FALL_THROUGH __attribute__ ((fallthrough))
#else
#define FALL_THROUGH ((void)0)
#endif

#pragma GCC diagnostic error "-Wstrict-prototypes"
#pragma GCC diagnostic error "-Winit-self"

// -Wformat=2 ¡currently! (2017-12) equivalent of the following 4
#pragma GCC diagnostic warning "-Wformat"
#pragma GCC diagnostic warning "-Wformat-nonliteral" // XXX ...
#pragma GCC diagnostic error "-Wformat-security"
#pragma GCC diagnostic error "-Wformat-y2k"

#pragma GCC diagnostic error "-Wcast-align"
#pragma GCC diagnostic error "-Wpointer-arith"
#pragma GCC diagnostic error "-Wwrite-strings"
#pragma GCC diagnostic error "-Wcast-qual"
#pragma GCC diagnostic error "-Wshadow"
#pragma GCC diagnostic error "-Wmissing-include-dirs"
#pragma GCC diagnostic error "-Wundef"
#pragma GCC diagnostic error "-Wbad-function-cast"
#ifndef __clang__
#pragma GCC diagnostic error "-Wlogical-op" // XXX ...
#endif
#pragma GCC diagnostic error "-Waggregate-return"
#pragma GCC diagnostic error "-Wold-style-definition"
#pragma GCC diagnostic error "-Wmissing-prototypes"
#pragma GCC diagnostic error "-Wmissing-declarations"
#pragma GCC diagnostic error "-Wredundant-decls"
#pragma GCC diagnostic error "-Wnested-externs"
#pragma GCC diagnostic error "-Winline"
#pragma GCC diagnostic error "-Wvla"
#pragma GCC diagnostic error "-Woverlength-strings"

//ragma GCC diagnostic error "-Wfloat-equal"
//ragma GCC diagnostic error "-Werror"
//ragma GCC diagnostic error "-Wconversion"

#endif /* defined (__GNUC__) */

#if defined(__linux__) && __linux__
#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#define _ATFILE_SOURCE
#endif

#define open open_hidden
#define open64 open64_hidden
#define openat openat_hidden
#define openat64 openat64_hidden
#define open open_hidden
#define fopen fopen_hidden
#define opendir opendir_hidden
#define stat stat_x // struct stat... //
#define lstat stat_hidden
#define __xstat __xstat_hidden
#define __xstat64 __xstat64_hidden
#define __lxstat __lxstat_hidden
#define __lxstat64 __lxstat64_hidden
#define access access_hidden
#define close close_hidden

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h> // offsetof()
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
//#include <.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <dlfcn.h>

#undef open
#undef open64
#undef openat
#undef openat64
#undef fopen
#undef opendir
#undef stat
#undef lstat
#undef __xstat
#undef __xstat64
#undef __lxstat
#undef __lxstat64
#undef access
#undef close

static void pfdwrite(const char * fn, const char * fname)
{
    char buf[4096];
    int l;
    pid_t pid = getpid();
    if (fname[0] == '/') {
	l = snprintf(buf, sizeof buf, "%d: %s: %s\n", pid, fn, fname);
    }
    else {
	char * p = getcwd(buf + (sizeof buf / 2), sizeof buf / 2);
	l = snprintf(buf, sizeof buf, "%d: %s: %s/%s\n", pid, fn, p, fname);
    }
    if (l > (int)sizeof buf) l = (int)sizeof buf;
    // use log-statopen-973.bash to make this fd //
    (void)!write(973, buf, l);
}

static void pfdwrite2(const char * fn, const char * name, const char * arg1)
{
    char buf[4096];
    int l;
    pid_t pid = getpid();
    l = snprintf(buf, sizeof buf, "%d: %s: %s %s\n", pid, fn, name, arg1);
    if (l > (int)sizeof buf) l = (int)sizeof buf;
    // use log-statopen-973.bash to make this fd //
    (void)!write(973, buf, l);
}

__attribute__((constructor))
static void fn(int argc, const char ** argv /*, unsigned char ** envp*/)
{
    (void)argc;
    pfdwrite2("/exec/", argv[0], argv[1]);
}


static void * dlsym_next(const char * symbol)
{
    void * sym = dlsym(RTLD_NEXT, symbol);
    char * str = dlerror();

    if (str != NULL) {
	fprintf(stderr, "finding symbol '%s' failed: %s", symbol, str);
	exit(1);
    }
    return sym;
}

// Macros FTW! -- use gcc -E to examine expansion

#define _deffn(_rt, _fn, _args) \
_rt _fn _args; \
_rt _fn _args { \
    static _rt (*_fn##_next) _args = NULL; \
    if (! _fn##_next ) *(void**) (&_fn##_next) = dlsym_next(#_fn); \
    const char * fn = #_fn;


#define awrite(fname) pfdwrite(fn, fname)

#if 0
#define cprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define cprintf(...) do {} while (0)
#endif

_deffn ( int, close, (int fd) )
#if 0
{
#endif
    // awrite(pathname); replace w/ something
    (void)fn;
    cprintf("*** close(%d)\n", fd);
    if (fd == 973) return 0;
    return close_next(fd);
}

_deffn ( int, open, (const char * pathname, int flags, mode_t mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** open(\"%s\", %x, %o)\n", pathname, flags, mode);
    return open_next(pathname, flags, mode);
}

_deffn ( int, open64, (const char * pathname, int flags, mode_t mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** open64(\"%s\", %x, %o)\n", pathname, flags, mode);
    return open64_next(pathname, flags, mode);
}

_deffn ( int, openat, (int dirfd, const char * pathname, int flags, mode_t mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** openat(%d, \"%s\", %x, %o)\n", dirfd, pathname, flags, mode);
    return openat_next(dirfd, pathname, flags, mode);
}

_deffn ( int, openat64, (int dirfd, const char * pathname, int flags, mode_t mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** openat64(%d, \"%s\", %x, %o)\n", dirfd, pathname, flags, mode);
    return openat64_next(dirfd, pathname, flags, mode);
}

_deffn ( FILE *, fopen, (const char * pathname, const char * mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** fopen(\"%s\", \"%s\")\n", pathname, mode);
    return fopen_next(pathname, mode);
}

_deffn ( DIR *, opendir, (const char * name) )
#if 0
{
#endif
    awrite(name);
    cprintf("*** opendir(\"%s\")\n", name);
    return opendir_next(name);
}

#if 1
_deffn ( int, stat, (const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** stat(\"%s\", \"%p\")\n", pathname, (void*)statbuf);
    return stat_next(pathname, statbuf);
}

_deffn ( int, lstat, (const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** lstat(\"%s\", \"%p\")\n", pathname, (void*)statbuf);
    return lstat_next(pathname, statbuf);
}
#endif

_deffn ( int, __xstat, (int v, const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** __xstat(%d, \"%s\", \"%p\")\n", v, pathname, (void*)statbuf);
    return __xstat_next(v, pathname, statbuf);
}

_deffn ( int, __xstat64, (int v, const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** __xstat64(%d, \"%s\", \"%p\")\n", v, pathname, (void*)statbuf);
    return __xstat64_next(v, pathname, statbuf);
}

// fake lstat*s(), same code as in stat*s()
_deffn ( int, __lxstat, (int v, const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** __lxstat(%d, \"%s\", \"%p\")\n", v, pathname, (void*)statbuf);
    return __lxstat_next(v, pathname, statbuf);
}

_deffn ( int, __lxstat64, (int v, const char * pathname, struct stat_x * statbuf) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** __lxstat64(%d, \"%s\", \"%p\")\n", v, pathname, (void*)statbuf);
    return __lxstat64_next(v, pathname, statbuf);
}

_deffn ( int, access, (const char * pathname, int mode) )
#if 0
{
#endif
    awrite(pathname);
    cprintf("*** access(\"%s\", %x)\n", pathname, mode);
    return access_next(pathname, mode);
}
