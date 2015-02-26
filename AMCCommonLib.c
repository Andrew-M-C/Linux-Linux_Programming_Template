/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang

	File name: 	AMCCommonLib.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide some covenient tool in calling library tools.
			
	History:
		2012-??-??: On about come date around middle of Year 2012, file created as "commonLib.h"
		2012-08-20: Till this day, errPrintf(), execFormatCmd(), markPerror(), randInt(), 
					simpleSigaction(), uptime(), simpleMknod(), strnUpper(), strnLower(),
					simpleSemCreate(), simpleSemSet(), simpleSemDelete(), simpleSem_P(),
					simpleSem_V() are created.
		2012-08-20: Add shared memory library; add message queue.
		2012-08-21: Add simple socket function interface.
		2012-10-10: Change file name as "AMCCommonLib.h"
		2012-12-04: Add client UDP support in AMC socket library
		2013-01-05: Add OS X support in uptime() function
		2013-01-21: Add support of simple timer setting
		2013-06-13: Add simpleSocketWrite_tcp() and simpleSocketRead_tcp();

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/* This is a common file */
#include "AMCDataTypes.h"

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
//#elif TARGET_OS_MAC == 1	/* OSX */
#else
#define	CFG_OS_TYPE		OS_TYPE_MAC_OS_X
#endif

#else	// ENDS: #if defined(__APPLE__) && defined(__MACH__)
/* Other */
#define	CFG_OS_TYPE		OS_TYPE_LINUX
#endif




//#define	COMM_LIB_DEBUG

#ifdef	COMM_LIB_DEBUG
#define	DB(x)	x
#define	TEST_VALUE(x)		printf("##"__FILE__", %d: "#x" = 0x%x = %d\n", __LINE__, (unsigned int)x, x)
#define	TEST_64B_VALUE(x)	printf("##"__FILE__", %d: "#x" = 0x%Lx = %Ld\n", __LINE__, x, x)
#define	TEST_CHAR(x)		printf("##"__FILE__", %d: "#x" is '%c'\n", __LINE__, x)
#define	TEST_STR(x)		printf("##"__FILE__", %d: "#x" is \"%s\"\n", __LINE__, x)
#define	TODO(x)			printf("##"__FILE__", %d: TODO: %s\n",__LINE__, x)
#else
#define	DB(x)
#define	TEST_VALUE(x)
#define	TEST_64B_VALUE(x)
#define	TEST_CHAR(x)
#define	TEST_STR(x)
#define	TODO(x)			printf("!!!"__FILE__" %d TODO: %s\n", x)
#endif




/* config of common library extension */
#define	CFG_MAX_ERROUT_SIZE	(1024)
#define CFG_MAC_AMC_PRINT_SIZE	(1024)

#ifndef	NULL
#define	NULL	((void *)0)
#endif

#include <unistd.h>
extern ssize_t write(int fd, const void *buf, size_t count);

#ifndef	_GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>
extern size_t strnlen(const char *s, size_t maxlen);

#include <stdarg.h>
//extern void va_start(va_list ap, last);
//extern void va_end(va_list ap);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#include <stdlib.h>
#include <time.h>
#include <stdio.h>
ssize_t errPrintf(const char *format, ...)
{
	char errBuff[CFG_MAX_ERROUT_SIZE];
	va_list vaList;

	va_start(vaList, format);
	vsnprintf((char *)errBuff, sizeof(errBuff), format, vaList);
	va_end(vaList);
	
	return (write(2, errBuff, strnlen((char *)errBuff, sizeof(errBuff))));
}

#include <sys/time.h>
ssize_t AMCPrintf(const char *format, ...)
{
	char buff[CFG_MAC_AMC_PRINT_SIZE];
	va_list vaList;
	size_t dateLen;
	time_t currTime = time(0);
	struct timeval currDayTime;

	gettimeofday(&currDayTime, NULL);
	dateLen = strftime(buff, CFG_MAC_AMC_PRINT_SIZE-1, "%F,%02H:%02M:%02S", localtime(&currTime));
	sprintf(buff + dateLen, ".%06ld", currDayTime.tv_usec);
	dateLen += 7;
	buff[dateLen - 3] = ' ';
	buff[dateLen - 2] = '\0';
	dateLen -= 2;

	va_start(vaList, format);
	vsnprintf((char *)(buff + dateLen), sizeof(buff) - dateLen - 1, format, vaList);
	va_end(vaList);

	dateLen = strlen(buff);
	buff[dateLen + 0] = '\n';
	buff[dateLen + 1] = '\0';

	return (write(1, buff, dateLen + 1));
}


