/*******************************************************************************
	Copyright (C), 2011-2014, Andrew Min Chang

	File name: 	AMCCommonLib.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide some covenient tool in calling library tools. One can easily include 
		library headers he wants by declaring the corresponding macros. 
			I hope this file is not only a header, but also a useful Linux library note.
			
	History:
		2012-??-??: On about come date around middle of Year 2012, file created as "commonLib.h"
		2012-08-20: Till this day, library declarations below are completed:
						CFG_LIB_STDOUT,
						CFG_LIB_ERROUT,
						CFG_LIB_STDIN,
						CFG_LIB_SLEEP,
						CFG_LIB_STRING,
						CFG_LIB_MEM,
						CFG_LIB_FILE,
						CFG_LIB_DEVICE,
						CFG_LIB_ERRNO,
						CFG_LIB_GETOPT,
						CFG_LIB_GETOPT_LONG,
						CFG_LIB_ENV,
						CFG_LIB_TIME,
						CFG_LIB_FORK,
						CFG_LIB_PID,
						CFG_LIB_SIGNAL,
						CFG_LIB_ENV,
						CFG_LIB_THREAD,
						CFG_LIB_RAND,
						CFG_LIB_MALLOC,
						CFG_LIB_PIPE,
						CFG_LIB_SYSTEM,
						CFG_LIB_FIFO,
						CFB_LIB_MINCORE,
						CFG_LIB_SEM,
						CFG_LIB_SHARED_MEM
		2012-08-20: Add shared memory library; add message queue.
		2012-08-21: Add socket library (local)
		2012-08-22: Add math library
		2012-08-23: Add socket library (internet)
		2012-08-24: Add daemon function
		2012-10-10: Change file name as "AMCCommonLib.h"
		2012-12-04: Add UDP support in AMC socket library
		2013-01-07: Add basic data type such as "sint8_t"
		2013-01-18: Add CFG_LIB_STR_NUM.
		2013-01-22: Add CFG_LIB_TIMER.
		2013-01-22: Remove CFG_LIB_DATA_TYPE because there is already AMCDataTypes.h
		2014-08-28: Add CFG_LIB_MMAP
		2014-09-18: Add CFG_LIB_INOTIFY and CFG_LIB_KQUEUE
		2014-09-22: Add header <strings.h> in CFG_LIB_MEM
		2014-09-22: Add CFG_LIB_DNS
		2015-07-29: Add spinlock and POSIX semaphore.	
		2015-10-19: Add poll(), epoll(), select() functions (CFG_LIB_SELECT)

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_COMMON_LIB
#define	_AMC_COMMON_LIB

#include "AMCDataTypes.h"

/* This is a common file */
/* define operating system type */
#define	OS_TYPE_LINUX			0
#define	OS_TYPE_MAC_OS_X		1
#define	OS_TYPE_IPHONE		1

/* Apple */
#if defined(__APPLE__) && defined(__MACH__)
	/* Apple OSX and iOS (Darwin). ------------------------------ */
#include <TargetConditionals.h>
#if TARGET_IPHONE_SIMULATOR == 1	/* iOS in Xcode simulator */
#define	CFG_OS_TYPE		OS_TYPE_IPHONE
#elif TARGET_OS_IPHONE == 1		/* iOS on iPhone, iPad, etc. */
#define	CFG_OS_TYPE		OS_TYPE_IPHONE
#elif TARGET_OS_MAC == 1	/* OSX */
#define CFG_OS_TYPE		OS_TYPE_MAC_OS_X
#else
#define	CFG_OS_TYPE		OS_TYPE_IPHONE
#endif

#else	// ENDS: #if defined(__APPLE__) && defined(__MACH__)
/* Other */
#define	CFG_OS_TYPE		OS_TYPE_LINUX
#endif

/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to study binary search tree"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"


/*******************************************************
 * library config
 */
#define _GNU_SOURCE


/* macros that should only be defined once finished */
/* pre-instructions below should be safe */
/* This file allows multiple include */

/*******************************************************
 * some program constance
 */
#ifndef	UNTIL
#define		UNTIL(condition)	while(!(condition))
#endif

#ifndef	__FILE__
#define	__FILE__	"some file"
#endif

#ifndef	__LINE__
#define	__LINE__	(-1)
#endif

#ifndef	__FUNC__
#if	defined(__func__)
#define	__FUNC__	__func__
#elif	defined(__function__)
#define	__FUNC__	__function__
#elif	defined(__FUNCTION__)
#define	__FUNC__	__FUNCTION__
#else
#define	__FUNC__	"function in "__FILE__
#endif
#endif


#ifdef	DEBUG
#define	DB(x)	x
#define	TEST_VALUE(x)		AMCPrintf("##"__FILE__", %d: "#x" = 0x%x = %d\n", __LINE__, (unsigned int)x, x)
#define	TEST_64B_VALUE(x)	AMCPrintf("##"__FILE__", %d: "#x" = 0x%Lx = %Ld\n", __LINE__, x, x)
#define	TEST_CHAR(x)		AMCPrintf("##"__FILE__", %d: "#x" is '%c'\n", __LINE__, x)
#define	TEST_STR(x)		AMCPrintf("##"__FILE__", %d: "#x" is \"%s\"\n", __LINE__, x)
#define	TODO(x)			AMCPrintf("##"__FILE__", %d: TODO: %s\n",__LINE__, x)
#define	MARK()		AMCPrintf("<<MARK>> "__FILE__", %d\n", __LINE__)
#define	FUNC_MARK()	AMCPrintf("<<MARK>>"__FUNC__", %d\n", __LINE__)
#else
#define	DB(x)
#define	TEST_VALUE(x)
#define	TEST_64B_VALUE(x)
#define	TEST_CHAR(x)
#define	TEST_STR(x)
#define	TODO(x)			AMCPrintf("!!!"__FILE__" %d TODO: %s\n", x)
#define	MARK()
#define	FUNC_MARK()
#endif

