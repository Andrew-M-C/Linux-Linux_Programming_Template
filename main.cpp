/* file encoding: UTF-8 */

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_PID
#define CFG_LIB_MMAP
#define CFG_LIB_STRING
#define CFG_LIB_SLEEP
#include "AMCCommonLib.h"

#include "AMCCoroutine.h"

#include <string>
#include <vector>


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test det(A)"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"

/*******************************************************
 * coroutines
 */

static void *_func_A(void *arg)
{
    AMCPrintf("A stack: %p", AMCGetStackAddr());

    for (unsigned tmp = 0; tmp < 10; tmp ++)
    {
        AMCPrintf("A: %u", tmp);
        usleep(100000);
        AMCCoroutineSchedule();
    }

    AMCPrintf("A fin");
    return NULL;
}


static void *_func_B(void *arg)
{
    AMCPrintf("B stack: %p", AMCGetStackAddr());

    for (unsigned tmp = 0; tmp < 5; tmp ++)
    {
        AMCPrintf("B: %u", tmp);
        usleep(100000);
        AMCCoroutineSchedule();
    }

    AMCPrintf("B fin");
    return NULL;
}


/**********/
/* main */
static int _true_main(int argc, char* argv[])
{
#if defined (__amd64__)
    AMCPrintf("Hello AMD64");
#elif defined (__x86_64__)
    AMCPrintf("Hello x86_64");
#elif defined(__i386__)
    AMCPrintf("Hello i386");
#error Upsupported CPU archetecture.
#else
#error Upsupported CPU archetecture.
#endif

    int ret = 0;
    AMCPrintf("Main stack: %p", AMCGetStackAddr());

    AMCCoroutineAdd(_func_A, NULL, 0, "_func_A()");
    AMCCoroutineAdd(_func_B, NULL, 0, "_func_B()");

    AMCPrintf("Pre: Stack pointer: %p", AMCGetStackAddr());
    ret = AMCCoroutineRun();
    AMCPrintf("Aft: Stack pointer: %p", AMCGetStackAddr());

    return ret;
}


int main(int argc, char* argv[])
{
	int mainRet;

	AMCPrintf (CFG_SOFTWARE_DISCRIPT_STR);
	AMCPrintf ("Version "CFG_SOFTWARE_VER_STR);
	AMCPrintf ("Author: "CFG_AUTHOR_STR);
	AMCPrintf ("main() build time: "__TIME__", "__DATE__);
	AMCPrintf ("----START----");

	mainRet = _true_main(argc, argv);
	TEST_VALUE(mainRet);

	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

