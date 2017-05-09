/**/

#define DEBUG
#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_FILE
#define CFG_LIB_DIR
#define CFG_LIB_RAND
#define CFG_LIB_TIME
#define CFG_LIB_THREAD
#define CFG_LIB_SLEEP
#define CFG_LIB_STRING
#define CFG_LIB_ERRNO
#define CFG_LIB_DEVICE
#define CFG_LIB_SYSTEM
#define CFG_LIB_PID
#define CFG_LIB_FORK
#include "AMCCommonLib.h"
#include "AMCDataTypes.h"

#define _CFG_TMP_FILE_PATH_A	"/tmp/log.txt"
#define _CFG_TMP_FILE_PATH_B	"/tmp/patch.patch"


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	ssize_t tmp, fdCount;
	int fdArray[256];

	/* get pid */
	pid_t selfPid = getpid();
	AMCPrintf("Pid: %d", selfPid);

	/* open a temporilary file */
	int fdA = open(_CFG_TMP_FILE_PATH_A, O_CREAT | O_RDONLY);
	AMCPrintf("Open file description %d.", fdA);

	int fdB = open(_CFG_TMP_FILE_PATH_B, O_CREAT | O_RDONLY);
	AMCPrintf("Open file description %d.", fdB);

	/* check current file descriptors */
	AMCPrintf("All opened file descriptors:");
	fdCount = getAllFds(fdArray, 256);
	for (tmp = 0; tmp < fdCount; tmp++)
	{
		printf("%d, ", fdArray[tmp]);
	}
	printf("\n");

	/* close fd */
	closeAllFds();
	//close(fdA);
	//close(fdB);

	/* final check */
	AMCPrintf("All opened file descriptors:");
	fdCount = getAllFds(fdArray, 256);
	for (tmp = 0; tmp < fdCount; tmp++)
	{
		printf("%d, ", fdArray[tmp]);
	}
	printf("\n");

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