#ifdef DEBUG
#define AMCLog(fmt, args...)	AMCPrintf("##"__FILE__", %d: "fmt, __LINE__, ##args)
#else
#define AMCLog(fmt, args...)	AMCPrintf(fmt, ##args)
#endif


#ifndef	FALSE
#define	FALSE	(0)
#define	TRUE	(!FALSE)
#endif

#ifndef	NULL
#define	NULL	((void *)0)
#endif

//#if (OS_TYPE_LINUX == CFG_OS_TYPE)
#ifndef	BOOL
#define	BOOL	int
#endif
//#endif

#ifndef	LENOF
#define	LENOF(x)	(sizeof(x) / sizeof(*(x)))
#endif

#ifndef OFFSET_OF
#ifdef offsetof
#define OFFSET_OF offsetof
#else
#define OFFSET_OF(TYPE, MEMBER)  ((size_t) &(((TYPE) *)0)->(MEMBER))
#endif
#endif


#ifndef FORCED_TYPE_CONVERT_MACROS
#define FORCED_TYPE_CONVERT_MACROS
#define CHAR_CVRT(c)	((char)(c))
#define UCHAR_CVRT(c)	((unsigned char)(c))
#define SHORT_CVRT(s)	((short)(s))
#define USHORT_CVRT(s)	((unsigned short)(s))
#define INT_CVRT(i)		((int)(i))
#define UINT_CVRT(i)	((unsigned int)(i))
#define LONG_CVRT(l)	((long)(l))
#define ULONG_CVRT(l)	((unsigned long)(l))
#define LONG64_CVRT(ll)	((long long)(ll))
#define ULONG64_CVRT(ll)	((unsigned long long){ll})
#define SIZE_T_CVRT(s)	((size_t)(s))
#define SSIZE_T_CVRT(s)	((ssize_t)(s))
#define VOID_P_CVRT(p)	((void *)(p))
#endif


/*******************************************************
 * Here shows how to define macros

#define	CFG_DECLARE_LIB_FUNC		declare library functions to make editor recognize them
*/

#ifdef	CFG_LIB_ALL
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_STDIN
#define	CFG_LIB_SLEEP
#define	CFG_LIB_STRING
#define	CFG_LIB_MEM
#define	CFG_LIB_FILE
#define	CFG_LIB_DEVICE
#define	CFG_LIB_ERRNO
#define	CFG_LIB_GETOPT
#define	CFG_LIB_GETOPT_LONG
#define	CFG_LIB_ENV
#define	CFG_LIB_TIME
#define	CFG_LIB_FORK
#define	CFG_LIB_PID
#define	CFG_LIB_SIGNAL
#define	CFG_LIB_ENV
#define	CFG_LIB_THREAD
#define	CFG_LIB_RAND
#define	CFG_LIB_MALLOC
#define	CFG_LIB_PIPE
#define	CFG_LIB_SYSTEM
#define	CFG_LIB_FIFO
#define	CFB_LIB_MINCORE
#define	CFG_LIB_SEMAPHORE	// POSIX semaphore
#define CFG_LIB_SEM			// XSI semaphore
#define	CFG_LIB_SHARED_MEM
#define	CFG_LIB_MSG_QUEUE
#define	CFG_LIB_SOCKET
#define	CFG_LIB_NET
#define	CFG_LIB_MATH
#define	CFG_LIB_DAEMON
#define	CFG_LIB_STR_NUM
#define	CFG_LIB_TIMER
#define	CFG_LIB_MMAP
#define CFG_LIB_INOTIFY
#define CFG_LIB_KQUEUE
#define CFG_LIB_DNS
#define CFG_LIB_SELECT
#endif



/* common data define */
#if 0
#ifdef	CFG_LIB_DATA_TYPE
/* unsigned */
#ifndef	_UINT8_T
#define	_UINT8_T
typedef	unsigned char	uint8_t;
#endif

#ifndef	_UINT16_T
#define	_UINT16_T
typedef	unsigned short	uint16_t;
#endif

#ifndef	_UINT32_T
#define	_UINT32_T
typedef	unsigned int	uint32_t;
#endif

#ifndef	_UINT64_T
#define	_UINT64_T
typedef	unsigned long long	uint64_t;
#endif

/* signed */
#ifndef	_SINT8_T
#define	_SINT8_T
typedef	signed char		sint8_t;
#endif

#ifndef	_SINT16_T
#define	_SINT16_T
typedef	signed short	sint16_t;
#endif

#ifndef	_SINT32_T
#define	_SINT32_T
typedef	signed int		sint32_t;
#endif

#ifndef	_SINT64_T
#define	_SINT64_T
typedef	signed long long	sint64_t;
#endif

/* signed, another type */
/* signed */
#ifndef	_INT8_T
#define	_INT8_T
typedef	sint8_t		int8_t;
#endif

#ifndef	_INT16_T
#define	_INT16_T
typedef	sint16_t	int16_t;
#endif

#ifndef	_INT32_T
#define	_INT32_T
typedef	sint32_t	int32_t;
#endif