static const char *_printfOptStr[] ={
	"\033[0m",

	"\033[0;30m",
	"\033[1;30m",
	"\033[0;34m",
	"\033[1;34m",
	"\033[0;32m",
	"\033[1;32m",
	"\033[0;36m",
	"\033[1;36m",
	"\033[0;31m",
	"\033[1;31m",
	"\033[0;35m",
	"\033[1;35m",
	"\033[0;33m",
	"\033[1;33m",
	"\033[0;37m",
	"\033[1;37m",

	"\033[0;40m",
	"\033[1;40m",
	"\033[0;44m",
	"\033[1;44m",
	"\033[0;42m",
	"\033[1;42m",
	"\033[0;46m",
	"\033[1;46m",
	"\033[0;41m",
	"\033[1;41m",
	"\033[0;45m",
	"\033[1;45m",
	"\033[0;43m",
	"\033[1;43m",
	"\033[0;47m",
	"\033[1;47m",

	"\033[4m",
	"\033[5m",
	"\033[7m",
	"\033[8m"
};

int AMCPrintfSetOpt(int option)
{
	static unsigned int arraySize = 0;

	if (0 == arraySize)
	{
		arraySize = (sizeof(_printfOptStr) / sizeof(*_printfOptStr));
	}

	if ((unsigned int)option >= arraySize)
	{
		return -1;
	}
	else
	{
		return printf(_printfOptStr[option]);
	}
}

/* headers included in errPrintf */
#define	COMMAND_LEN		(256)
static int systemEx(char *command);
int execFormatCmd(const char *format, ...)
{
	char cmdBuff[COMMAND_LEN];
	va_list vaList;

	va_start (vaList, format);
	vsnprintf ((char *)cmdBuff, sizeof(cmdBuff), format, vaList);
	va_end (vaList);

	return systemEx((char *)cmdBuff);
}

/* headers used to fork */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
//extern long sysconf(int name);
//extern int execve(const char *filename, char *const argv[], char *const envp[]);
static int systemEx(char *command)
{
	int pid = 0;
	int status = 0;
	int fd;
	char *argv[4];
	extern char **environ;

	if (NULL == command)
	{
		return 0;
	}

	pid = fork();
	if (pid < 0)
	{
		return -1;
	}
	
	if ( pid == 0 ) 
	{
		for (fd = 3; fd <  sysconf(_SC_OPEN_MAX); fd++)
		{
			close(fd); 
		}
	
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;

		execve("/bin/sh", argv, environ);
		exit(127);
	}

	/* wait for child process return */
	do 
	{
		if ( waitpid(pid, &status, 0) == -1 )
		{
			if ( errno != EINTR )
			{
				return -1;
			}
	   	}
		else
		{
			return status;
		}
	} while (1);

	return 0;
}


/* print an error with file name and line. Callling SYS_PERROR() recommanded */
#include <stdio.h>
#include <errno.h>
#define	PERROR_STR_HEADER_LEN	(256)
void markPerror(char *fileName, int fileLine)
{
	char head[PERROR_STR_HEADER_LEN];

	//sprintf(head, "%s, %d", fileName, fileLine);
	snprintf(head, sizeof(head), "%s, %d", fileName, fileLine);
	perror(head);

	return;
}


/* return a random interger between minInt and maxInt */
#include <stdlib.h>
int randInt(int minInt, int maxInt)
{
	int result;

	if (minInt == maxInt)
	{
		result = minInt;
	}
	else
	{
		if (minInt > maxInt)
		{
			/* swap */
			result = minInt;
			minInt = maxInt;
			maxInt = result;
		}

		/* randomize */
		result = minInt + (int)((float)(maxInt - minInt + 1) * (float)(rand() / (RAND_MAX + 1.0)));
	}
	return result;
}


/* a package of sigaction() */
#include <signal.h>
#include <errno.h>
typedef void (*sighandler_t)(int);
int simpleSigaction(int signum, sighandler_t act)
{
	struct sigaction sigConf = {};

	if (NULL == act)
	{
		errno = EIO;
		return -1;
	}

	sigemptyset(&(sigConf.sa_mask));
	sigConf.sa_handler = act;
	sigConf.sa_flags = 0;
	

	return sigaction(signum, &sigConf, NULL);
}


