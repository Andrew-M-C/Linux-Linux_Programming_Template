/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_THREAD
#include "AMCCommonLib.h"
#include "AMCThreadPool.h"

static void _count_down_task(void *arg)
{
	int tmp = 10;
	while(tmp >= 0)
	{
		AMCPrintf("%s: Now %d\n", (char*)arg, tmp);
		tmp --;
		sleep(1);
	}
}

static void *_count_down_task_test(void *arg)
{
	pthread_detach(pthread_self());

	_count_down_task(arg);

	pthread_exit(NULL);
	return NULL;
}


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	struct AMCThreadPool *threadPool = AMCThreadPool_Create(10);

	AMCPrintf("Thread pool: %p", threadPool);
	AMCThreadPool_AddTask(threadPool, _count_down_task, "AAAA");

	sleep(1);
	MARK();
	AMCThreadPool_AddTask(threadPool, _count_down_task, "BBBB");


	sleep(2);
	MARK();
	AMCThreadPool_Destroy(threadPool);
	threadPool = NULL;
	
	sleep(10);
	MARK();

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