#ifndef	_UINT64_T
#define	_INT64_T
typedef	sint64_t	int64_t;
#endif
#endif	/* CFG_LIB_DATA_TYPE */
#endif

/* general data types */
	/* The only application of these macros is make the editor recognize them */
#ifdef	CFG_NOT_USING_LIB	/* This macro shall NEVER be defined unless you design your own OS! */
typedef size_t size_t;
typedef ssize_t ssize_t;
typedef FILE FILE;
typedef option option;
typedef time_t time_t;
typedef pid_t pid_t;
typedef pthread_t pthread_t;
typedef pthread_attr_t pthread_attr_t;
typedef pthread_mutex_t pthread_mutex_t;
typedef pthread_spinlock_t pthread_spinlock_t;
typedef pthread_mutexattr_t pthread_mutexattr_t;
#define	PTHREAD_MUTEX_INITIALIZER	PTHREAD_MUTEX_INITIALIZER
typedef pthread_cond_t pthread_cond_t;
typedef pthread_condattr_t pthread_condattr_t;
#define PTHREAD_COND_INITIALIZER		PTHREAD_COND_INITIALIZER
typedef pthread_rwlock_t pthread_rwlock_t;
typedef pthread_rwlockattr_t pthread_rwlockattr_t;
#define PTHREAD_RWLOCK_INITIALIZER		PTHREAD_RWLOCK_INITIALIZER
typedef sigset_t sigset_t;
typedef mode_t mode_t;
typedef sem_t sem_t;
#endif

/* general functions */
#include <stdlib.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void exit(int status);
#endif


/* standard out */
#ifdef	CFG_LIB_STDOUT
#include <stdio.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int printf(const char *format, ...);
extern int sprintf(char *str, const char *format, ...);
extern int snprintf(char *str, size_t size, const char *format, ...);
extern int putc(int c, FILE *stream);
extern int putchar(int c);
extern int fputc(int c, FILE *stream);
extern int fputs(const char *s, FILE *stream);
extern int puts(const char *s);
#endif
typedef enum {
	PrintfOpt_None = 0,
		
	PrintfColor_Front_Black,
	PrintfColor_Front_DarkGrey,
	PrintfColor_Front_Blue,
	PrintfColor_Front_LightBlue,
	PrintfColor_Front_Green,
	PrintfColor_Front_LightGreen,
	PrintfColor_Front_Cyan,
	PrintfColor_Front_LightCyan,
	PrintfColor_Front_Red,
	PrintfColor_Front_LightRed,
	PrintfColor_Front_Purple,
	PrintfColor_Front_LightPurple,
	PrintfColor_Front_Brown,
	PrintfColor_Front_Yellow,
	PrintfColor_Front_LightGrey,
	PrintfColor_Front_White,

	PrintfColor_Background_None,
	PrintfColor_Background_Black,
	PrintfColor_Background_DarkGrey,
	PrintfColor_Background_Blue,
	PrintfColor_Background_LightBlue,
	PrintfColor_Background_Green,
	PrintfColor_Background_LightGreen,
	PrintfColor_Background_Cyan,
	PrintfColor_Background_LightCyan,
	PrintfColor_Background_Red,
	PrintfColor_Background_LightRed,
	PrintfColor_Background_Purple,
	PrintfColor_Background_LightPurple,
	PrintfColor_Background_Brown,
	PrintfColor_Background_Yellow,
	PrintfColor_Background_LightGrey,
	PrintfColor_Background_White,

	PrintfCursor_Underline,
	PrintfCursor_Blink,
	PrintfCursor_Invert,
	PrintfCursor_Blanking,

	PrintfOpt_Illegal
} PrintfOpt_t;
ssize_t AMCPrintf(const char *format, ...);
ssize_t AMCPrintErr(const char *format, ...);
int AMCPrintfSetOpt(PrintfOpt_t option);
#endif



/* standard error out */
#ifdef	CFG_LIB_ERROUT
ssize_t errPrintf(const char *format, ...);
#endif


/* standard in */
#ifdef	CFG_LIB_STDIN
#include <stdio.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int scanf(const char *format, ...);
extern int sscanf(const char *str, const char *format, ...);
extern char *gets(char *s);
#endif
#endif


/* sleep */
#ifdef	CFG_LIB_SLEEP
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern unsigned int sleep(unsigned int seconds);
/* this may be alternate between different architectures */
extern int usleep (__useconds_t __useconds);
#endif
#endif


/* string operation */
#ifdef	CFG_LIB_STRING
#include <string.h>
#include <ctype.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern char *strcpy(char *dest, const char *src);
extern char *strncpy(char *dest, const char *src, size_t n);
extern size_t strlen(const char *s);
extern size_t strnlen(const char *s, size_t maxlen);
extern int strcmp(const char *s1, const char *s2);
extern int strncmp(const char *s1, const char *s2, size_t n);
extern int strcasecmp(const char *s1, const char *s2);
extern int strncasecmp(const char *s1, const char *s2, size_t n);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t n);
extern int toupper(int c);
extern int tolower(int c);
extern size_t strspn(const char *s, const char *accept);
extern size_t strcspn(const char *s, const char *reject);
extern char *strstr(const char *haystack, const char *needle);
extern char *strcasestr(const char *haystack, const char *needle);
extern int isalnum(int c);
extern int isalpha(int c);
extern int isascii(int c);
extern int isblank(int c);
extern int iscntrl(int c);
extern int isdigit(int c);
extern int isgraph(int c);
extern int islower(int c);
extern int isprint(int c);
extern int ispunct(int c);
extern int isspace(int c);
extern int isupper(int c);
extern int isxdigit(int c);
#endif
void strnUpper(char *str, const size_t size);
void strnLower(char *str, const size_t size);
void printData(const void *data, const size_t size);
#endif