/* pthread tool */
#include <pthread.h>
#include <errno.h>
int pthread_alive(pthread_t thread)
{
	if (!thread)
	{
		return 0;
	}

	int callStat = pthread_kill(thread, 0);
	if (ESRCH == callStat)
	{
		return 0;
	}
	else if (EINVAL == callStat)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

inline int pthread_disableCancel()
{
	return pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
}

inline int pthread_enableCancel()
{
	return pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}


/* get system-up time */
#if (OS_TYPE_LINUX == CFG_OS_TYPE)
#include <time.h>
#include <stdio.h>
#include <errno.h>
#define	UPTIME_FILE_LEN	(256)
time_t uptime(time_t *sec, int *mSec)
{
	FILE *file;
	char buff[UPTIME_FILE_LEN];
	size_t upSec;
	int upMiliSec;

	file = fopen("/proc/uptime", "r");
	if (NULL == file)
	{
		return ((time_t)-1);
	}

	fgets(buff, sizeof(buff), file);
	fclose(file);

	if (sizeof(time_t) > 4)
	{
		sscanf(buff, "%lld.%d", (long long int *)(&upSec), &upMiliSec);
	}
	else
	{
		sscanf(buff, "%d.%d", (int *)(&upSec), &upMiliSec);
	}

	if (NULL != sec)
	{
		*sec = ((time_t)upSec);
	}
	if (NULL != mSec)
	{
		*mSec = upMiliSec * 10;
	}

	return ((time_t)upSec);
}
#elif (OS_TYPE_MAC_OS_X == CFG_OS_TYPE)
#include <sys/time.h>
#include <sys/sysctl.h>
time_t uptime(time_t *sec, int *msec)
{
	struct timeval currTimeSt;
	struct timeval upTimeSt;
	struct timeval diffTimeSt;
	size_t len = sizeof(upTimeSt);
	int mib[2] = {CTL_KERN, KERN_BOOTTIME};

	if (sysctl(mib, 2, &upTimeSt, &len, NULL, 0) < 0)
	{
		return -1;
	}
	else
	{
		gettimeofday(&currTimeSt, NULL);

		diffTimeSt.tv_sec = currTimeSt.tv_sec - upTimeSt.tv_sec;
		diffTimeSt.tv_usec = currTimeSt.tv_usec - upTimeSt.tv_usec;

		if (diffTimeSt.tv_usec < 0)
		{
			diffTimeSt.tv_sec --;
			diffTimeSt.tv_usec *= -1;
		}

		if (NULL != sec)
		{
			*sec = diffTimeSt.tv_sec;
		}

		if (NULL != msec)
		{
			*msec = diffTimeSt.tv_usec / 1000;
		}
	}

	return diffTimeSt.tv_sec;
}
#else
/* cause an error */
#endif

/* make a nod */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
int simpleMknod(const char *pathname, mode_t mode)
{
	return mknod(pathname, (mode | S_IFIFO), (dev_t)0);
}


/* string upper */
void strnUpper(char *str, const size_t size)
{
	int tmp;
	for (tmp = 0; (tmp < size) && ('\0' != str[tmp]); tmp++)
	{
		if ((str[tmp] >= 'a') && (str[tmp] <= 'z'))
		{
			str[tmp] += ('A' - 'a');
		}
	}
}


/* string lower */
void strnLower(char *str, const size_t size)
{
	int tmp;
	for (tmp = 0; (tmp < size) && ('\0' != str[tmp]); tmp++)
	{
		if ((str[tmp] >= 'A') && (str[tmp] <= 'Z'))
		{
			str[tmp] += ('a' - 'A');
		}
	}
}


/* print data */
static char _charFromByte(uint8_t byte)
{
	if ((byte >= '!') && (byte <= 0x7F))
	{
		return (char)byte;
	}
	else if ('\n' == byte)
	{
		return '.';
	}
	else if ('\r' == byte)
	{
		return '.';
	}
	else if (' ' == byte)
	{
		return ' ';
	}
	else
	{
		return '.';
	}
}

void printData(const void *pData, const size_t size)
{
	size_t column, tmp;
	char lineString[64] = "";
	char linechar[24] = "";
	size_t lineLen = 0;
	uint8_t byte;
	const uint8_t *data = pData;

	printf ("---------------------------------------------------------------------------\n");
	printf ("Base: 0x%08lx, length %d(0x%04x)\n", (unsigned long)(data), size, size);
	printf ("----  +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F    01234567 89ABCDEF\n");
//	printf ("---------------------------------------------------------------------------\n");
	
	for (tmp = 0; 
		(tmp + 16) <= size; 
		tmp += 16)
	{
		memset(lineString, 0, sizeof(lineString));
		memset(linechar, 0, sizeof(linechar));
	
		for (column = 0, lineLen = 0;
			column < 16;
			column ++)
		{
			byte = data[tmp + column];
			sprintf(lineString + lineLen, "%02X ", byte & 0xFF);
			
			lineLen += 3;

			if (column < 7)
			{
				linechar[column] = _charFromByte(byte);
			}
			else if (7 == column)
			{
				linechar[column] = _charFromByte(byte);
				linechar[column+1] = ' ';
				sprintf(lineString + lineLen, " ");
				lineLen += 1;
			}
			else
			{
				linechar[column+1] = _charFromByte(byte);
			}
		}

		printf ("%04X: %s   %s\n", tmp, lineString, linechar);
	}

	/* last line */
	if (tmp < size)
	{
		memset(lineString, 0, sizeof(lineString));
		memset(linechar, 0, sizeof(linechar));
	
		for (column = 0, lineLen = 0;
			column < (size - tmp);
			column ++)
		{
			byte = data[tmp + column];
			sprintf(lineString + lineLen, "%02X ", byte & 0xFF);
			lineLen += 3;

			if (column < 7)
			{
				linechar[column] = _charFromByte(byte);
			}
			else if (7 == column)
			{
				linechar[column] = _charFromByte(byte);
				linechar[column+1] = ' ';
				sprintf(lineString + lineLen, " ");
				lineLen += 1;
			}
			else
			{
				linechar[column+1] = _charFromByte(byte);
			}
		}
#if 1
		for (/* null */;
			column < 16;
			column ++)
		{
			sprintf(lineString + lineLen, "   ");
			lineLen += 3;
		
			if (7 == column)
			{
				sprintf(lineString + lineLen, " ");
				lineLen += 1;
			}
		}
#endif
		printf ("%04X: %s   %s\n", tmp, lineString, linechar);
	}
	
	printf ("---------------------------------------------------------------------------\n");
	
	/* ends */
}


/* simple semaphore operation functions */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#if (OS_TYPE_MAC_OS_X != CFG_OS_TYPE)
union semun{
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
	unsigned short  *array;  /* Array for GETALL, SETALL */
 	struct seminfo  *__buf;  /* Buffer for IPC_INFO (Linux specific) */
};
#endif
int simpleSemCreate(key_t key)
{
	return semget(key, 1, 0666 | IPC_CREAT);
}

int simpleSemSet(int semId)
{
	union semun semUnion;
	semUnion.val = 1;
	return semctl(semId, 0, SETVAL, semUnion);
}

int simpleSemDelete(int semId)
{
	union semun semUnion;

	return semctl(semId, 0, IPC_RMID, semUnion);
}

int simpleSem_P(int semId)
{
	struct sembuf semB;

	semB.sem_num = 0;
	semB.sem_op = -1;	/* P() */
	semB.sem_flg = SEM_UNDO;

	return semop(semId, &semB, 1);
}

int simpleSem_V(int semId)
{
	struct sembuf semB;

	semB.sem_num = 0;
	semB.sem_op = 1;	/* V() */
	semB.sem_flg = SEM_UNDO;

	return semop(semId, &semB, 1);
}


/* simple shared memory functions */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
int simpleShmCreate(key_t key, size_t size)
{
	return shmget(key, size, 0666 | IPC_CREAT);
}

void *simpleShmLink(int memId)
{
	return shmat(memId, NULL, 0);
}

int simpleShmUnlink(const void *shmAddr)
{
	return shmdt(shmAddr);
}

int simpleShmDelete(int shmId)
{
	return shmctl(shmId, IPC_RMID, NULL);
}


/* simple message queue functions */
ssize_t getSystemMsgmax(void)
{
	FILE *procFile;
	ssize_t msgmax;

	procFile = fopen("/proc/sys/kernel/msgmax", "r");
	if (NULL == procFile)
	{
		return -1;
	}

#if ((OS_TYPE_IPHONE == CFG_OS_TYPE) || (OS_TYPE_MAC_OS_X == CFG_OS_TYPE))
	fscanf(procFile, "%ld", &msgmax);
#else
	fscanf(procFile, "%d", &msgmax);
#endif

	fclose(procFile);

	if (0 == msgmax)
	{
		return -1;
	}
	else
	{
		return msgmax;
	}
}

ssize_t getSystemMsgmnb(void)
{
	FILE *procFile;
	ssize_t msgmnb;

	procFile = fopen("/proc/sys/kernel/msgmnb", "r");
	if (NULL == procFile)
	{
		return -1;
	}
	
#if ((OS_TYPE_IPHONE == CFG_OS_TYPE) || (OS_TYPE_MAC_OS_X == CFG_OS_TYPE))
		fscanf(procFile, "%ld", &msgmnb);
#else
	fscanf(procFile, "%d", &msgmnb);
#endif

	fclose(procFile);

	if (0 == msgmnb)
	{
		return -1;
	}
	else
	{
		return msgmnb;
	}
}

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
int simpleMsqCreate(key_t key)
{
	return msgget(key, 0666 | IPC_CREAT);
}

int simpleMsqDelete(int msqId)
{
	return msgctl(msqId, IPC_RMID, NULL);
}

int simpleMsqSend(int msqId, long int msgType, void * pMsg, size_t size)
{
	void *msgBuff = NULL;
	static ssize_t msgmax = 0;
	int sendStat = 0;
	int errnoCopy;

	if (NULL == pMsg)
	{
		errno = EINVAL;
		return -1;
	}

	if (0 == msgmax)
	{
		msgmax = getSystemMsgmax();
		if (msgmax <= 0)
		{
			return -1;
		}
	}

	if ((ssize_t)size > msgmax)
	{
		errno = EMSGSIZE;
		return -1;
	}

	msgBuff = malloc(size + sizeof(long int));
	if (NULL == msgBuff)
	{
		return -1;
	}

	*((long int *)msgBuff) = msgType;
	memcpy(msgBuff+ sizeof(long int), pMsg, size);	
	
	sendStat = msgsnd(msqId, msgBuff, size, 0);
	errnoCopy = errno;

	free(msgBuff);
	errno = errnoCopy;

	return sendStat;
}

ssize_t simpleMsqRecv(int msqId, long int *pMsgType, void * pMsg, size_t size, int isNoWait)
{
	void *msgBuff = NULL;
	static ssize_t msgmax = 0;
	ssize_t recvStat = 0;
	int errnoCopy;

	if (NULL == pMsgType || NULL == pMsg)
	{
		errno = EINVAL;
		return 0;
	}

	if (0 == msgmax)
	{
		msgmax = getSystemMsgmax();
		if (msgmax <= 0)
		{
			return -1;
		}
	}

	if ((ssize_t)size > msgmax)
	{
		errno = EMSGSIZE;
		return -1;
	}

	msgBuff = malloc(size + sizeof(long int));
	if (NULL == msgBuff)
	{
		return -1;
	}

	if (0 != isNoWait)
	{
		isNoWait = IPC_NOWAIT;
	}
	
	recvStat = msgrcv(msqId, msgBuff, size, *pMsgType, isNoWait);
	errnoCopy = errno;

	if (recvStat >= 0)
	{
		*pMsgType = *((long int *)msgBuff);
		memcpy(pMsg, msgBuff + sizeof(long int), recvStat);
	}

	free(msgBuff);
	errno = errnoCopy;

	if ((ENOMSG == errno) && (-1 == recvStat))
	{
		return 0;
	}
	else
	{
		return recvStat;
	}
}


/* simple socket interface */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>		/* AF_UNIX */
#include <netinet/in.h>		/* AF_INET */
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
static unsigned int _socketLibErrLine = 0;
#define	SOCK_PRESET_ERR_LINE()		_socketLibErrLine = __LINE__
unsigned int simpleSocketErrLine(void)
{
	return _socketLibErrLine;
}
int simpleSocketCreate_local(void)
{
	return socket(AF_UNIX, SOCK_STREAM, 0);
}

int simpleSocketCreate_tcp(void)
{
	return socket(AF_INET, SOCK_STREAM, 0);
}

int simpleSocketCreate_udp(void)
{
	return socket(AF_INET, SOCK_DGRAM, 0);
}

int simpleSocketName_local(int socketId, const char *socketName)
{
	struct sockaddr_un address;

	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, socketName, sizeof(address.sun_path));
	return bind(socketId, (struct sockaddr *)&address, sizeof(address));
}

