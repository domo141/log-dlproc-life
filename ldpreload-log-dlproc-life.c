#if 0 /*
 bn=${0##*''/}; bn=${bn%.c} # instead of bn=`basename "$0" .c`
 case ${1-} in	( [0-9] | [1-9]*[0-9] ) fd=$(($1 + 0)) ;; # OK
		( * ) echo "to compile: sh $0 {fd}" >&2; exit 1 ;; esac
 so=$bn-$fd.so
 set -xeuf
 test -f $so && rm $so
 gcc -O2 -std=c99 -shared -fPIC -s -o $so "$0" -DFD=$fd -ldl
 exec chmod 644 $so
 exit
*/
#endif
/*
 * Created: Fri 02 Oct 2015 18:47:15 +0300 too
 * L.st modified: Fri 11 Nov 2016 21:38:42 +0200 too
 * Last modified: Tue 25 Jan 2022 19:16:29 +0200 too
 */

/* SPDX-License-Identifier: BSD-2-Clause */

/* Nice test:
 *  $ sh ldpreload-log-dlproc-life.c 1
 *  $ env -i LD_PRELOAD=./ldpreload-log-dlproc-life-1.so env env env true
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


#define _GNU_SOURCE 1 // needed for sched_getcpu() (but must be up here...)

#define fork fork0
#define execve execve0
#define wait wait0
#define waitpid waitpid0
#define wait3 wait30
#define wait4 wait40
#define write write0 // not wrapped but to avoid warning

#include <unistd.h>
#include <string.h>
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

#undef fork
#undef execve
#undef wait
#undef waitpid
#undef wait3
#undef wait4
#undef write
ssize_t write(int fd, const void * buf, size_t count);

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
#define HAVE_SID 1
#define HAVE_PGRP 1
#define HAVE_PWD 1
#define HAVE_TIME 1
#define HAVE_CPU 1
#define HAVE_ARGS 1
#define HAVE_ENV 1

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
#if HAVE_SID
#define MAY_ADD_SID ABUF0(",\n" "  \"sid\": "); p = lutoa(p, getsid(0))
#else
#define MAY_ADD_SID do {} while (0)
#endif
#if HAVE_PGRP
#define MAY_ADD_PGRP ABUF0(",\n" "  \"pgrp\": "); p = lutoa(p, getpgrp())
#else
#define MAY_ADD_PGRP do {} while (0)
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

__attribute__((constructor))
static void fn(int argc, unsigned char ** argv, unsigned char ** envp)
{
#if ! HAVE_ARGS
    (void)argc; (void)argv;
#endif
#if ! HAVE_ENV
    (void)envp;
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"constructor\"");
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_SID;
    MAY_ADD_PGRP;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;

#if HAVE_ARGS
#undef ABUF00
#if PP_ARGS
#define ABUF00(nil, s) ABUF0(s)
#else
#define ABUF00(s, nil) ABUF0(s)
#endif
    BB;
    ABUF0(",\n"
	  "  \"args\": [");
    ABUF00(" ", "\n   ");
    unsigned char * pp;
    for (int i = 0, cmm = 0; i < argc; i++) {
	if (cmm)
	    ABUF00(", \"", ",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(argv[i]);
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("...\"");
	    break;
	}
	ABUFC('"');
    }
    BE;
    ABUF00(" ]", "\n  ]");
#endif
#if HAVE_ENV
#undef ABUF00
#if PP_ENV
#define ABUF00(nil, s) ABUF0(s)
#else
#define ABUF00(s, nil) ABUF0(s)
#endif
    ABUF0(",\n"
	  "  \"env\": [");
    ABUF00(" ", "\n   ");
    BB;
    unsigned char * pp;
    for (int i = 0, cmm = 0; envp[i]; i++) {
	//if (envp[i][0] != 'Z') continue;
	if (cmm)
	    ABUF00(", \"", ",\n   \"");
	else {
	    ABUFC('"');
	    cmm = 1;
	}
	pp = p;
	ABUF1(envp[i]);
	if (sizeof (buf) - (p - buf) < 1040) {
	    p = pp;
	    ABUF0("BUFFER_FULL=\"t\"");
	    break;
	}
	ABUFC('"');
	//if (i == 3) break;
    }
    BE;
    ABUF00(" ]", "\n  ]");
#endif
    EOB;

    (void)write(FD, buf, p - buf);
}

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
    MAY_ADD_SID;
    MAY_ADD_PGRP;
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

    (void)write(FD, buf, p - buf);
}

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


_deffn ( pid_t, fork, (void) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"fork\",\n"
	  "  \"where\": \"parent\"");
    MAY_ADD_PID;
    MAY_ADD_PPID;
    MAY_ADD_SID;
    MAY_ADD_PGRP;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;

    // note that the libc fork syscall wrapper may call e.g. clone(2)
    pid_t rv = fork_next();

    if (rv) {
	MAY_ADD_TIME(time_after);
	MAY_ADD_CPU(cpu_after);
	ABUF0(",\n" "  \"child\": "); p = lutoa(p, rv);
    }
    else {
	p = buf;

	ABUF0(" {\n"
	      "  \"fn\": \"fork\",\n"
	      "  \"where\": \"child\"");
	MAY_ADD_PID;
	MAY_ADD_PPID;
	MAY_ADD_TIME(time);
	MAY_ADD_CPU(cpu);
    }
    // env?
    EOB;

    (void)write(FD, buf, p - buf);
    return rv;
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
    MAY_ADD_SID;
    MAY_ADD_PGRP;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);
    MAY_ADD_PWD;
    // env ?
    EOB;

    (void)write(FD, buf, p - buf);

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

    (void)write(FD, buf, p - buf);

    return rv;
}

/*
_deffn ( int, clone,
	 (int (*fn)(void *), void *stack, int flags, void *arg, ... ) )
#if 0
{
#endif
    va_list ap;

    va_start(ap, arg);
    pid_t ptid = va_arg(ap, pid_t);
    void * tls = va_arg(ap, tls);
    pid_t ctid = va_arg(ap, pid_t);
    int pid = clone_next(fn, stack, flags, arg, ptid, tls, ctid);
    va_end(ap);
    return pid;
*/


pid_t wait(int * wstatus);
pid_t wait(int * wstatus)
{
    static pid_t (*wait3_next)(int * wstatus, int options,
			       struct rusage * rusage) = null;
    if (wait3_next == null) *(void**)&wait3_next = dlsym_next("wait3");

    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"wait\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);

    struct rusage rusage;
    pid_t rv = wait3_next(wstatus, 0, &rusage);
    if (rv <= 0) return rv;

    MAY_ADD_TIME(time_after);
    MAY_ADD_CPU(cpu_after);
    ABUF0(",\n" "  \"ret\": "); p = lutoa(p, rv);
    p = insert_rusage(rusage, p);
    MAY_ADD_PWD;
    EOB;

    (void)write(FD, buf, p - buf);

    return rv;
}

pid_t waitpid (pid_t pid, int * wstatus, int options);
pid_t waitpid (pid_t pid, int * wstatus, int options)
{
    static pid_t (*wait4_next)(pid_t pid, int * wstatus, int options,
			       struct rusage * rusage) = null;
    if (wait4_next == null) *(void**)&wait4_next = dlsym_next("wait4");

    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"waitpid\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);

    struct rusage rusage;
    pid_t rv = wait4_next(pid, wstatus, options, &rusage);
    if (rv <= 0) return rv;

    MAY_ADD_TIME(time_after);
    MAY_ADD_CPU(cpu_after);
    ABUF0(",\n" "  \"ret\": "); p = lutoa(p, rv);
    p = insert_rusage(rusage, p);
    MAY_ADD_PWD;
    EOB;

    (void)write(FD, buf, p - buf);

    return rv;
}

_deffn ( pid_t, wait4, (pid_t pid, int * wstatus, int options,
			struct rusage * rusage) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"wait4\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);

    struct rusage rusage_d;
    if (rusage == null) rusage = &rusage_d;

    pid_t rv = wait4_next(pid, wstatus, options, rusage);
    if (rv <= 0) return rv;

    MAY_ADD_TIME(time_after);
    MAY_ADD_CPU(cpu_after);
    ABUF0(",\n" "  \"ret\": "); p = lutoa(p, rv);
    p = insert_rusage(*rusage, p);
    MAY_ADD_PWD;
    EOB;

    (void)write(FD, buf, p - buf);

    return rv;
}

_deffn ( pid_t, wait3, (int * wstatus, int options, struct rusage * rusage) )
#if 0
{
#endif
    unsigned char buf[LBSIZE];
    unsigned char * p = buf;

    ABUF0(" {\n"
	  "  \"fn\": \"wait3\"");
    MAY_ADD_PID;
    MAY_ADD_TIME(time);
    MAY_ADD_CPU(cpu);

    struct rusage rusage_d;
    if (rusage == null) rusage = &rusage_d;

    pid_t rv = wait3_next(wstatus, options, rusage);
    if (rv <= 0) return rv;

    MAY_ADD_TIME(time_after);
    MAY_ADD_CPU(cpu_after);
    ABUF0(",\n" "  \"ret\": "); p = lutoa(p, rv);
    p = insert_rusage(*rusage, p);
    MAY_ADD_PWD;
    EOB;

    (void)write(FD, buf, p - buf);

    return rv;
}
