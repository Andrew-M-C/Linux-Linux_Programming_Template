/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_TIME
#define CFG_LIB_SOCKET
#define CFG_LIB_DEVICE
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DNS
#define CFG_LIB_NET
#define CFG_LIB_STRING
#define CFG_LIB_SIGNAL
#include "AMCCommonLib.h"

#include "AMCRandom.h"


/**********/
/* main */
#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	AMCPrintf("Random Long: %d", AMCRandomInt(-100, 100));
	AMCPrintf("Random Long: %d", AMCRandomInt(0, 0x7FFFFFFF));
	AMCPrintf("Random Long: %d", AMCRandomInt(0, 0x7FFFFFFF));
	AMCPrintf("Random Long: %d", AMCRandomInt(0, 0x7FFFFFFF));
	AMCPrintf("RAND_MAX = 0x%08lX", RAND_MAX);
	AMCPrintf("RAND_MIN = 0x%08lX", -RAND_MAX);
	
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