int simpleSocketName_tcp(int socketId, const char *serverIpAddr, int port)
{
	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(serverIpAddr);
	address.sin_port = htons(port);
	return bind(socketId, (struct sockaddr *)&address, sizeof(address));
}

int simpleSocketListen(int socketId, int pendingLimit)
{
	return listen(socketId, pendingLimit);
}

int simpleSocketAccept_local(int socketId, const char *socketName, int isNoWait)
{
	struct sockaddr_un address;
	int funcCallStat;
	long flags = fcntl(socketId, F_GETFL, NULL);
	socklen_t sockLen = sizeof(address);

	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, socketName, sizeof(address.sun_path));
	
	if (0 != isNoWait)
	{
		SOCK_PRESET_ERR_LINE();
		funcCallStat = fcntl(socketId, F_SETFL, (flags |O_NONBLOCK));
	}
	else
	{
		SOCK_PRESET_ERR_LINE();
		funcCallStat = fcntl(socketId, F_SETFL, (flags & (~O_NONBLOCK)));
	}

	if (-1 == funcCallStat)
	{
		return -1;
	}

	SOCK_PRESET_ERR_LINE();
	return accept(socketId, (struct sockaddr *)&address, &sockLen);
}

int simpleSocketAccept_internet(int socketId, const char *serverIpAddr, int port, int isNoWait)
{
	struct sockaddr_in address;
	int funcCallStat;
	long flags = fcntl(socketId, F_GETFL, NULL);
	socklen_t sockLen = sizeof(address);

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(serverIpAddr);
	address.sin_port = htons(port);
	
	if (0 != isNoWait)
	{
		SOCK_PRESET_ERR_LINE();
		funcCallStat = fcntl(socketId, F_SETFL, (flags |O_NONBLOCK));
	}
	else
	{
		SOCK_PRESET_ERR_LINE();
		funcCallStat = fcntl(socketId, F_SETFL, (flags & (~O_NONBLOCK)));
	}

	if (-1 == funcCallStat)
	{
		return -1;
	}

	SOCK_PRESET_ERR_LINE();
	return accept(socketId, (struct sockaddr *)&address, &sockLen);
}