/* memory operation */
#ifdef	CFG_LIB_MEM
#include <string.h>
#include <strings.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern void *memmove(void *dest, const void *src, size_t n);
extern void bzero(void *s, size_t n);
extern void bcopy(const void *src, void *dest, size_t n);
int bcmp(const void *s1, const void *s2, size_t n);
#endif
#endif



/* file operation */
#ifdef	CFG_LIB_FILE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern FILE *fopen(const char *path, const char *mode);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int fclose(FILE *stream);
extern int fgetc(FILE *stream);
extern int getc(FILE *stream);
extern int fputc(int c, FILE *stream);
extern int fputs(const char *s, FILE *stream);
extern char *fgets(char *s, int size, FILE *stream);
extern int fstat(int filedes, struct stat *buf);
extern int lstat(const char *path, struct stat *buf);
extern int fseek(FILE *stream, long offset, int whence);
extern long ftell(FILE *stream);
extern int fprintf(FILE *stream, const char *format, ...);
extern int fscanf(FILE *stream, const char *format, ...);
extern int fflush(FILE *stream);
extern int feof(FILE *stream);
extern int ferror(FILE *stream);
extern int symlink(const char *oldpath, const char *newpath);	/* soft link */
extern int link(const char *oldpath, const char *newpath);		/* hard link */
extern int unlink(const char *pathname);
extern int access(const char *pathname, int mode);
#endif
#ifndef	__STAT_T
#define	__STAT_T
typedef	struct stat stat_st;
#endif
#endif


/* device operation */
#ifdef	CFG_LIB_DEVICE
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern ssize_t write(int fildes, const void *buf, size_t nbytes);
extern ssize_t read (int fidles, void *buff, size_t nbytes);
extern int open (const char *path, int oflags);
extern int close(int fildes);
extern int ioctl (int fildes, int cmd, ...);
extern off_t lseek(int fildes, off_t offset, int whence);
extern int stat(const char *path, struct stat *buf);
extern int fcntl(int fd, int cmd, ... /* arg */ );
#endif
int nonblock(int fd);
int noninheritance(int fd);
#ifndef	__STAT_T
#define	__STAT_T
typedef	struct stat stat_st;
#endif
#endif


/* error number */
#ifdef	CFG_LIB_ERRNO
#include <errno.h>
#include <stdio.h>
#include <string.h>
extern int errno;
#ifdef	CFG_DECLARE_LIB_FUNC
extern void perror(const char *s);
extern char *strerror(int errnum);
#endif
void markPerror(char *fileName, int fileLine);
#define	SYS_PERROR()	markPerror(__FILE__, __LINE__);
#endif


/* get option */
#ifdef	CFG_LIB_GETOPT
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int getopt(int argc, char * const argv[], const char *optstring);
#endif
extern char *optarg;
extern int optind, opterr, optopt;
#endif


/* get option long */
#ifdef	CFG_LIB_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
typedef struct option option_st;
#ifdef	CFG_DECLARE_LIB_FUNC
extern int getopt_long(int argc, char * const argv[],
						const char *optstring,
						const struct option *longopts, int *longindex);
#endif
#endif


/* enviroment */
#ifdef	CFG_LIB_ENV
#include <stdlib.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int putenv(char *string);
extern char *getenv(const char *name);
#endif
#endif


/* system time */
#ifdef	CFG_LIB_TIME
#include <time.h>
#include <sys/time.h>
typedef struct tm tm_st;
typedef struct timeval timeval_st;
typedef struct timezone timezone_st;
#ifdef	CFG_DECLARE_LIB_FUNC
extern time_t time(time_t *t);
extern double difftime(time_t time1, time_t time0);
extern struct tm *gmtime(const time_t *timep);
extern struct tm *localtime(const time_t *timep);
extern time_t mktime(struct tm *tm);
extern size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);
extern int gettimeofday(struct timeval *tv, struct timezone *tz);
extern int settimeofday(const struct timeval *tv , const struct timezone *tz);
/* This function is not used occasionally */
//#define _XOPEN_SOURCE
//extern char *strptime(const char *s, const char *format, struct tm *tm);
#endif
time_t uptime(time_t *sec, int *mSec);
#endif


/* create a new process */
#ifdef	CFG_LIB_FORK
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define	PID_CHILD	(0)
#define	PID_FAIL	(-1)
#ifdef	CFG_DECLARE_LIB_FUNC
extern pid_t fork(void);
extern pid_t vfork(void);
extern pid_t wait(int *status);
extern pid_t waitpid(pid_t pid, int *status, int options);
extern int execl(const char *path, const char *arg, ...);
extern int execlp(const char *file, const char *arg, ...);
extern int execle(const char *path, const char *arg, ..., char * const envp[]);
extern int execv(const char *path, char *const argv[]);
extern int execvp(const char *file, char *const argv[]);
#endif
#endif


/* get PID */
#ifdef	CFG_LIB_PID
#include <sys/types.h>
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern pid_t getpid(void);
extern pid_t getppid(void);
#endif
#endif


