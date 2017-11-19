/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_STDIN
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DAEMON
#define CFG_LIB_SLEEP
#define CFG_LIB_PID
#define CFG_LIB_FORK
#define CFG_LIB_SIGNAL
#include "AMCCommonLib.h"


/*******************************************************
 * software information
 */
#define	CFG_SOFTWARE_NAME_STR		"(no name)"
#define	CFG_SOFTWARE_DISCRIPT_STR	"An application to test CPU usage tool"
#define	CFG_SOFTWARE_VER_STR		"0.0.1"
#define	CFG_AUTHOR_STR		"Andrew Chang"

#include "AMCCpuUsage.h"

#define _TO_PTR(value)		((void *)(value))

/*--------------------------------------------------*/
static void _remove_new_line_char(char *line)
{
	BOOL isOK = FALSE;

	for (isOK = FALSE; FALSE == isOK; line++)
	{
		switch(*line)
		{
			case '\r':
			case '\n':
				*line = '\0';
				isOK = TRUE;
				break;
			case '\0':
				isOK = TRUE;
				break;
			default:
				break;
		}
	}

	return;
}


/*--------------------------------------------------*/
static void _try_fork()
{
	static int count = 0;
	pid_t pid = 0;

	pid = vfork();
	switch(pid)
	{
		case PID_FAIL:
			AMCPrintErr("Failed to fork: %s");
			break;
		case PID_CHILD:
			count ++;
			pid = getpid();
			AMCPrintf("Child pid: %d, count: %d", (int)pid, count);
			exit(0);
			break;
		default:
			AMCPrintf("Parent pid: %d", (int)pid);
			break;
	}

	return;
}



/**********/
/* main */
#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	size_t lineCap = 1024;
	char *line = malloc(lineCap);
	ssize_t lineSize = 0;
	char input = '\0';
	BOOL shouldExit = FALSE;

	signal(SIGCHLD, SIG_IGN);

	AMCPrintf("Parent pid: %d", (int)getpid());

	if (NULL == line) {
		AMCPrintErr("Cannot allocate buffer: %s", strerror(errno));
	}

	while(FALSE == shouldExit)
	{
		AMCPrintf("Please input option:");
		
		lineSize = getline(&line, &lineCap, stdin);
		if (lineSize < 0)
		{
			AMCPrintErr("Failed to getline: %s", strerror(errno));
			shouldExit = TRUE;
		}
		else if (0 == lineSize)
		{
			AMCPrintf("Exit");
			shouldExit = TRUE;
		}
		/* Handle option */
		else
		{
			_remove_new_line_char(line);
			AMCPrintf("Get line: %s", line);
			_try_fork();
		}
	}


	AMCPrintf("Get char: %c", input);

	if (line)
	{
		free(line);
		line = NULL;
	}
	return 0;
}


int main(int argc, char* argv[])
{
	int mainRet;

	AMCPrintf (CFG_SOFTWARE_DISCRIPT_STR);
	AMCPrintf ("Version "CFG_SOFTWARE_VER_STR);
	AMCPrintf ("Author: "CFG_AUTHOR_STR);
	AMCPrintf ("main() build time: "__TIME__", "__DATE__);
	AMCPrintf ("----START----");

	mainRet = trueMain(argc, argv);
	TEST_VALUE(mainRet);
	
	AMCPrintf ("----ENDS-----");
	exit(mainRet);
	return 0;
}