int simpleSocketConnect_local(int socketId, const char *socketName)
{
	struct sockaddr_un address;

	address.sun_family = AF_UNIX;
	strncpy(address.sun_path, socketName, sizeof(address.sun_path));
	return connect(socketId, (struct sockaddr *)&address, sizeof(address));
}

int simpleSocketConnect_internet(int socketId, const char *serverIpAddr, int port, const struct timeval *timeout)
{
	struct sockaddr_in address;
	int funcCallStat;
	long flags = fcntl(socketId, F_GETFL, NULL);
	fd_set rSet, wSet;
	int getsockoptErrno = 0;
	socklen_t getsockoptLen = sizeof(getsockoptErrno);
	struct timeval timeoutCopy;

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(serverIpAddr);
	address.sin_port = htons(port);

	if (!timeout)	// Wait forever
	{
		SOCK_PRESET_ERR_LINE();
		return connect(socketId, (struct sockaddr *)&address, sizeof(address));
	}

	// else: get a timeout
	timeoutCopy.tv_sec = timeout->tv_sec;
	timeoutCopy.tv_usec = timeout->tv_usec;
	SOCK_PRESET_ERR_LINE();
	funcCallStat = fcntl(socketId, F_SETFL, (flags |O_NONBLOCK));
	if (-1 == funcCallStat)
	{
		goto RETURN;
	}

	// connect
	funcCallStat = connect(socketId, (struct sockaddr*)&address, sizeof(address));
	if (0 == funcCallStat)
	{
		SOCK_PRESET_ERR_LINE();
		goto RETURN;	// Success immediately
	}
	else if ((errno != EINPROGRESS) && (funcCallStat < 0))
	{
		// other system error
		goto RETURN;
	}
	else
	{
		FD_ZERO(&rSet);
		FD_SET(socketId, &rSet);
		wSet = rSet;

		funcCallStat = select(socketId + 1, &rSet, &wSet, NULL, &timeoutCopy);
		if (-1 == funcCallStat)	// System camm error
		{
			SOCK_PRESET_ERR_LINE();
			goto RETURN;
		}
		else if (0 == funcCallStat)		// connect timeout
		{
			SOCK_PRESET_ERR_LINE();
			errno = ETIME;
			funcCallStat = -1;
			goto RETURN;
		}
		else
		{
			if (!FD_ISSET(socketId, &rSet) &&
				!FD_ISSET(socketId, &wSet))
			{
				SOCK_PRESET_ERR_LINE();	// Connect() fail
				funcCallStat = -1;
				goto RETURN;
			}

			funcCallStat = getsockopt(socketId, SOL_SOCKET, SO_ERROR, &getsockoptErrno, &getsockoptLen);
			if (funcCallStat < 0)
			{
				SOCK_PRESET_ERR_LINE();	// getsockopt() fail
				goto RETURN;
			}

			if (0 != getsockoptErrno)
			{
				SOCK_PRESET_ERR_LINE();	// connect() fail
				goto RETURN;
			}

			// Finally, success
			funcCallStat = 0;
		}

	}

	

RETURN:
	fcntl(socketId, F_SETFL, flags);	// set back
	return funcCallStat;
}

