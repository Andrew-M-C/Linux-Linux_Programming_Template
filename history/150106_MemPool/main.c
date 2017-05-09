/**/

#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_FILE
#define CFG_LIB_RAND
#define CFG_LIB_TIME
#define CFG_LIB_THREAD
#include "AMCCommonLib.h"
#include "AMCDataTypes.h"

#include "AMCMemPool.h"



#include <termios.h>
/* Initialize new terminal i/o settings */
static struct termios old, new;
void initTermios(int echo) 
{
	tcgetattr(0, &old); /* grab old terminal i/o settings */
	new = old; /* make new settings same as old settings */
	new.c_lflag &= ~ICANON; /* disable buffered i/o */
	new.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
	tcsetattr(0, TCSANOW, &new); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void) 
{
	tcsetattr(0, TCSANOW, &old);
}


/* Read 1 character - echo defines echo mode */
char getch_(int echo) 
{
	char ch;
	initTermios(echo);
	ch = getchar();
	resetTermios();
	return ch;
}

/* Read 1 character without echo */
char getch() 
{
	return getch_(0);
}

/* Read 1 character with echo */
char getche() 
{
	return getch_(1);
}


static void _printUsage(void)
{
	printf("Usage:\n");
	printf("\ta:\tAllocate a new unit.\n");
}



/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	long tmp;
	struct AMCMemPool *pool = AMCMemPool_Create(16, 5, 3, TRUE);
	struct AMCMemUnit *unitArray[20];

	if (NULL == pool)
	{
		AMCPrintf ("Cannot create mempry pool");
	}
	else
	{
		AMCMemPool_DebugStdout(pool);
	}


	for (tmp = 0; tmp < 14; tmp++)
	{
		unitArray[tmp] = AMCMemPool_Alloc(pool);
	}
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[6]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[0]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[8]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[5]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[7]);
	AMCMemPool_DebugStdout(pool);

	unitArray[6] = AMCMemPool_Alloc(pool);
	AMCMemPool_DebugStdout(pool);

	unitArray[8] = AMCMemPool_Alloc(pool);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[11]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[12]);
	AMCMemPool_DebugStdout(pool);

	AMCMemPool_Free(unitArray[13]);
	AMCMemPool_DebugStdout(pool);
	

	AMCMemPool_Destory(pool);
	pool = NULL;

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
	//TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

