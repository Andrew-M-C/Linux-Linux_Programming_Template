/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_MEM
#include "AMCCommonLib.h"


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test associative array"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"

#include "AMCDictionary.h"

#define _TO_PTR(value)		((void *)(value))


/**********/
/* main */
#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	size_t count = 3;
	AMCDictionary_st *dict = AMCDictionary_New(NULL);
	if (NULL == dict) {
		return -1;
	}

	AMCDictionary_SetObject(dict, "14B", "Li", FALSE, NULL);
	AMCDictionary_SetObject(dict, "12C", "Wang", FALSE, NULL);
	AMCDictionary_SetObject(dict, "14B", "Yong", FALSE, NULL);
	AMCDictionary_SetObject(dict, "14B", "Andrew", FALSE, NULL);
	AMCDictionary_SetObject(dict, "12A", "Cong", FALSE, NULL);
	AMCDictionary_SetObject(dict, "11C", "Liu", FALSE, NULL);
	AMCDictionary_SetObject(dict, "12A", "Lily", FALSE, NULL);
	AMCDictionary_SetObject(dict, "11C", "Zhan", FALSE, NULL);
	AMCDictionary_SetObject(dict, "12A", "Foo", FALSE, NULL);
	AMCDictionary_DumpStatus(dict, NULL);

	AMCPrintf("Level for Andrew: %s", (const char *)AMCDictionary_GetObject(dict, "Andrew"));
	
	
	/****/
	/* ENDS */
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

