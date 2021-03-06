/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang

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
						CFG_LIB_SEMAPHORE,
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

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_COMMON_LIB
#define	_AMC_COMMON_LIB

#include "AMCCommonLibConfig.h"


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
define	CFG_OS_TYPE		OS_TYPE_IPHONE
#elif TARGET_OS_IPHONE == 1		/* iOS on iPhone, iPad, etc. */
define	CFG_OS_TYPE		OS_TYPE_IPHONE
//#elif TARGET_OS_MAC == 1	/* OSX */
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
#define	CFG_SOFTWARE_NAME_STR		"motor"
#define	CFG_SOFTWARE_DISCRIPT_STR	"A file to test GPIO"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"


/*******************************************************
 * library config
 */
#define _GNU_SOURCE


#endif
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
#define	DB_LOG(fmt, args...)	printf("##"__FILE__", %d: "fmt, __LINE__, ##args)
#define	LOG(fmt, args...)	printf("##"__FILE__", %d: "fmt, __LINE__, ##args)
#define	DB(x)	x
#define	TEST_VALUE(x)		printf("##"__FILE__", %d: "#x" = 0x%x = %d\n", __LINE__, (unsigned int)x, x)
#define	TEST_64B_VALUE(x)	printf("##"__FILE__", %d: "#x" = 0x%Lx = %Ld\n", __LINE__, x, x)
#define	TEST_CHAR(x)		printf("##"__FILE__", %d: "#x" is '%c'\n", __LINE__, x)
#define	TEST_STR(x)		printf("##"__FILE__", %d: "#x" is \"%s\"\n", __LINE__, x)
#define	TODO(x)			printf("##"__FILE__", %d: TODO: %s\n",__LINE__, x)
#define	MARK()		printf("<<MARK>> "__FILE__", %d\n", __LINE__)
#define	FUNC_MARK()	printf("<<MARK>>"__FUNC__", %d\n", __LINE__)
#else
#define	DB_LOG(fmt, args...)
#define	LOG(fmt, args...)	printf(fmt, ##args)
#define	DB(x)
#define	TEST_VALUE(x)
#define	TEST_64B_VALUE(x)
#define	TEST_CHAR(x)
#define	TEST_STR(x)
#define	TODO(x)			printf("!!!"__FILE__" %d TODO: %s\n", x)
#define	MARK()
#define	FUNC_MARK()
#endif

#ifndef	FALSE
#define	FALSE	(0)
#define	TRUE	(!FALSE)
#endif

#ifndef	NULL
#define	NULL	((void *)0)
#endif

#if (OS_TYPE_LINUX == CFG_OS_TYPE)
#ifndef	BOOL
#define	BOOL	int
#endif
#endif

#ifndef	LENOF
#define	LENOF(x)	(sizeof(x) / sizeof(*(x)))
#endif


/*******************************************************
 * Here shows how to define macros */

//#define	CFG_DECLARE_LIB_FUNC		// declare library functions to make editor recognize them


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
#define	CFG_LIB_SEMAPHORE
#define	CFG_LIB_SHARED_MEM
#define	CFG_LIB_MSG_QUEUE
#define	CFG_LIB_SOCKET
#define	CFG_LIB_NET
#define	CFG_LIB_MATH
#define	CFG_LIB_DAEMON
#define	CFG_LIB_STR_NUM
#define	CFG_LIB_TIMER
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
typedef pthread_mutexattr_t pthread_mutexattr_t;
#define	PTHREAD_MUTEX_INITIALIZER	PTHREAD_MUTEX_INITIALIZER
typedef sigset_t sigset_t;
typedef mode_t mode_t;
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
#endif
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
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, size_t n);
extern int toupper(int c);
extern int tolower(int c);
#endif
void strnUpper(char *str, const size_t size);
void strnLower(char *str, const size_t size);
#endif


/* memory operation */
#ifdef	CFG_LIB_MEM
#include <string.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern void *memcpy(void *dest, const void *src, size_t n);
extern void *memset(void *s, int c, size_t n);
extern int memcmp(const void *s1, const void *s2, size_t n);
extern  void *memmove(void *dest, const void *src, size_t n);
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
extern pid_t wait(int *status);
extern pid_t waitpid(pid_t pid, int *status, int options);
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
typedef struct sigaction SIGACTION_st;
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
#endif


/* POSIX thread */
#ifdef	CFG_LIB_THREAD
#include <pthread.h>
#ifdef	CFG_DECLARE_LIB_FUNC
extern int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);
extern int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
extern int pthread_mutex_destroy(pthread_mutex_t *mutex);
extern int pthread_mutex_lock(pthread_mutex_t *mutex);
extern int pthread_mutex_trylock(pthread_mutex_t *mutex);
extern int pthread_mutex_unlock(pthread_mutex_t *mutex);
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


/* semaphore */
	/* In different Linux system, the definations may be various */
#ifdef	CFG_LIB_SEMAPHORE
union semun{
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
 	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
typedef struct sembuf SEMBUF_st;
typedef struct timespec TIMESPEC_st;
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
#define	SOCKET_NAME_LEN_LIMIT	(sizeof(sockaddr_un_t) - sizeof(sa_family_t))

#ifdef	CFG_DECLARE_LIB_FUNC
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
/* Please pay attention to "in_addr_t"  and "struct in_addr" */
#ifdef	CFG_DECLARE_LIB_FUNC
extern int inet_aton(const char *cp, struct in_addr *inp);
extern in_addr_t inet_addr(const char *cp);
extern in_addr_t inet_network(const char *cp);
extern char *inet_ntoa(struct in_addr in);
extern struct in_addr inet_makeaddr(int net, int host);
extern in_addr_t inet_lnaof(struct in_addr in);
extern in_addr_t inet_netof(struct in_addr in);
extern int inet_pton(int af, const char *src, void *dst);
extern const char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt);
#endif
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

#ifndef	CFG_UNSUPPORTED_TIMER
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
#endif


//#endif	/* end of file */
