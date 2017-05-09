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

#include "AMCLibeventTool.h"

#define _SERVER_PORT	1070
#define _SERVER_IP		"127.0.0.1"


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
	printf("\ts: Start a new 5-sec timing procedure.\n");
	printf("\ti: Timing information.\n");
	printf("\tr: Reset timer to 5 seconds.\n");
	printf("\tt: Stop current timing procedure.\n");
	printf("\tq: Quit.\n");
}


static struct event *_dummyEvent = NULL;
static BOOL _shouldRemoveDummy = FALSE;

static void _addDummyEvent(struct event_base *);
static void _dummyCallback(evutil_socket_t fd, short what, void *arg)
{
	AMCPrintf("Dummy event [fd %d] elapsed, argument: 0x%04x.\n", fd, what);
	AMCPrintf("Status:");
	AMCEventBase_DumpStdout((struct event_base *)arg);

	if (_shouldRemoveDummy)
	{
		/* do nothing */
		//event_del(_dummyEvent);
	}
	else
	{
		_addDummyEvent((struct event_base *)arg);
	}
	
	return;
}

static void _addDummyEvent(struct event_base *base)
{
	const struct timeval dummyTime = {.tv_sec = 5, .tv_usec = 0};
	struct event *dummyEvent = evtimer_new(base, _dummyCallback, base);
	
	AMCPrintf("Add event [0x%08x --> 0x%08x]", _dummyEvent, dummyEvent);
	if (_dummyEvent)
	{
		event_del(_dummyEvent);
	}
	_dummyEvent = dummyEvent;
	evtimer_add(dummyEvent, &dummyTime);
}

static void *_threadEventBaseDispatch(void *arg)
{
	struct event_base *base = (struct event_base*)arg;
	_addDummyEvent(base);
	event_base_dispatch(base);
	AMCPrintf("ERROR: event dispatch ends!!!");
	return NULL;
}


static void _requestRemoveEventBase(struct event_base *base)
{
	struct timeval timeout;
	struct timeval currTime;
	struct timeval timeRemain;
	int callStat;
	AMCPrintf("Event base status:\n");
	AMCEventBase_DumpStdout(base);

	_shouldRemoveDummy = TRUE;
	AMCPrintf("Let's trigger the event 0x%08x", (unsigned int)_dummyEvent);

	evutil_gettimeofday(&currTime, NULL);
	callStat = event_pending(_dummyEvent, EV_TIMEOUT, &timeout);
	evutil_timersub(&timeout, &currTime, &timeRemain);
	if (callStat)
	{
		AMCPrintf("Event pending at time %d.%lld.", timeRemain.tv_sec, timeRemain.tv_usec);
	}
	else
	{
		AMCPrintf("Event non-pending at time %d.%lld.", timeRemain.tv_sec, timeRemain.tv_usec);
	}
	
	//event_active(_dummyEvent, EV_TIMEOUT, 0);
	//event_del(_dummyEvent);
	AMCEventBase_Stop(base, TRUE);
}



/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	char cmd;
	struct event_base *base = AMCEventBase_New();
	pthread_t eventThread;
	int callStat;


	if (NULL == base)
	{
		return -1;
	}

	callStat = pthread_create(&eventThread, NULL, _threadEventBaseDispatch, base);
	if (0 != callStat)
	{
		AMCEventBase_Free(base);
		return -1;
	}
	
	
	do
	{
		_printUsage();
		cmd = (char)getche();
		puts("\n");
		
		switch(cmd)
		{
			case 's':
				break;
			case 't':
				break;
			case 'r':
				break;
			case 'i':
				AMCPrintf("Event base status:");
				AMCEventBase_DumpStdout(base);
				break;

			case 'q':
				break;
			default:
				AMCPrintf("Unknown key %d", cmd);
				break;
		}
	}
	while('q' != cmd);

	AMCPrintf("Now quit program.");
	_requestRemoveEventBase(base);
	AMCPrintf("Event request removed.");
	AMCEventBase_Free(base);

	pthread_join(eventThread, NULL);

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

