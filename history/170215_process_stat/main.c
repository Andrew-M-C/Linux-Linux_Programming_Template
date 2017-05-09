/**/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <sys/time.h>

/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test CPU usage tool"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"
#define CFG_MAC_AMC_PRINT_SIZE 1024

#include "AMCCpuUsage.h"

#define _TO_PTR(value)		((void *)(value))


/**********/
/* main */


ssize_t AMCPrintf(const char *format, ...)
{
	char buff[CFG_MAC_AMC_PRINT_SIZE];
	va_list vaList;
	size_t dateLen = 0;
	
	tzset();
	time_t currSec = time(0);
	struct tm currTime;
	struct tm *pTime = localtime(&currSec);
	struct timeval currDayTime;

	gettimeofday(&currDayTime, NULL);
	if (pTime)
	{
		memcpy(&currTime, pTime, sizeof(currTime));
		dateLen = sprintf(buff, "%04d-%02d-%02d,%02d:%02d:%02d.%06ld ", 
							currTime.tm_year + 1900, currTime.tm_mon + 1, currTime.tm_mday,
							currTime.tm_hour, currTime.tm_min, currTime.tm_sec, currDayTime.tv_usec);
	}

	va_start(vaList, format);
	vsnprintf((char *)(buff + dateLen), sizeof(buff) - dateLen - 1, format, vaList);
	va_end(vaList);

	dateLen = strlen(buff);
	buff[dateLen + 0] = '\n';
	buff[dateLen + 1] = '\0';

	return (write(1, buff, dateLen + 1));
}


#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	if (argc < 2) {
		return -1;
	}

	AMCCpuUsageErrno_st error = {0, 0};
	AMCCpuUsage_st *usage = NULL;
	pid_t pid = strtol(argv[1], NULL, 10);
	AMCPrintf("Get pid %d", pid);

	int sleepSec = 1;
	if (argc >= 3) {
		sleepSec = strtol(argv[2], NULL, 10);
		if (sleepSec <= 0) {
			sleepSec = 1;
		}
	}

	usage = AMCCpuUsage_New(pid, &error);
	while(0 == error.obj_error)
	{
		error = AMCCpuUsage_Update(usage);

		if (0 == error.obj_error) {
			AMCPrintf("[%d] CPU %ld%%", pid, AMCCpuUsage_GetCpuUsagePercent(usage));
			AMCPrintf("[%d] CPU %.03lf%%", pid, AMCCpuUsage_GetCpuUsageDouble(usage) * 100.0);
		}
	
		sleep(sleepSec);
	}
	
	
	
	/****/
	/* ENDS */
	if (usage) {
		AMCCpuUsage_Free(usage);
		usage= NULL;
	}
	if (0 != error.obj_error) {
		AMCPrintf("Error: %d", error.obj_error);
	}
	return 0;
}


int main(int argc, char* argv[])
{
	int mainRet;

	AMCPrintf (CFG_SOFTWARE_DISCRIPT_STR);
	AMCPrintf ("Version "CFG_SOFTWARE_VER_STR);
	AMCPrintf ("Author: "CFG_AUTHOR_STR);
	AMCPrintf ("main() build time: "__TIME__", "__DATE__);
	AMCPrintf ("----START----");

	mainRet = trueMain(argc, argv);
	
	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