/* catch a signal */
#ifdef	CFG_LIB_SIGNAL
#include <signal.h>
typedef void (*sighandler_t)(int);
typedef struct sigaction sigaction_st;
//extern sighandler_t signal(int signum, sighandler_t handler);		/* not recommanded */
int simpleSigaction(int signum, sighandler_t act);
#ifdef	CFG_DECLARE_LIB_FUNC
extern int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
extern int sigemptyset(sigset_t *set);
extern int sigfillset(sigset_t *set);
extern int sigaddset(sigset_t *set, int signum);
extern int sigdelset(sigset_t *set, int signum);
extern int sigismember(const sigset_t *set, int signum);
#endif
#if 0
#define	SIGABRT
#define	SIGFPE 
#define	SIGILL 
#define	SIGINT		/* Ctrl + C */
#define	SIGSEGV 
#define	SIGTERM		
#define	SIGKILL
#endif
#endif


/* POSIX thread */
#ifdef	CFG_LIB_THREAD
#include <pthread.h>
#include <signal.h>
#ifdef __TIMESPEC_ST
#define __TIMESPEC_ST
typedef struct timespec timespec_st;
#endif
int pthread_alive(pthread_t thread);
inline int pthread_disableCancel(void);
inline int pthread_enableCancel(void);
#ifdef	CFG_DECLARE_LIB_FUNC
extern int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
extern int pthread_join(pthread_t thread, void **retval);
extern int pthread_detach(pthread_t thread);
extern void pthread_exit(void *retval);
extern pthread_t pthread_self(void);
extern int pthread_setcancelstate(int state, int *oldstate);
extern int pthread_setcanceltype(int type, int *oldtype);
extern void pthread_testcancel(void);
extern int pthread_cancel(pthread_t thread);
extern int pthread_kill(pthread_t thread, int sig);

extern int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
extern int pthread_mutex_destroy(pthread_mutex_t *mutex);
extern int pthread_mutex_lock(pthread_mutex_t *mutex);
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);

extern int pthread_spin_init(pthread_spinlock_t *lock, int pshared);
extern int pthread_spin_destroy(pthread_spinlock_t *lock);
extern int pthread_spin_lock(pthread_spinlock_t *lock);
extern int pthread_spin_trylock(pthread_spinlock_t *lock);
extern int pthread_spin_unlock(pthread_spinlock_t *lock);

extern int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
extern int pthread_cond_destroy(pthread_cond_t *cond);
extern int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
extern int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
extern int pthread_cond_broadcast(pthread_cond_t *cond);
extern int pthread_cond_signal(pthread_cond_t *cond);

extern int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr);
extern int pthread_rwlock_destroy(pthread_rwlock_t *rwlock);
extern int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock);
extern int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock);
extern int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock);
extern int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock);
extern int pthread_rwlock_unlock(pthread_rwlock_t *rwlock);
#endif
#endif

/* random functions */
#ifdef	CFG_LIB_RAND
#include <stdlib.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int rand(void);
extern int rand_r(unsigned int *seedp);
extern void srand(unsigned int seed);
#endif
int randInt(int minInt, int maxInt);
#endif


/* memory allocation */
#ifdef	CFG_LIB_MALLOC
#include <stdlib.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void *calloc(size_t nmemb, size_t size);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void *realloc(void *ptr, size_t size);
#endif
#endif


/* pipe */
#ifdef	CFG_LIB_PIPE
#include <stdio.h>
#include <unistd.h>
#define	PIPE_I		(0)
#define	PIPE_O		(1)
#ifdef	CFG_DECLARE_LIB_FUNC
extern FILE *popen(const char *command, const char *type);	/* not recommanded */
extern int pclose(FILE *stream);
extern int pipe(int filedes[2]);
#endif
#endif


/* formated system() */
#ifdef	CFG_LIB_SYSTEM
int execFormatCmd(const char *format, ...);
#endif


/* FIFO */
#ifdef	CFG_LIB_FIFO
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int mkfifo(const char *pathname, mode_t mode);
//extern int mknod(const char *pathname, mode_t mode, dev_t dev);	/* not recomamded */
#endif
int simpleMknod(const char *pathname, mode_t mode);
#endif


/* mincore() */
#ifdef	CFB_LIB_MINCORE
#include <unistd.h>
#include <sys/mman.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int mincore(void *start, size_t length, unsigned char *vec);
#endif
#endif


/* XSI semaphore */
	/* In different Linux system, the definations may be various */