ssize_t simpleSocketWrite_tcp(int socketId, const void *data, size_t dataLen)
{
	return write(socketId, data, dataLen);
}

ssize_t simpleSocketRead_tcp(int socketId, void *dataRead, size_t dataReadLimit, const struct timeval *timeout)
{
	struct timeval timeoutCopy;
	fd_set fd;
	int funcCallStat;

	/* prameter check */
	if (NULL == dataRead)
	{
		errno = EINVAL;
		return -1;
	}

	/* check if we want to block and wait forever */
	if (NULL != timeout)
	{
		memcpy(&timeoutCopy, timeout, sizeof(struct timeval));

		FD_ZERO(&fd);
		FD_SET(socketId, &fd);

		SOCK_PRESET_ERR_LINE();
		funcCallStat = select(socketId + 1, &fd, NULL, NULL, &timeoutCopy);
		if (-1 == funcCallStat)
		{
			return -1;
		}

		if (!FD_ISSET(socketId, &fd))	/* no responce get */
		{
			SOCK_PRESET_ERR_LINE();
			return 0;
		}
		else
		{
			/* continue to recvfrom() below */
		}
	}

	/* read */
	SOCK_PRESET_ERR_LINE();
	return read(socketId, dataRead, dataReadLimit);
}

ssize_t simpleSocketSendto_udp(int socketId, const char *targetIPAddr, int port, const void *data, size_t dataLen)
{
	int tmp;
	int setOptStat;
	struct sockaddr_in dstAddr;

	//printf("Now sendto %s.\n", serverIPAddr);

	/* broadcast */
	if (0 == memcmp("255.255.255.255", targetIPAddr, 15))
	{
		//printf ("Broadcast.\n");
		tmp = 1;
		SOCK_PRESET_ERR_LINE();
		setOptStat = setsockopt(socketId, SOL_SOCKET, SO_BROADCAST, (void *)&tmp, sizeof(tmp));
		if (-1 == setOptStat)
		{
			//printf("setsockopt() failed.\n");
			return setOptStat;
		}
		else
		{
			//printf("setsockopt() OK.\n");
		}
		
		dstAddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	}
	else
	{
		SOCK_PRESET_ERR_LINE();
		setOptStat = inet_pton(AF_INET, targetIPAddr, (void *)(&(dstAddr.sin_addr.s_addr)));
		if (-1 == setOptStat)
		{
			return setOptStat;
		}
	}

	/* sendto */
	dstAddr.sin_family = AF_INET;
	dstAddr.sin_port = htons(port);
	SOCK_PRESET_ERR_LINE();
	return sendto(socketId, data, dataLen, 0, (struct sockaddr*)&dstAddr, sizeof(struct sockaddr_in));
}


