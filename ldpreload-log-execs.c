#if 0 /* -*- mode: c; c-file-style: "stroustrup"; tab-width: 8; -*-
 set -euf
 case ${1-} in --fd=*) fd=$((${1#*=} + 0)); shift ;; *) fd=975
 esac
 so=${0##*''/}; so=${so%.c}-$fd.so; test -e "$so" && rm "$so"
 test $# = 0 && set -- -O2
 set -x
 ${CC:-gcc} -std=c11 -shared -fPIC -o "$so" "$0" "$@" -DFD=$fd -ldl
 exec chmod 644 $so
 exit not reached
 */
#endif
/*
 * Created: Fri 02 Oct 2015 18:47:15 +0300 too
 * L.st modified: Fri 11 Nov 2016 21:38:42 +0200 too
 * L.st modified: Tue 25 Jan 2022 19:16:29 +0200 too
 * Last modified: Tue 24 Sep 2024 17:40:35 +0300 too
 */

/* SPDX-License-Identifier: BSD-2-Clause */

/* Nice test:
 *  $ sh ldpreload-log-execs.c --fd=1
 *  $ env -i LD_PRELOAD=./ldpreload-log-execs-1.so env env env true
 *
 * add LD_DEBUG=all to above to see what calls were not seen...
 * (first time it was useful to notice that wait3() was needed...)
 */

#if defined(__linux__) && __linux__ || defined(__CYGWIN__) && __CYGWIN__
// on linux: man feature_test_macros -- try ftm.c at the end of it
#define _DEFAULT_SOURCE 1
// for older glibc's on linux (< 2.19 -- e.g. rhel7 uses 2.17...)
#define _BSD_SOURCE 1
#define _SVID_SOURCE 1
#define _POSIX_C_SOURCE 200809L
#define _ATFILE_SOURCE 1
// more extensions (less portability?)
//#define _GNU_SOURCE 1
#endif

// hint: gcc -dM -E -xc /dev/null | grep -i gnuc
// also: clang -dM -E -xc /dev/null | grep -i gnuc
#if defined (__GNUC__)

#if 0 // use of -Wpadded gets complicated, 32 vs 64 bit systems
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
//ragma GCC diagnostic error "-Wconversion"

// avoiding known problems (turning some errors set above to warnings)...
#if __GNUC__ == 4
#ifndef __clang__
#pragma GCC diagnostic warning "-Winline" // gcc 4.4.6 ...
#pragma GCC diagnostic warning "-Wuninitialized" // gcc 4.4.6, 4.8.5 ...
#endif
#endif

#endif /* defined (__GNUC__) */


#define _GNU_SOURCE 1 // needed for sched_getcpu() (but must be up here...)

#define execve execve0

#define execv execv0
#define execvp execvp0
#define execvpe execvpe0

#define execl execl0
#define execlp execlp0
#define execle execle0

#define close close_hidden

#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sched.h>
//#include <sys/times.h>
#include <sys/resource.h>
#include <errno.h>

/* The "default" option -- USE_GETTIMEOFDAY 0 -- makes use of clock_gettime()
 * instead. Some older systems did not have the latter. Some systems had it
 * in 'rt' library, so e.g. LD_PRELOAD=/lib64/librt.so.1 may work there.
 * Thought of implementing clock_gettime() syscall in asm, but that is bad
 * idea -- clock_gettime() is usually not implemented as (slow) syscall...
 */
#define USE_GETTIMEOFDAY 0 // set to 1 if there is no clock_gettime()
#if USE_GETTIMEOFDAY
#include <sys/time.h>
#else
#include <time.h>
#endif

#undef execve

#undef execv
#undef execvp
#undef execvpe

#undef execl
#undef execlp
#undef execle

#undef close

/*
 * gcc -std=c11 -dM -xc -E /dev/null | grep STDC_V was used to get 201100L
 * gcc -std=c2x -dM -xc -E /dev/null | grep STDC_V was used to get 202000L
 */
#if !defined (__STDC_VERSION__) || __STDC_VERSION__ < 201100L
#undef static_assert
#define static_assert(x) do { } while 0
#elif __STDC_VERSION__ < 202000L
/* c23 (single argument) static assert to c11 (fyi: gcc9 knows this already) */
#undef static_assert
#define static_assert(x) _Static_assert(x, #x)
#endif

#if 0
// could be used to increase precision of nanosecond-resolution timestamps
// when loaded from json to system native formats. usually the overhead of
// this system makes the values imprecise enough, but for fun...
#define TSS(x) ((x) - 1640000000)
#else
#define TSS(x) (x)
#endif


#define HAVE_PID 1
#define HAVE_PPID 1
#define HAVE_PWD 1
#define HAVE_TIME 1
#define HAVE_CPU 1
#define HAVE_ARGS 1

// "pretty print" defs
#define PP_ARGS 0
#define PP_ENV 0

// block begin/end macros (for variable declarations) -- explicit liveness...
#define BB {
#define BE }

// w/ extern and no optimization we got function code (which is not called,
// though) w/ static or w/o neither no function code was emitted
// static inline was used in some header files...
#if __GNUC__ >= 4
#define always_inline static inline __attribute__ ((always_inline))
#else
#define always_inline static inline
#endif

// type checked casts
always_inline char * uc2sc_p(unsigned char * c) { return (char *)c; }
always_inline unsigned char * sc2uc_p(char * c) { return (unsigned char *)c; }
//#define uc2sc_p(c) (char *)(c) // uncomment and test diff with gcc -O2 -S ...
// end of type checked casts

always_inline unsigned char * lutoa(unsigned char * p, unsigned long u)
{
    unsigned long v = u;
    unsigned char * r = p;
    do {
	v /= 10;
	p++;
    } while (v > 0);
    r = p;
    *p-- = '\0';
    do {
	*p-- = (u % 10) + '0';
	u /= 10;
    } while (u > 0);
    return r;
}

static const char hex[16] = "0123456789abcdef"; // without trailing '\0'

static void abufs(unsigned char ** pp, int l, const unsigned char * s)
{
    unsigned char * p = *pp;
    while (l > 0) {
	unsigned char c = *s++;
	if (c == '\0')
	    break;
	if (c <= '"') { //  32 0x20 ' '  --  33 0x21 !  --  34 0x22 "
	    switch (c) {
	    case ' ': case '!': break;
	    case '"':  *p++ = '\\'; l--; break;
	    case 0x08: *p++ = '\\'; l--; c = 'b'; break;
	    case 0x09: *p++ = '\\'; l--; c = 't'; break;
	    case 0x0a: *p++ = '\\'; l--; c = 'n'; break;
	    case 0x0c: *p++ = '\\'; l--; c = 'f'; break;
	    case 0x0d: *p++ = '\\'; l--; c = 'r'; break;
	    default:
		*p++ = '\\'; *p++ = 'u'; *p++ = '0'; *p++ = '0';
		*p++ = hex[c >> 4]; *p++ = hex[c & 0xf]; l -= 6;
		continue;
	    }
	}
	else if (c > 0x7e) {
	    /*
	     * XXX: the invalid utf-8 code point check is not fully tested
	     */
	    if (c < 0xc2) { // not any utf8 first code point. skip. //
		if (c < 0xc0)
		    goto comma; // XXX, initially all were these...
		// else (0xc0 or 0xc1 - overlong) -- add U+FFFD
	    }
	    else if (c < 0xe0) { // potential 2-byte utf8-code //
		if (c == 0xc2 && s[0] < 0xa0)
		    ; // U+0080 - U+009f: invalid -- add U+FFFD
		else if ((s[0] & 0xc0) == 0x80) {
		    p[0] = c; p[1] = s[0];
		    s += 1; p += 2; l -= 2;
		    continue;
		}
	    }
	    else if (c < 0xf0) { // potential 3-byte utf8-code //
		if (c == 0xe0 // check less than e0 a0 80
		    && (s[0] < 0xa0)) // 0x80 is tested in next if
		    ; // overlong < U+0800 -- add U+FFFD
		// write(1, "quirk\n", 6); // move around when needed //
		else if ((s[0] & 0xc0) == 0x80 && (s[1] & 0xc0) == 0x80) {
		    p[0] = c; p[1] = s[0]; p[2] = s[1];
		    s += 2; p += 3; l -= 3;
		    continue;
		}
	    }
#if 1
	    else if (c < 0xf4) { // potential 4-byte utf8-code //
		if (c == 0xf0 // check less than f0 90 80 80
		    && (s[0] < 0x90)) // both 0x80's are tested in next if
		    ; // overlong < U+10000 -- add U+FFFD
		else if (   (s[0] & 0xc0) == 0x80 && (s[1] & 0xc0) == 0x80
			    && (s[2] & 0xc0) == 0x80) {
		    memcpy(p, s - 1, 4);
		    s += 3; p += 4; l -= 4;
		    continue;
		}
	    }

#else	    // vvv invalid according to https://en.wikipedia.org/wiki/UTF-8 vvv

	    else if (c < 0xf8) { // potential 4-byte utf8-code //
		if (   (s[0] & 0xc0) == 0x80 && (s[1] & 0xc0) == 0x80
		       && (s[2] & 0xc0) == 0x80) {
		    memcpy(p, s - 1, 4);
		    s += 3; p += 4; l -= 4;
		    continue;
		}
	    }
	    // XXX ditto
	    else if (c < 0xfc) { // potential 5-byte utf8-code //
		if (   (s[0] & 0xc0) == 0x80 && (s[1] & 0xc0) == 0x80
		       && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80) {
		    memcpy(p, s - 1, 5);
		    s += 4; p += 5; l -= 5;
		    continue;
		}
	    }
	    // XXX ditto
	    else if (c < 0xfe) { // potential 6-byte utf8-code //
		if (   (s[0] & 0xc0) == 0x80 && (s[1] & 0xc0) == 0x80
		       && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80
		       && (s[4] & 0xc0) == 0x80) {
		    memcpy(p, s - 1, 6);
		    s += 5; p += 6; l -= 6;
		    continue;
		}
	    }
#endif
	    // inject replacement character U+FFFD
	    p[0] = 0xef; p[1] = 0xbf; p[2] = 0xbd;
	    p += 3; l -= 3;
	    // skip "continuation" bytes; alternative would be repl. chars
	    while ((*s & 0xc0) == 0x80)
		s++;
	    continue;
	comma:
	    c = ',';
	}
	else if (c == '\\') {
	    *p++ = '\\'; l--;
	}
	// here in all cases above except when continue; was there
	*p++ = c; l--;
    }
    // *p = '\0';
    *pp = p;
}

#if 0
static inline unsigned char * chkpwd(unsigned char * pwd)
{
    while (*pwd) {
	if (*pwd == '\\' || *pwd == '"' || *pwd < 0x20 || *pwd > 0x7e)
	    *pwd = ',';
	pwd++;
    }
    return pwd;
}
#endif

// buffer size for output message. initially write getcwd() output to the
// same buffer so that if it is fully expanded (1 byte -> 6 bytes) will fit
// into resulting position. PATH_MAX is ~4096 so align offset based on that.
#define LBSIZE 32768
#define CWDOFF 28416 // 32768 - 28416 = 4352 (4096 + 256) (*6 = 26112)


/* used by constructor and destructor -- requires p and l from scope */
#define ABUFC(c) do { *p++ = (unsigned char)c; } while (0)
#define ABUF0(s) do {	int sl = sizeof (s) - 1; \
			memcpy(p, s, sl); p += sl; } while (0)
#define ABUF1(s) abufs(&p, LBSIZE - (p - buf) - 1024, s)

#if 1 // concatenated json
#define EOB ABUF0("\n }\n")
#else // previously had this...
#define EOB ABUF0("\n },\n")
#endif

#if HAVE_PID
#define MAY_ADD_PID ABUF0(",\n" "  \"pid\": "); p = lutoa(p, getpid())
#else
#define MAY_ADD_PID do {} while (0)
#endif
#if HAVE_PPID
#define MAY_ADD_PPID ABUF0(",\n" "  \"ppid\": "); p = lutoa(p, getppid())
#else
#define MAY_ADD_PPID do {} while (0)
#endif

#if HAVE_TIME
always_inline unsigned char * may_add_time(unsigned char * p)
{
#if USE_GETTIMEOFDAY
    struct timeval tv;
    if (gettimeofday(&tv, NULL) == 0)
#else
    struct timespec tv;
    if (clock_gettime(CLOCK_REALTIME, &tv) == 0)
#endif
    {
	p = lutoa(p, TSS(tv.tv_sec));
#if USE_GETTIMEOFDAY
	int u = tv.tv_usec; // u = 7;
	/**/ if (u >= 100000) ABUF0(".");
	else if (u >= 10000) ABUF0(".0");
	else if (u >= 1000) ABUF0(".00");
	else if (u >= 100) ABUF0(".000");
	else if (u >= 10) ABUF0(".0000");
	else /* ---- */  ABUF0(".00000");
#else
	int u = tv.tv_nsec; // u = 7;
	/**/ if (u >= 100000000) ABUF0(".");
	else if (u >= 10000000) ABUF0(".0");
	else if (u >= 1000000) ABUF0(".00");
	else if (u >= 100000) ABUF0(".000");
	else if (u >= 10000) ABUF0(".0000");
	else if (u >= 1000) ABUF0(".00000");
	else if (u >= 100) ABUF0(".000000");
	else if (u >= 10) ABUF0(".0000000");
	else /* ---- */  ABUF0(".00000000");
#endif
	p = lutoa(p, u);
    }
    else
	ABUF0("-1");
    return p;
}
#define MAY_ADD_TIME(t) ABUF0(",\n  \"" #t "\": "); p = may_add_time(p)
#else
#define MAY_ADD_TIME(t) do {} while (0)
#endif
#if HAVE_CPU
#define MAY_ADD_CPU(c) ABUF0(",\n  \"" #c "\": "); p = lutoa(p, sched_getcpu())
#else
#define MAY_ADD_CPU(c) do {} while (0)
#endif

#if HAVE_PWD
always_inline unsigned char * may_add_pwd(unsigned char * buf,
					  unsigned char * p)
{
    ABUF0(",\n"
	  "  \"pwd\": \"");
    if (getcwd(uc2sc_p(buf) + CWDOFF, LBSIZE - CWDOFF) != NULL) {
	ABUF1(buf + CWDOFF);
	ABUFC('"');
#if 0
	p = chkpwd(p);
#endif
    } else
	// this should always fit, though (PATH_MAX is like 4096)
	ABUF0("error: no buffer space for path");
    return p;
}
#define MAY_ADD_PWD p = may_add_pwd(buf, p)
#else
#define MAY_ADD_PWD do {} while (0)
#endif

#if 1
always_inline unsigned char *
insert_rusage(struct rusage rusage, unsigned char * p)
{
    ABUF0(",\n"
	  "  \"utime\": ");
    p = lutoa(p, rusage.ru_utime.tv_sec);
    int u = rusage.ru_utime.tv_usec;
    /**/ if (u >= 100000) ABUFC('.');
    else if (u >= 10000) ABUF0(".0");
    else if (u >= 1000) ABUF0(".00");
    else if (u >= 100) ABUF0(".000");
    else if (u >= 10) ABUF0(".0000");
    else /* ---- */  ABUF0(".00000");
    p = lutoa(p, u);
    ABUF0(",\n  \"stime\": ");
    p = lutoa(p, rusage.ru_stime.tv_sec);
    u = rusage.ru_stime.tv_usec;
    /**/ if (u >= 100000) ABUFC('.');
    else if (u >= 10000) ABUF0(".0");
    else if (u >= 1000) ABUF0(".00");
    else if (u >= 100) ABUF0(".000");
    else if (u >= 10) ABUF0(".0000");
    else /* ---- */  ABUF0(".00000");
    p = lutoa(p, u);
    ABUF0(",\n  \"maxrss\": ");  p = lutoa(p, rusage.ru_maxrss);
    ABUF0(",\n  \"minflt\": ");  p = lutoa(p, rusage.ru_minflt);
    ABUF0(",\n  \"majflt\": ");  p = lutoa(p, rusage.ru_majflt);
    ABUF0(",\n  \"inblock\": "); p = lutoa(p, rusage.ru_inblock);
    ABUF0(",\n  \"oublock\": "); p = lutoa(p, rusage.ru_oublock);
    ABUF0(",\n  \"nvcsw\": ");   p = lutoa(p, rusage.ru_nvcsw);
    ABUF0(",\n  \"nivcsw\": ");  p = lutoa(p, rusage.ru_nivcsw);
    return p;
}

__attribute__((destructor))
static void dfn(void/*int ev*/) // ev is wild guess -- which did not work !!!
{
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"destructor\"");
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
#if 0
    ABUF0("  \"exit\": ");
    p = lutoa(p, ev);
    ABUF0(",\n");
#endif
    BB;
#if 0
    struct tms tms;
    (void)times(&tms);
    unsigned char * q;
    p = lutoa(p,tms.tms_utime);   ABUFC(' ');
    p = lutoa(p,tms.tms_stime);   ABUFC(' ');
    p = lutoa(p,tms.tms_cutime);  ABUFC(' ');
    p = lutoa(p,tms.tms_cstime);  ABUFC(' ');
#else
    struct rusage rusage;
    (void)getrusage(RUSAGE_SELF, &rusage);
    p = insert_rusage(rusage, p);
#endif
    BE;
    EOB;

    (void)!write(FD, buf, p - buf);
}
#endif

// wrappers part //

#include <dlfcn.h>
#include <stdlib.h>

#define null ((void*)0)

static void * dlsym_next(const char * symbol)
{
    void * sym = dlsym(RTLD_NEXT, symbol);
    char * str = dlerror();

    if (str != null)
	exit(77);
	//die("finding symbol '%s' failed: %s", symbol, str);

    return sym;
}

// Macros FTW! -- use gcc -E to examine expansion (or clang -E ...)

#define _deffn(_rt, _fn, _args) \
_rt _fn _args; \
_rt _fn _args { \
    static _rt (*_fn##_next) _args = null; \
    if (! _fn##_next ) *(void**) (&_fn##_next) = dlsym_next(#_fn);


_deffn ( int, close, (int fd) )
#if 0
{
#endif
    if (fd == 975) return 0;
    return close_next(fd);
}

_deffn ( int, execve, (char * path, char ** const argv, char ** const envp) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execve\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;

    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    char ** av = argv;
    unsigned char * pp;
    for (int cmm = 0; *av; av++) {
	if (cmm)
	    ABUF0(",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(sc2uc_p(*av));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    int rv = execve_next(path, argv, envp);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execve\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

_deffn ( int, execv, (char * path, char ** const argv) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execv\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    char ** av = argv;
    unsigned char * pp;
    for (int cmm = 0; *av; av++) {
	if (cmm)
	    ABUF0(",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(sc2uc_p(*av));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    int rv = execv_next(path, argv);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execv\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

_deffn ( int, execvp, (char * path, char ** const argv) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execvp\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    char ** av = argv;
    unsigned char * pp;
    for (int cmm = 0; *av; av++) {
	if (cmm)
	    ABUF0(",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(sc2uc_p(*av));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    int rv = execvp_next(path, argv);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execvp\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

_deffn ( int, execvpe, (char * path, char ** const argv, char ** const envp) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execvpe\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    char ** av = argv;
    unsigned char * pp;
    for (int cmm = 0; *av; av++) {
	if (cmm)
	    ABUF0(",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(sc2uc_p(*av));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    int rv = execvpe_next(path, argv, envp);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execvpe\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

#define _nextfn(_rt, _fn, _args) \
    static _rt (*_fn##_next) _args = null; \
    if (! _fn##_next ) *(void**) (&_fn##_next) = dlsym_next(#_fn)


int execl (char * path, char * arg, ...);
int execl (char * path, char * arg, ...)
{
    _nextfn( int, execv, (const char * pathname, char *const argv[]) );

    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    //static_assert(((intptr_t)buf & 7) == 0);

    ABUF0(" {\n"
	  "  \"fn\": \"execl\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    ABUFC('"');
    ABUF1(sc2uc_p(arg));
    ABUFC('"');
    va_list ap;
    va_start(ap, arg);
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	ABUF0(",\n   \"");
	unsigned char * pp = p;
	ABUF1(sc2uc_p(a));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    va_end(ap);
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    va_list ap;
    va_start(ap, arg);
    char ** av = (char **)buf;
    *av++ = arg;
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	*av++ = a;
	// fixme: log (common fn?)
	if ((unsigned char *)av - buf > CWDOFF) _exit(111);
    }
    va_end(ap);
    *av = NULL;
    int rv = execv_next(path, (char **)buf);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execl\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

int execlp (char * path, char * arg, ...);
int execlp (char * path, char * arg, ...)
{
    _nextfn( int, execvp, (const char * pathname, char *const argv[]) );

    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    //static_assert(((intptr_t)buf & 7) == 0);

    ABUF0(" {\n"
	  "  \"fn\": \"execlp\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    ABUFC('"');
    ABUF1(sc2uc_p(arg));
    ABUFC('"');
    va_list ap;
    va_start(ap, arg);
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	ABUF0(",\n   \"");
	unsigned char * pp = p;
	ABUF1(sc2uc_p(a));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    va_end(ap);
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    va_list ap;
    va_start(ap, arg);
    char ** av = (char **)buf;
    *av++ = arg;
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	*av++ = a;
	// fixme: log (common fn?)
	if ((unsigned char *)av - buf > CWDOFF) _exit(111);
    }
    va_end(ap);
    *av = NULL;
    int rv = execvp_next(path, (char **)buf);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execlp\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}

int execle (char * path, char * arg, ...);
int execle (char * path, char * arg, ...)
{
    _nextfn( int, execvpe, (const char * pathname,
			    char *const argv[], char *const envp[]) );

    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    //static_assert(((intptr_t)buf & 7) == 0);

    ABUF0(" {\n"
	  "  \"fn\": \"execle\",\n"
	  "  \"pathname\": \"");
    ABUF1(sc2uc_p(path));
    ABUFC('"');
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF0("\n   ");
    ABUFC('"');
    ABUF1(sc2uc_p(arg));
    ABUFC('"');
    va_list ap;
    va_start(ap, arg);
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	ABUF0(",\n   \"");
	unsigned char * pp = p;
	ABUF1(sc2uc_p(a));
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    va_end(ap);
    BE;
    ABUF0("\n  ]");
    EOB;

    (void)!write(FD, buf, p - buf);

    va_list ap;
    va_start(ap, arg);
    char ** av = (char **)buf;
    *av++ = arg;
    for (char * a = va_arg(ap, char *); a; a = va_arg(ap, char *)) {
	*av++ = a;
	// fixme: log (common fn?)
	if ((unsigned char *)av - buf > CWDOFF) _exit(111);
    }
    char * const * envp = va_arg(ap, char * const *);
    va_end(ap);
    *av = NULL;
    int rv = execvpe_next(path, (char **)buf, envp);

    p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"execle\",\n"
	  "  \"pathname\": \":failed:\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    ABUF0(",\n" "  \"errno\": ");
    p = lutoa(p, errno);
    MAY_ADD_CPU(cpu);  // probably always same, let's follow...
    EOB;

    (void)!write(FD, buf, p - buf);

    return rv;
}