#ifdef	CFG_LIB_SEM
union semun{
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
 	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
typedef struct sembuf sembuf_st;
#ifndef __TIMESPEC_ST
#define __TIMESPEC_ST
typedef struct timespec timespec_st;
#endif
#ifdef	CFG_DECLARE_LIB_FUNC
extern int semctl(int semid, int semnum, int cmd, ...);
extern int semget(key_t key, int nsems, int semflg);
extern int semop(int semid, struct sembuf *sops, unsigned nsops);
extern int semtimedop(int semid, struct sembuf *sops, unsigned nsops, struct timespec *timeout);
#endif
int simpleSemCreate(key_t key);
int simpleSemSet(int semId);
int simpleSemDelete(int semId);
int simpleSem_P(int semId);
int simpleSem_V(int semId);
#endif

/* POSIX semaphore */
#ifdef CFG_LIB_SEMAPHORE
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#ifndef __TIMESPEC_ST
#define __TIMESPEC_ST
typedef struct timespec timespec_st;
#endif
#ifdef	CFG_DECLARE_LIB_FUNC
extern sem_t *sem_open(const char *name, int oflag);
//extern sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
extern int sem_close(sem_t *sem);
extern int sem_destroy(sem_t *sem);
extern int sem_getvalue(sem_t *sem, int *sval);
extern int sem_init(sem_t *sem, int pshared, unsigned int value);
extern int sem_post(sem_t *sem);
extern int sem_wait(sem_t *sem);
extern int sem_trywait(sem_t *sem);
extern int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
extern int sem_unlink(const char *name);
#endif
#endif


/* shared memory */
#ifdef	CFG_LIB_SHARED_MEM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void *shmat(int shmid, const void *shmaddr, int shmflg);
extern int shmdt(const void *shmaddr);
extern int shmctl(int shmid, int cmd, struct shmid_ds *buf);;
extern int shmget(key_t key, size_t size, int shmflg);
#endif
int simpleShmCreate(key_t key, size_t size);
void *simpleShmLink(int memId);
int simpleShmUnlink(const void *shmAddr);
int simpleShmDelete(int shmId);
#endif


/* message queue */
#ifdef	CFG_LIB_MSG_QUEUE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int msgget(key_t key, int msgflg);
extern int msgctl(int msqid, int cmd, struct msqid_ds *buf);
extern ssize_t msgrcv(int msqid, void *msgp, size_t msgsz, long msgtyp, int msgflg);
extern int msgsnd(int msqid, const void *msgp, size_t msgsz, int msgflg);
#endif
ssize_t getSystemMsgmax(void);
ssize_t getSystemMsgmnb(void);
int simpleMsqCreate(key_t key);
int simpleMsqDelete(int msqId);
int simpleMsqSend(int msqId, long int msgType, void * pMsg, size_t size);
ssize_t simpleMsqRecv(int msqId, long int *pMsgType, void * pMsg, size_t size, int isNoWait);
#endif


/* Berkeley Socket */
/* 	If you want to use socket locally only, use AF_UNIX. If you want it open on Internet, 
 * use AF_INET. And define the macro "CFG_LIB_NET".
 */
#ifdef	CFG_LIB_SOCKET
/* file */
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
/* socket */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>		/* AF_UNIX */
#include <netinet/in.h>		/* AF_INET */
#include <arpa/inet.h>
typedef struct sockaddr sockaddr_st;
typedef struct sockaddr_un sockaddr_un_st;
typedef struct sockaddr_in sockaddr_in_st;
typedef struct sockaddr_in6 sockaddr_in6_st;
typedef struct in_addr in_addr_st;
#define	SOCKET_NAME_LEN_LIMIT	(sizeof(sockaddr_un_t) - sizeof(sa_family_t))

#ifdef	CFG_DECLARE_LIB_FUNC
struct in_addr {
	uint32_t	s_addr;
};
struct in6_addr {
	union {
		uint8_t		u6_addr8[16];
		uint16_t	u6_addr16[8];
		uint32_t	u6_addr32[4];
	} in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
};
struct sockaddr {
	uint16_t	sa_family;		/* address family, AF_xxx	*/
	char		sa_data[14];	/* 14 bytes of protocol address	*/
};
struct sockaddr_un {
	uint16_t	sun_family;	/* AF_UNIX */
	char		sun_path[108];	/* pathname */
};
struct sockaddr_in {
	uint16_t		sin_family;	/* Address family		*/
	uint16_t		sin_port;		/* Port number			*/
	struct in_addr	sin_addr;		/* Internet address		*/

	/* Pad to size of `struct sockaddr'. */
	unsigned char		__pad[8];
};
struct sockaddr_in6 {
	uint16_t		sin6_family;    /* AF_INET6 */
	uint16_t		sin6_port;      /* Transport layer port # */
	uint32_t		sin6_flowinfo;  /* IPv6 flow information */
	struct in6_addr		sin6_addr;      /* IPv6 address */
	uint32_t			sin6_scope_id;  /* scope id (new in RFC2553) */
};
extern int socket(int domain, int type, int protocol);
extern int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
extern int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen);
extern int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
extern int listen(int sockfd, int backlog);
extern ssize_t send(int s, const void *buf, size_t len, int flags);
extern ssize_t sendto(int s, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen);
extern ssize_t recv(int s, void *buf, size_t len, int flags);
extern ssize_t recvfrom(int s, void *buf, size_t len, int flags,struct sockaddr *from, socklen_t *fromlen);
extern int getsockopt(int s, int level, int optname, void *optval, socklen_t *optlen);
extern int setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen);
#endif
unsigned int simpleSocketErrLine(void);		// used when a function contains mutiple system call
int simpleSocketCreate_local(void);
int simpleSocketCreate_tcp(void);
int simpleSocketCreate_udp(void);
int simpleSocketName_local(int socketId, char *socketName);
int simpleSocketName_tcp(int socketId, const char *serverIpAddr, int port);
int simpleSocketListen(int socketId, int pendingLimit);
int simpleSocketAccept_local(int socketId, const char *socketName, int isNoWait);
int simpleSocketAccept_internet(int socketId, const char *serverIpAddr, int port, int isNoWait);
int simpleSocketConnect_local(int socketId, char *socketName);
int simpleSocketConnect_internet(int socketId, const char *serverIpAddr, int port, const struct timeval *timeout);
int simpleSocketBind_udp(int socketId, int portFrom);
int simpleSocketBind_tcp(int socketId, int portFrom);
ssize_t simpleSocketWrite_tcp(int socketId, const void *data, size_t dataLen);
ssize_t simpleSocketRead_tcp(int socketId, void *dataRead, size_t dataReadLimit, const struct timeval *timeout);
ssize_t simpleSocketSendto_udp
							(int socketId, 
							const char *targetIPAddr, 
							int port, 
							const void *data, 
							size_t dataLen);
