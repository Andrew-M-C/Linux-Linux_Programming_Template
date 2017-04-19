/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_SLEEP
#define CFG_LIB_MEM
#define CFG_LIB_SIGNAL
#include "AMCCommonLib.h"


static void _sig_quit(int signum)
{
	static int callCount = 0;

	AMCPrintf("[%02d] Got signal: %s", callCount, strsignal(signum));
	callCount ++;

	if (callCount >= 10) {
		exit(0);
	}

	return;
}


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

	callStat = signal(SIGQUIT, SIG_IGN);
	if (callStat < 0) {
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

