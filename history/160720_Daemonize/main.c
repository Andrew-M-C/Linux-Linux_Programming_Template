/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#include "AMCCommonLib.h"


/**********/
/* main */
#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	int pid = -1;

	AMCAppDaemonize(FALSE);

	if (AMCAppAlreadyRunning("/home/falcon/projects/MyStudy/MyWork/160720_Daemonize/amc/tmp", "tmp.pid", &pid))
	{
		AMCPrintf("Pid %d already running", pid);
		return -1;
	}
	else
	{
		AMCPrintf("Got pid: %d", pid);
	}	

	while (1)
	{
		AMCPrintf("Sleep...");
		sleep(1);
	}
	
	/****/
	/* ENDS */
ENDS:

	return 0;
}


int main(int argc, char* argv[])
{
	int mainRet;

	printf ("\n"CFG_SOFTWARE_DISCRIPT_STR"\n");
	printf ("Version "CFG_SOFTWARE_VER_STR"\n");
	printf ("Author: "CFG_AUTHOR_STR"\n");
	printf ("main() build time: "__TIME__", "__DATE__"\n");
	printf ("----START----\n");

	mainRet = trueMain(argc, argv);
	TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