ssize_t simpleSocketReceiveFrom_udp
							(int socketId, 
							const char *sourceIpAddr, 
							struct sockaddr_in *srcAddress,
							void *dataRead, 
							size_t dataReadLimit, 
							const struct timeval *timeout);
int simpleSocketClose(int socketId);
#endif


/* network interface */	// TODO:
#ifdef	CFG_LIB_NET
#include <netinet/in.h>		/* AF_INET */
#include <arpa/inet.h>
#include <ifaddrs.h>
/* Please pay attention to "in_addr_t"  and "struct in_addr" */
#ifdef	CFG_DECLARE_LIB_FUNC
#define INET_ADDRSTRLEN INET_ADDRSTRLEN
#define INET6_ADDRSTRLEN INET6_ADDRSTRLEN
struct ifaddrs {
	struct ifaddrs		*ifa_next;    /* Next item in list */
	char				*ifa_name;    /* Name of interface */
	unsigned int		ifa_flags;   /* Flags from SIOCGIFFLAGS */
	struct sockaddr		*ifa_addr;    /* Address of interface */
	struct sockaddr		*ifa_netmask; /* Netmask of interface */
	union {
		struct sockaddr *ifu_broadaddr;	/* Broadcast address of interface */
		struct sockaddr *ifu_dstaddr;		/* Point-to-point destination address */
	} ifa_ifu;
	#define ifa_broadaddr	ifa_ifu.ifu_broadaddr
	#define ifa_dstaddr		ifa_ifu.ifu_dstaddr
	void				*ifa_data;    /* Address-specific data */
};
extern int inet_aton(const char *cp, struct in_addr *inp);
extern in_addr_t inet_addr(const char *cp);
extern in_addr_t inet_network(const char *cp);
//extern char *inet_ntoa(struct in_addr in);			/* not recommended */
extern struct in_addr inet_makeaddr(int net, int host);
extern in_addr_t inet_lnaof(struct in_addr in);
extern in_addr_t inet_netof(struct in_addr in);
extern int inet_pton(int af, const char *src, void *dst);
extern const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
extern uint32_t htonl(uint32_t hostlong);
extern uint16_t htons(uint16_t hostshort);
extern uint32_t ntohl(uint32_t netlong);
extern uint16_t ntohs(uint16_t netshort);
extern int getifaddrs(struct ifaddrs **ifap);
extern void freeifaddrs(struct ifaddrs *ifa);
#endif
typedef struct ifaddrs ifaddrs_st;
BOOL isSystemBigEndian();
#if defined(__APPLE__) && defined(__MACH__)
#else
uint64_t htonll(uint64_t hostlonglong);
uint64_t ntohll(uint64_t netlonglong);
#endif
size_t ifaddrsGetAllIPv4(const struct ifaddrs *ifa, 
                           struct in_addr *ipBuffer, 
                           struct in_addr *maskBuffer,
                           size_t bufferCountMax);
void inet_n4top(struct in_addr addr, char* str, size_t strLenLimit);
char *sock_ntop(const struct sockaddr *sa, socklen_t salen);
#endif


/* math.h */
#ifdef	CFG_LIB_MATH
#include <math.h>
#include <stdlib.h>
/* 	Macros should have been defined: 
 * float_t, double_t, 
 * M_E, M_LOG2E, M_LOG10E, M_LN2, M_LN10
 * M_PI, M_PI_2, M_PI_4, M_1_PI, M_2_PI, 
 * M_2_SQRTPI, M_SQRT2, M_SQRT1_2
 * MAXFLOAT
 * HUGE_VAL, HUGE_VALF, HUGE_VALL, INFINITY
 */
#define	INFINITY_F	HUGE_VALF
#define	INFINITY_D	HUGE_VAL
#define	INFINITY_L	HUGE_VALL
#ifdef	CFG_DECLARE_LIB_FUNC
/* abs() */;
extern int abs(int j);
extern long int labs(long int j);
extern long long int llabs(long long int j);
/* sin() */
extern double sin(double x);
extern float sinf(float x);
extern long double sinl(long double x);
extern double asin(double x);
extern float asinf(float x);
extern long double asinl(long double x);
/* cos() */
extern double cos(double x);
extern float cosf(float x);
extern long double cosl(long double x);
extern double acos(double x);
extern float acosf(float x);
extern long double acosl(long double x);
/* tan() */
extern double tan(double x);
extern float tanf(float x);
extern long double tanl(long double x);
extern double atan(double x);
extern float atanf(float x);
extern long double atanl( long double x);
/* exp() */
extern double exp(double x);
extern float expf(float x);
extern long double expl(long double x);
/* power */
extern double pow(double x, double y);
extern float powf(float x, float y);
extern long double powl(long double x, long double y);
/* square */
extern double sqrt(double x);
extern float sqrtf(float x);
extern long double sqrtl(long double x);
/* split the number x into a normalized fraction and an exponent, based 2 */
extern double frexp(double x, int *exp);
extern float frexpf(float x, int *exp);
extern long double frexpl(long double x, int *exp);
/* log() */
extern double log(double x);
extern float logf(float x);
extern long double logl(long double x);
extern double log10(double x);
extern float log10f(float x);
extern long double log10l(long double x);
/* symbol process */
extern fabs(double x);
extern float fabsf(float x);
extern long double fabsl(long double x);
extern double ceil(double x);
extern float ceilf(float x);
extern long double ceill(long double x);
#endif
#endif


