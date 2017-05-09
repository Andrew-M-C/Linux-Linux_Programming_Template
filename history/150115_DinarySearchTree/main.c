/**/

#define DEBUG
#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_FILE
#define CFG_LIB_RAND
#define CFG_LIB_TIME
#define CFG_LIB_THREAD
#define CFG_LIB_SLEEP
#include "AMCCommonLib.h"
#include "AMCDataTypes.h"

#include "AMCTree.h"


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

#if 0
static void _printUsage(void)
{
	printf("Usage:\n");
	printf("\ta:\tAllocate a new unit.\n");
}
#endif

struct _CbArg {
	long id;
	long count;
	struct AMCTimingSecTool *tool;
};

#define _INSERT_NODE(pTree, id)		printf("==========\n+ %ld:\n", (unsigned long)(id)); AMCStaticRbTree_Insert((pTree), (id), NULL);AMCStaticRbTree_DebugStdout(pTree)
#define _DELETE_NODE(pTree, id)		printf("==========\n- %ld:\n", (unsigned long)(id)); AMCStaticRbTree_Delete((pTree), (id));AMCStaticRbTree_DebugStdout(pTree)

/*
uint64_t fibonacci(uint64_t input, uint64_t inputMinusOne)
{
	if (0 == input)
	{
		return 0;
	}
	else if (1 == input)
	{
		return 1;
	}
	else
	{
		return fibonacci(input - 1) + fibonacci(input - 2);
	}
}
*/

/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	unsigned long tmp;
	struct AMCStaticRedBlackTree *tree = AMCStaticRbTree_New(16, 8, NULL);

	_INSERT_NODE(tree, 50);
	_INSERT_NODE(tree, 11);
	_INSERT_NODE(tree, 30);
	_INSERT_NODE(tree, 99);
	_INSERT_NODE(tree, 60);
	_INSERT_NODE(tree, 75);
	_INSERT_NODE(tree, 59);
	_INSERT_NODE(tree, 70);
	_INSERT_NODE(tree, 77);
	_INSERT_NODE(tree, 65);
	_INSERT_NODE(tree, 13);
	_INSERT_NODE(tree, 55);
	_INSERT_NODE(tree, 43);
	_INSERT_NODE(tree, 42);
	_INSERT_NODE(tree, 44);
	_INSERT_NODE(tree, 41);

	_DELETE_NODE(tree, 43);
	_DELETE_NODE(tree, 55);
	_DELETE_NODE(tree, 60);
	_DELETE_NODE(tree, 30);

/*
	for (tmp = 0; tmp < 100; tmp++)
	{
		AMCPrintf("Start %ld...", tmp);
		TEST_64B_VALUE(fibonacci(tmp));
		AMCPrintf("Ends");
	}
*/

	AMCStaticRbTree_Destroy(tree);

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

