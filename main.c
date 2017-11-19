/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_STDIN
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_SLEEP
#define CFG_LIB_MEM
#define CFG_LIB_SIGNAL
#include "AMCCommonLib.h"


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test CPU usage tool"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"


/**********/
/* main */
static int trueMain(int argc, char* argv[])
{
	int retryTimes = 5;
	sigset_t newMask, oldMask, pendMask;
	int callStat = 0;

	sigemptyset(&newMask);
	sigemptyset(&oldMask);
	sigemptyset(&pendMask);

	if (SIG_ERR == signal(SIGQUIT, SIG_IGN))
	{
		AMCPrintErr("Failed in signal(): %s", strerror(errno));
		goto END;
	}

	sigaddset(&newMask, SIGQUIT);

	for (/**/; retryTimes > 0; retryTimes --)
	{
		callStat = sigprocmask(SIG_BLOCK, &newMask, &oldMask);
		if (callStat < 0) {
			AMCPrintErr("Failed in sigprocmask(): %s", strerror(errno));
			goto END;
		}

		AMCPrintf("Sleeping starts");
		sleep(5);
		AMCPrintf("Sleeping stops");

		callStat = sigpending(&pendMask);
		if (callStat < 0) {
			AMCPrintErr("Failed in sigpending(): %s", strerror(errno));
			goto END;
		}

		if (sigismember(&pendMask, SIGQUIT)) {
			AMCPrintf("Signal SIGQUIT pending");
		}

		callStat = sigprocmask(SIG_SETMASK, &oldMask, NULL);
		if (callStat < 0) {
			AMCPrintErr("Failed in sigprocmask(): %s", strerror(errno));
			goto END;
		}
	}
	
	/****/
	/* ENDS */
END:
	return (0 == callStat) ? 0 : -1;
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
	TEST_VALUE(mainRet);
	
	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