/* run in the background */
#ifdef	CFG_LIB_DAEMON
#include <unistd.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int daemon(int nochdir, int noclose);
#endif
#endif


#ifdef CFG_LIB_STR_NUM
#include <stdlib.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int strtol(const char *nptr, char **endptr, int base);
extern long long strtoll(const char *nptr, char **endptr, int base);
#endif
#endif

#ifdef CFG_LIB_TIMER
#include <signal.h>
#include <time.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int timer_create(clockid_t clockid, 
						struct sigevent *restrict evp, 
						timer_t *restrict timerid);
extern int timer_gettime(timer_t timerid, struct itimerspec *value);
extern int timer_settime(timer_t timerid, 
							int flags,
							const struct itimerspec *restrict value,
							struct itimerspec *restrict ovalue);
extern int timer_getoverrun(timer_t timerid);
#endif
#define	TIMER_N_ONE_USEC	(1000)
#define	TIMER_N_ONE_MSEC	(1000000)
#define	TIMER_N_ONE_SEC	(1000000000)
int simpleTimerCreate(int sec, int nsec, void (*timeOutCallback)(void));
int simpleTimerStop();
#endif

#ifdef	CFG_LIB_MMAP
#include <sys/mman.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
extern void *mmap64(void *addr, size_t length, int prot, int flags, int fd, off64_t offset);
extern int munmap(void *addr, size_t length);
extern int msync(void *addr, size_t length, int flags);
#endif
#endif

#ifdef CFG_LIB_INOTIFY
#if (CFG_OS_TYPE == OS_TYPE_LINUX)		/* Linux system only */ 
#include <sys/inotify.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int inotify_init(void);
extern int inotify_add_watch(int fd, const char* pathname, int mask);
extern int inotify_rm_watch(int fd, int wd);
#endif
#endif
#endif

#ifdef CFG_LIB_KQUEUE
#if (CFG_OS_TYPE == OS_TYPE_IPHONE) || (CFG_OS_TYPE == OS_TYPE_MAC_OS_X)
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int kqueue(void);
extern int kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout);
extern int kevent64(int kq, const struct kevent64_s *changelist, int nchanges, struct kevent64_s *eventlist, int nevents, unsigned int flags, const struct timespec *timeout);
extern EV_SET(&kev, ident, filter, flags, fflags, data, udata);
extern EV_SET64(&kev, ident, filter, flags, fflags, data, udata, ext[_], ext[1]);
/* https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man2/kqueue.2.html */
#endif
#endif
#endif


#ifdef CFG_LIB_DNS
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
typedef struct hostent hostent_st;
typedef struct addrinfo addrinfo_st;
#ifdef	CFG_DECLARE_LIB_FUNC
struct hostent {
	char  *h_name;            /* official name of host */
	char **h_aliases;         /* alias list */
	int    h_addrtype;        /* host address type */
	int    h_length;          /* length of address */
	char **h_addr_list;       /* list of addresses */
}
#define h_addr h_addr_list[0] /* for backward compatibility */
extern int h_errno;

struct addrinfo {
	int              ai_flags;
	int              ai_family;
	int              ai_socktype;
	int              ai_protocol;
	size_t           ai_addrlen;
	char            *ai_canonname;
	struct sockaddr *ai_addr;
	struct addrinfo *ai_next;
};

extern struct hostent *gethostbyname(const char *name);			/* not recommanded */
extern struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
extern void sethostent(int stayopen);
extern void endhostent(void);
extern void herror(const char *s);
extern const char *hstrerror(int err);
extern struct hostent *gethostent(void);
extern struct hostent *gethostbyname2(const char *name, int af);

extern int gethostent_r(struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop);
extern int gethostbyaddr_r(const void *addr, socklen_t len, int type, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop);
extern int gethostbyname_r(const char *name, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop);
extern int gethostbyname2_r(const char *name, int af, struct hostent *ret, char *buf, size_t buflen, struct hostent **result, int *h_errnop);

extern int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
extern void freeaddrinfo(struct addrinfo *res);
extern const char *gai_strerror(int errcode);
#endif
#endif

#ifdef CFG_LIB_SELECT
/******/
/* select() for all */
/* According to POSIX.1-2001 */
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int  select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
extern void FD_CLR(int fd, fd_set *set);
extern int  FD_ISSET(int fd, fd_set *set);
extern void FD_SET(int fd, fd_set *set);
extern void FD_ZERO(fd_set *set);
extern int  pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);
#endif
/******/
/* poll() for Linux */
#if	(CFG_OS_TYPE==OS_TYPE_LINUX)
#include <poll.h>
 #ifdef	CFG_DECLARE_LIB_FUNC
extern int poll(struct pollfd *fds, nfds_t nfds, int timeout);
 #endif
#endif
/******/
/* epoll() for Linux */
#if	(CFG_OS_TYPE==OS_TYPE_LINUX)
#include <sys/epoll.h>
 #ifdef	CFG_DECLARE_LIB_FUNC
extern int epoll_create(int size);
extern int epoll_ctl(int epfd, int op, struct epoll_event *event);
 #endif
#endif

#endif	/* endof CFG_LIB_SELECT */


#endif	/* end of file */