int simpleSocketBind_udp(int socketId, int portFrom)
{
	struct sockaddr_in address;
	
	address.sin_family = AF_INET;
	address.sin_port = htons(portFrom);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	return bind(socketId, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
}


int simpleSocketBind_tcp(int socketId, int portFrom)
{
	struct sockaddr_in address;
	
	address.sin_family = AF_INET;
	address.sin_port = htons(portFrom);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	return bind(socketId, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
}

ssize_t simpleSocketReceiveFrom_udp
							(int socketId, 
							const char *sourceIpAddr, 
							struct sockaddr_in *srcAddress,
							void *dataRead, 
							size_t dataReadLimit, 
							const struct timeval *timeout)
{
	int funcCallStat;
	struct timeval timeoutCopy;
	fd_set fd;
	socklen_t len;	/* used in recvfrom */

	/* prameter check */
	if ((NULL == srcAddress) || 
		(NULL == dataRead))
	{
		errno = EINVAL;
		return -1;
	}

	/* convert source IP addr */
	if ((NULL == sourceIpAddr) || 
		(0 == strcmp(sourceIpAddr, "")))
	{
		/* ip address any */
		srcAddress->sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		SOCK_PRESET_ERR_LINE();
		funcCallStat = inet_pton(AF_INET, sourceIpAddr, (void *)(&(srcAddress->sin_addr.s_addr)));
		if (-1 == funcCallStat)
		{
			return (ssize_t)funcCallStat;
		}
	}
	srcAddress->sin_family = AF_INET;

	/* check if we want to block and wait forever */
	if (NULL != timeout)
	{
		memcpy(&timeoutCopy, timeout, sizeof(struct timeval));

		FD_ZERO(&fd);
		FD_SET(socketId, &fd);

		SOCK_PRESET_ERR_LINE();
		funcCallStat = select(socketId + 1, &fd, NULL, NULL, &timeoutCopy);
		if (-1 == funcCallStat)
		{
			return -1;
		}

		if (!FD_ISSET(socketId, &fd))	/* no responce get */
		{
			SOCK_PRESET_ERR_LINE();
			return 0;
		}
		else
		{
			/* continue to recvfrom() below */
		}
	}

	/* receive from */
	len = sizeof(*srcAddress);
	SOCK_PRESET_ERR_LINE();
	return recvfrom(socketId, (void *)dataRead, dataReadLimit, 0, (struct sockaddr *)srcAddress, &len);
}


int simpleSocketClose(int socketId)
{
	close(socketId);
	return 0;
}

// TODO: socket lib not finished, I think...


/* network interface */
#include <sys/types.h>
#include <ifaddrs.h>
int isSystemBigEndian()
{
	static const union {
		uint32_t i;
		char c[4];
	}bint = {0x01020304};
	return bint.c[0] == 1;
}
#if defined(__APPLE__) && defined(__MACH__)
#else
uint64_t htonll(uint64_t hostlonglong)
{
	union {
		uint32_t u32[2];
		uint64_t u64;
	} ret;

	if (isSystemBigEndian())
	{
		return hostlonglong;
	}
	else
	{
		ret.u32[0] = htonl((uint32_t)(hostlonglong >> 32));
		ret.u32[1] = htonl((uint32_t)(hostlonglong & 0x00000000FFFFFFFF));
		return ret.u64;
	}
}

uint64_t ntohll(uint64_t netlonglong)
{
	union {
		uint32_t u32[2];
		uint64_t u64;
	} ret;
	
	if (isSystemBigEndian())
	{
		return netlonglong;
	}
	else
	{
		ret.u32[0] = ntohl((uint32_t)(netlonglong >> 32));
		ret.u32[1] = ntohl((uint32_t)(netlonglong & 0x00000000FFFFFFFF));
		return ret.u64;
	}
}
#endif

size_t ifaddrsGetAllIPv4(const struct ifaddrs *ifa, 
                           struct in_addr *ipBuffer, 
                           struct in_addr *maskBuffer,
                           size_t bufferCountMax)
{
	size_t tmp;
	const struct ifaddrs *tmpPIfAddr;

	if (NULL == ifa)
	{
		return 0;
	}
	

	/* parameters OK */
	for (tmpPIfAddr = ifa, tmp = 0; 
		(NULL != tmpPIfAddr) && (tmp < bufferCountMax);
		tmpPIfAddr = tmpPIfAddr->ifa_next)
	{
		if (AF_INET == (tmpPIfAddr->ifa_addr->sa_family))
		{
			if (ipBuffer){
				ipBuffer[tmp] = ((struct sockaddr_in*)(tmpPIfAddr->ifa_addr))->sin_addr;
			}

			if (maskBuffer){
				maskBuffer[tmp] = ((struct sockaddr_in*)(tmpPIfAddr->ifa_netmask))->sin_addr;
			}
			
			tmp ++;
		}
		else
		{}
	}

	/* ends */
	return tmp;
}


void inet_n4top(struct in_addr addr, char* str, size_t strLenLimit)
{
	if (NULL == str)
	{
		return;
	}

	inet_ntop(AF_INET, &addr, str, strLenLimit);
	return;
}


char *sock_ntop(const struct sockaddr *sa, socklen_t salen)
{
	char portstr[8];
	static char str[128];		/* UNIX domain is largest */

	if (NULL == sa)
	{
		return NULL;
	}
	
	switch (sa->sa_family)
 	{
 		case AF_INET:
		{
			struct sockaddr_in *sin = (struct sockaddr_in *)sa;
			if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str))==NULL)
			{
				return NULL;
			}
			if (ntohs(sin->sin_port)!=0)
			{
				snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin_port));
				strcat(str, portstr);
			}
			return str;
		}
			break;

		case AF_INET6:
		{
			struct sockaddr_in6 *sin = (struct sockaddr_in6 *)sa;
			if (inet_ntop(AF_INET6, &sin->sin6_addr, str, sizeof(str))==NULL)
			{
				return NULL;
			}
			if (ntohs(sin->sin6_port)!=0)
			{
				snprintf(portstr, sizeof(portstr), ".%d", ntohs(sin->sin6_port));
				strcat(str, portstr);
			}
			return str;
		}
			break;

		default:
			return NULL;
			break;
 	}
	return NULL;
}



