/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_MEM
#include "AMCCommonLib.h"

#include "AMCArray.h"

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
	size_t count = 0;
	AMCArray_st *array = AMCArray_New(NULL);
	if (NULL == array) {
		return -1;
	}

	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCArray_AddObject(array, _TO_PTR(count++));
	AMCAray_DumpStatus(array, NULL);

	AMCArray_InsertObject(array, _TO_PTR(count++), 0);
	AMCAray_DumpStatus(array, NULL);
		
	AMCArray_InsertObject(array, _TO_PTR(count++), 5);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_InsertObject(array, _TO_PTR(count++), 2);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_RemoveObject(array, 3, FALSE);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_RemoveObject(array, 5, FALSE);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_RemoveObject(array, 0, FALSE);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_RemoveObject(array, 6, FALSE);
	AMCAray_DumpStatus(array, NULL);

	AMCArray_RemoveObject(array, 5, FALSE);
	AMCAray_DumpStatus(array, NULL);
	
	
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

