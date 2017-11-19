/* file encoding: UTF-8 */

#if defined (__cplusplus)
extern "C" {
#endif

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_PID
#include "AMCCommonLib.h"

#include <setjmp.h>

#if defined (__cplusplus)
}
#endif

#include <string>
#include <vector>


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test det(A)"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"


extern "C"  int asm_swap(unsigned long *pValA, unsigned long *pValB);


static void _func_A()
{
    for (unsigned tmp = 0; tmp < 10; tmp ++)
    {
        AMCPrintf("A: tmp = %02u", tmp);
        usleep(5000000);
    }
    return;
}


static void _func_B()
{
    for (unsigned tmp = 0; tmp < 10; tmp ++)
    {
        AMCPrintf("B: tmp = %02u", tmp);
        usleep(5000000);
    }
    return;
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
#else
#error Upsupported CPU archetecture.
#endif

    unsigned long valA = 111;
    unsigned long valB = 888;

    AMCPrintf("valA = %lu", valA);
    AMCPrintf("valB = %lu", valB);

    asm_swap(&valA, &valB);

    AMCPrintf("valA = %lu", valA);
    AMCPrintf("valB = %lu", valB);

	/****/
	/* ENDS */
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

	mainRet = _true_main(argc, argv);
	TEST_VALUE(mainRet);
	
	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

