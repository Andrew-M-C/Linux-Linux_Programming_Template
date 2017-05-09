/**/

#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_FILE
#define CFG_LIB_RAND
#define CFG_LIB_TIME
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
	AMCPrintf("Usage:");
	AMCPrintf("\tc: Send random number.");
	AMCPrintf("\ti: Information.");
	AMCPrintf("\tt: Temp test command.");
	AMCPrintf("\tq: Quit.");
}


//static struct bufferevent *_tmpBev = NULL;

/******/
/* bufferevent operation */

static void _writeCb(struct bufferevent *bev, void *arg)
{
	AMCPrintf("[fd %d] Write done.", bufferevent_getfd(bev));
	//AMCBufferevent_Free(bev);
}

static void _eventCb(struct bufferevent *bev, short what, void *arg)
{
	if (what & BEV_EVENT_CONNECTED)
	{
		/* write */
		struct evbuffer *buffer = bufferevent_get_output(bev);
		if (buffer)
		{
			int num = randInt(1000, 99999999);
			//AMCEvent_DumpStdout(arg);
			evbuffer_add_printf(buffer, "Data from Andrew Chang: %d.", num);
			AMCPrintf("[fd %d] Send number %d.", bufferevent_getfd(bev), num);
		}
		else
		{
			AMCPrintf("Failed to get evbuffer");
		}
	}
	else if (what & BEV_EVENT_ERROR)
	{
		AMCPrintf ("Cannot connect to remote server.");
	}
	else
	{
		AMCPrintf("Get unknown event 0x%04x", what);
	}
}

static void _startConnect(struct event_base *base)
{
	const struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
	struct bufferevent *bev = AMCBufferevent_NewAndConfig(base, FALSE, FALSE, NULL, NULL, _writeCb, &timeout,_eventCb, base);

	if (bev)
	{
		//_tmpBev = bev;
		AMCBufferevent_Connect(bev, _SERVER_PORT, "127.0.0.1");
		AMCEventBase_Dispatch(base);
	}
	else
	{
		AMCPrintf("Create bufferevent error");
	}
}



/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	char cmd;
	struct event_base *base = AMCEventBase_New();
	srand(time(NULL));

	if (NULL == base)
	{
		return -1;
	}
	
	
	do
	{
		_printUsage();
		cmd = (char)getche();
		
		switch(cmd)
		{
			case 'c':
				_startConnect(base);
				break;
			case 'i':
				AMCPrintf("Event base status:");
				AMCEventBase_DumpStdout(base);
				break;

			case 't':
				AMCPrintf("nop");
				//bufferevent_enable(_tmpBev, EV_WRITE);
				//_eventCb(_tmpBev, BEV_EVENT_CONNECTED, base);
				break;

			case 'q':
				break;
			default:
				AMCPrintf("Unknown key %d", cmd);
				break;
		}
	}
	while('q' != cmd);

	AMCEventBase_Stop(base, TRUE);
	AMCEventBase_Free(base);

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

