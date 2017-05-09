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

#include "AMCTimingTool.h"


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


static void _cbNewEvent(struct AMCTimingSecObject *obj, void *arg)
{
	struct _CbArg *cbArg = (struct _CbArg *)arg;
	AMCLog("Callback! %d", cbArg->id);

	cbArg->count --;

	if (cbArg->count <= 0)
	{
		AMCLog("Timeout, destroy self");
		free(arg);
		AMCTimingSecond_DelObject(obj);
		AMCTimingSecond_DebugStdout(cbArg->tool);
	}
	return;
}



static void _cbOneSecondRespond(struct AMCTimingSecObject *obj, void *arg)
{
	struct _CbArg *cbArg = (struct _CbArg *)arg;
	AMCLog("Callback! %d", cbArg->id);

	cbArg->count --;

	if (0 == cbArg->count)
	{
		AMCLog("Timeout, we should add a new event");
		struct _CbArg *newArg = malloc(sizeof(*newArg));
		newArg->id = 2;
		newArg->count = 2;
		newArg->tool = cbArg->tool;
		if (newArg)
		{
			AMCTimingSecond_AddObject(cbArg->tool, _cbNewEvent, 1, TmCfg_Repeated, newArg, NULL);
		}
		else
		{
			AMCLog("Malloc failed");
		}
		AMCTimingSecond_DebugStdout(cbArg->tool);
	}
	
	return;
}


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	long count = AMC_TM_1_SECOND * 10;
	struct _CbArg arg[2];
	struct AMCTimingSecTool *secTool = AMCTimingSecond_New(8, NULL);
	struct AMCTimingSecObject *basicObj;

	if (NULL == secTool)
	{
		return -1;
	}

	AMCLog("Mark");
	arg[0].id = 1;
	arg[0].count = 3;
	arg[0].tool = secTool;
	basicObj = AMCTimingSecond_AddObject(secTool, _cbOneSecondRespond, 1, TmCfg_Repeated, &(arg[0]), NULL);
	AMCLog("Mark");
	AMCTimingSecond_Start(secTool);

	
	/* OBJ 1 */
	while(count > 0)
	{
		usleep(AMC_TM_1_SECOND);
		count -= AMC_TM_1_SECOND;
	}
	usleep(100);

	AMCLog("Mark");
	AMCTimingSecond_DelObject(basicObj);
	AMCTimingSecond_DebugStdout(secTool);
	AMCTimingSecond_Destroy(secTool);
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

