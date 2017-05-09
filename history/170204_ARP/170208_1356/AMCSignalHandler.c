/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: signalHandler.c
	Description: 	
			This file provide container to create signal processor.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-10: File created as "configParser.c"
		2012-10-10: Change file name as "AMCConfigParser.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include <signal.h>	/* signal */
#include <pthread.h>	/* thread */
#include <stdio.h>		/* printf() */
#include <stdlib.h>	/* exit() */
#include <errno.h>

static int sigNum = 0;
pthread_t hdlthreadSignalProcessor = 0;
pthread_mutex_t signalMutex = PTHREAD_MUTEX_INITIALIZER;

typedef void (*sighandler_t)(int);
static int staticSimpleSigaction(int signum, sighandler_t act)
{
	struct sigaction sigConf = {};

	if (NULL == act)
	{
		errno = EIO;
		return -1;
	}

	sigemptyset(&(sigConf.sa_mask));
	sigConf.sa_handler = act;
	sigConf.sa_flags = 0;
	

	return sigaction(signum, &sigConf, NULL);
}

static void staticSignalCatcher(int sig)
{
	pthread_mutex_lock(&signalMutex);
	sigNum = sig;
	//printf ("Catch signal %d.\n", sigNum);
	pthread_mutex_unlock(&signalMutex);
}

static void *threadSignalProcessor(void *arg)
{
	int sigNumCopy = 0;

	//printf ("Signal monitor initialized.\n");

	do
	{
		if (0 != sigNum)
		{
			pthread_mutex_lock(&signalMutex);
			sigNumCopy = sigNum;
			sigNum = 0;
			pthread_mutex_unlock(&signalMutex);
			switch (sigNumCopy)
			{
				case SIGALRM:
					printf("<SIGALRM>\n");
					break;
				case SIGFPE:
					printf ("<SIGFPE>\n");
					exit(1);
					break;
				case SIGHUP:
					printf ("<SIGHUP>\n");
					exit(1);
					break;
				case SIGILL:
					printf ("<SIGILL>\n");
					exit(1);
					break;
				case SIGINT:
					printf ("<SIGINT>\n");
					exit(1);
					break;
				case SIGPIPE:
					printf ("<SIGPIPE>\n");
					exit(1);
					break;
				case SIGQUIT:
					printf ("<SIGQUIT>\n");
					exit(1);
					break;
				case SIGSEGV:
					printf ("<SIGSEGV>\n");
					exit(1);
					break;
				case SIGTERM:
					printf ("<SIGTERM>\n");
					exit(1);
					break;
				case SIGUSR1:
					printf ("<SIGUSR1>\n");
					exit(1);
					break;
				case SIGUSR2:
					printf ("<SIGUSR2>\n");
					exit(1);
					break;
				default:
					printf ("Catch unknown signal %d.\n", sigNumCopy);
					exit(1);
					break;
			}
		}
		else
		{
			/* continue */
		}
	}
	while(1);
}

int AMCSignalRegister()
{
	int funcCallStat;
	int tmp;

	const int validSignal[] = {
		SIGALRM, 
		SIGFPE,
		SIGHUP,
		SIGILL,
		SIGINT,
		SIGPIPE,
		SIGQUIT,
		SIGSEGV,
		SIGTERM,
		SIGUSR1,
		SIGUSR2,
	};

	//alarm(5);
	/**/
	funcCallStat = pthread_mutex_init(&signalMutex, NULL);
	if (0 != funcCallStat)
	{
		return -1;
	}

	funcCallStat = pthread_create(&hdlthreadSignalProcessor, NULL, threadSignalProcessor, NULL);
	if (0 != funcCallStat)
	{
		return -1;
	}

	/* register signal capture */
	for (tmp = 0; tmp < (sizeof(validSignal) / sizeof(int)); tmp++)
	{
		//sigaction(validSignal[tmp], &sigConf, NULL);
		funcCallStat = staticSimpleSigaction(validSignal[tmp], staticSignalCatcher);
		if (funcCallStat < 0)
		{
			break;
		}
	}

	return funcCallStat;
}