/* simple timer configuration */
#if (OS_TYPE_LINUX == CFG_OS_TYPE)
#include <signal.h>
#include <time.h>
/* needs -lrt flag */
/* private global functions */
void (*timeOutFunc)(void);
static timer_t simpleTimer;
static struct sigevent simpleTimerSig;		/* dummy */
static struct itimerspec simpleTimerConf;
static bool_t isTimerCreated = FALSE;			/* adoid duplicated timer */

static void hddIdleTimerReset()
{
	timer_settime(simpleTimer, 0, &simpleTimerConf, NULL);
	if (NULL != timeOutFunc)
	{
		timeOutFunc();
	}
	
	return;
}

int simpleTimerCreate(int sec, int nsec, void (*timeOutCallback)(void))
{
	int funcCallStat;

	if (isTimerCreated)
	{
		return -1;
	}

	memset(&simpleTimer, 0, sizeof(simpleTimer));
	memset(&simpleTimerSig, 0, sizeof(simpleTimerSig));
	memset(&simpleTimerConf, 0, sizeof(simpleTimerConf));

	simpleTimerSig.sigev_notify = SIGEV_THREAD;
	simpleTimerSig.sigev_notify_function = hddIdleTimerReset;
	simpleTimerConf.it_value.tv_sec = sec;
	simpleTimerConf.it_value.tv_nsec = nsec;

	timeOutFunc = timeOutCallback;

	funcCallStat = timer_create(CLOCK_REALTIME, &simpleTimerSig, &simpleTimer);
	if (0 != funcCallStat)
	{
		return funcCallStat;
	}

	funcCallStat = timer_settime(simpleTimer, 0, &simpleTimerConf, NULL);
	if (0 != funcCallStat)
	{
		return funcCallStat;
	}

	isTimerCreated = TRUE;
	return funcCallStat;
}

int simpleTimerStop()
{
	if (isTimerCreated)
	{
		simpleTimerConf.it_interval.tv_sec = 0;
		simpleTimerConf.it_interval.tv_nsec = 0;
		simpleTimerConf.it_value.tv_sec = 0;
		simpleTimerConf.it_value.tv_nsec = 0;
		isTimerCreated = FALSE;
		return timer_settime(simpleTimer, 0, &simpleTimerConf, NULL);
	}
	else
	{
		return 0;
	}
}
#elif (OS_TYPE_MAC_OS_X == CFG_OS_TYPE)
int simpleTimerCreate(int sec, int nsec, void (*timeOutCallback)(void))
{
	errno = ENOSYS;
	return -1;
}

int simpleTimerStop()
{
	errno = ENOSYS;
	return -1;
}
#endif


