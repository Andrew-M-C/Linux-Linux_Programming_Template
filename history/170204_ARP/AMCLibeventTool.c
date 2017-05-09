/*******************************************************************************
	Copyright (C), 2011-2014, Andrew Min Chang
	
	File name: AMCLibeventTool.c
	Description: 	
			This file provide implementations of AMCLibeventTool.h.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2014-12-25: File created as "AMCLibeventTool.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCLibeventTool.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <pthread.h>


/**********/
#define __PRIVATE_MACROS
#ifdef __PRIVATE_MACROS

#define _LOCK_IF_COND(pLock, condition)		if (condition) pthread_mutex_lock(pLock)
#define _UNLOCK_IF_COND(pLock, condition)	if (condition) pthread_mutex_unlock(pLock)
#define _COPY_TIMEVAL(pTo, pFrom)			(pTo)->tv_sec = (pFrom)->tv_sec; (pTo)->tv_usec = (pFrom)->tv_usec

#endif


/**********/
#define __PUBLIC_EVENT_BASE_INTERFACES
#ifdef __PUBLIC_EVENT_BASE_INTERFACES


struct event_base *AMCEventBase_New()
{
	return event_base_new();
}


int AMCEventBase_Free(struct event_base *base)
{
	event_base_free(base);
	return 0;
}

int AMCEventBase_Dispatch(struct event_base *base)
{
	return event_base_dispatch(base);
}


int AMCEventBase_Stop(struct event_base *base, BOOL immediately)
{
	if (immediately)
	{
		return event_base_loopbreak(base);
	}
	else
	{
		return event_base_loopexit(base, NULL);
	}
}


int AMCEventBase_DumpStdout(struct event_base *base)
{
	FILE *out = fopen("/dev/stdout", "w");
	if (out)
	{
		event_base_dump_events(base, out);
		fclose(out);
		return 0;
	}
	else
	{
		return -1;
	}
}



#endif


/**********/
#define __PUBLIC_EVENT_INTERFACES
#ifdef __PUBLIC_EVENT_INTERFACES
struct event *AMCEvent_TcpListenNew(
		struct event_base *base, 
		int bindPort, 
		int listenBacklog, 
		BOOL shouldPersist, 
		event_callback_fn acceptCallback, 
		void *callbackArg)
{
	int fd = 0;
	int status = 0;
	struct event *retEv = NULL;

	/****/
	/* check parameters */
	if (NULL == base)
	{
		return NULL;
	}

	/****/
	/* create socket and make non-blocking */
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd)
	{
		status = evutil_make_socket_nonblocking(fd);
	}
	else
	{
		status = -1;
	}

	/****/
	/* bind socket */
	if ((bindPort > 0) && (0 == status))
	{
		struct sockaddr_in address;
	
		address.sin_family = AF_INET;
		address.sin_port = htons(bindPort);
		address.sin_addr.s_addr = htonl(INADDR_ANY);		
	
		status = bind(fd, (struct sockaddr *)&address, sizeof(struct sockaddr_in));
	}

	/****/
	/* listen */
	if ((bindPort > 0) && (0 == status))
	{
		status = listen(fd, listenBacklog);
	}

	/****/
	/* set event and callback */
	if (0 == status)
	{
		retEv = event_new(base, fd, EV_READ | (shouldPersist ? EV_PERSIST : 0), acceptCallback, callbackArg);
	}


	/****/
	/* return */
	if (0 != status)
	{
		if (retEv)
		{
			event_free(retEv);
			retEv = NULL;
		}

		if (fd)
		{
			close(fd);
		}
	}
	return retEv;
}


int AMCEvent_Add(struct event *event, const struct timeval *timeout)
{
	return event_add(event, timeout);
}


int AMCEvent_Del(struct event *event)
{
	return event_del(event);
}


int AMCEvent_Free(struct event *event)
{
	int fd = event_get_fd(event);
	event_free(event);

	if (fd > 2)
	{
		return close(fd);
	}
	else
	{
		return 0;
	}
}


int AMCEvent_DumpString(struct event *event, char *string, size_t lenLimit)
{
#define _CAT_STRING_PART()	if (len < remainLen){remainLen -= len;strcat(string, strBuff);}else{ret = -1;}do{}while(0)
	size_t remainLen = lenLimit;
	char strBuff[64];
	const int BUFFLEN = sizeof(strBuff - 1);
	int len;
	short what;
	int ret = 0;

	if ((NULL == event) || (NULL == string) || (0 == lenLimit))
	{
		return -1;
	}
	string[0] = '\0';

	/****/
	/* event address */
	len = snprintf(strBuff, BUFFLEN, "Event 0x%08lx", (unsigned long)event);
	_CAT_STRING_PART();

	/****/
	/* event socket fd */
	len = snprintf(strBuff, BUFFLEN, " [fd %d]", event_get_fd(event));
	_CAT_STRING_PART();


	/****/
	/* event type */
	what = event_get_events(event);
	
	if (what & EV_TIMEOUT)
	{
		len = snprintf(strBuff, BUFFLEN, " Timeout");
		_CAT_STRING_PART();
	}

	if (what & EV_READ)
	{
		len = snprintf(strBuff, BUFFLEN, " Read");
		_CAT_STRING_PART();
	}

	if (what & EV_WRITE)
	{
		len = snprintf(strBuff, BUFFLEN, " Write");
		_CAT_STRING_PART();
	}

	if (what & EV_SIGNAL)
	{
		len = snprintf(strBuff, BUFFLEN, " Signal");
		_CAT_STRING_PART();
	}

	if (what & EV_PERSIST)
	{
		len = snprintf(strBuff, BUFFLEN, " Persist");
		_CAT_STRING_PART();
	}

	if (what & EV_ET)
	{
		len = snprintf(strBuff, BUFFLEN, " Edge");
		_CAT_STRING_PART();
	}

	/****/
	/* return */
	if (0 == ret)
	{
		return lenLimit - remainLen;
	}
	else
	{
		return ret;
	}
#undef _CAT_STRING_PART
}


int AMCEvent_DumpStdout(struct event *event)
{
	int ret;
	char strBuff[128]; 	// should be enough
	ret = AMCEvent_DumpString(event, strBuff, sizeof(strBuff));
	printf("%s\n", strBuff);
	return (ret < 0) ? -1 : 0;
}

#endif



/**********/
#define __PUBLIC_BUFFEREVENT_INTERFACES
#ifdef __PUBLIC_BUFFEREVENT_INTERFACES

struct bufferevent *AMCBufferevent_New(struct event_base *base, BOOL shouldDeferCallbacks, BOOL isThreadSafe)
{
	int flags = 0;

	flags |= BEV_OPT_CLOSE_ON_FREE;
	flags |= shouldDeferCallbacks ? BEV_OPT_DEFER_CALLBACKS : 0;
	flags |= isThreadSafe ? BEV_OPT_THREADSAFE : 0;
	
	return bufferevent_socket_new(base, -1, flags);
}


struct bufferevent *AMCBufferevent_NewAndConfig(
		struct event_base *base, 
		BOOL shouldDeferCallbacks, 
		BOOL isThreadSafe, 
		bufferevent_data_cb readCallback, 
		const struct timeval *timeoutRead, 
		bufferevent_data_cb writeCallback, 
		const struct timeval *timeoutWrite, 
		bufferevent_event_cb eventCallback, 
		void *callbackArg)
{
	struct bufferevent *buffev = AMCBufferevent_New(base, shouldDeferCallbacks, isThreadSafe);

	if (buffev)
	{
		short readWriteFlag = 0;
		readWriteFlag |= readCallback ? EV_READ : 0;
		readWriteFlag |= writeCallback ? EV_WRITE : 0;
		
		bufferevent_setcb(buffev, readCallback, writeCallback, eventCallback, callbackArg);
		bufferevent_enable(buffev, readWriteFlag);

		if (timeoutRead || timeoutWrite)
		{
			bufferevent_set_timeouts(buffev, timeoutRead, timeoutWrite);
		}
	}

	return buffev;
}


int AMCBufferevent_Connect(struct bufferevent *buffEv, int targetPort, char *targetIPv4)
{
	struct sockaddr_in targetAddr;

	if ((NULL == buffEv) || 
		(NULL == targetIPv4) ||
		(targetPort <= 0) ||
		(targetPort > 65535))
	{
		return -1;
	}

	targetAddr.sin_family = AF_INET;
	targetAddr.sin_port = htons(targetPort);
	inet_aton(targetIPv4, &(targetAddr.sin_addr));

	return bufferevent_socket_connect(buffEv, (struct sockaddr *)&targetAddr, sizeof(targetAddr));
}


int AMCBufferevent_Free(struct bufferevent *buffEv)
{
	bufferevent_free(buffEv);
	return 0;
}

#endif


/**********/
#define __PUBLIC_EVENT_INTERFACES
#ifdef __PUBLIC_EVENT_INTERFACES

struct AMCTimingEvent{
	struct AMCTimingEvent   *pNext;
	struct AMCTimingEvent   *pPrev;
	struct AMCTimingEventBase *pBase;
	AMCTimingEventConf_t    flag;
	timing_callback_fn      callback;
	struct timeval          currTimer;
	struct timeval          presetTimer;
	BOOL                    isTimeoutFlag;
	BOOL                    isRepeat;
	void                    *arg;
};

struct AMCTimingEventBase{
	AMCTimingEventBaseConf_t flag;
	BOOL                     isRequestedFree;
	pthread_mutex_t          lock;
	pthread_t                thread;
	struct timeval           timeIntvl;
	struct AMCTimingEvent    *pEvents;
	struct event_base        *pTimingBase;
	struct event             *pTimingEvent;
};


static void _timingEventCallback(evutil_socket_t fd, short what, void *arg)
{
	struct AMCTimingEventBase *pAmcBase = (struct AMCTimingEventBase *)arg;
	BOOL useLock = ((pAmcBase->flag) & AMCTmEvBseCfg_DoNotUseLock) ? FALSE : TRUE;
	struct timeval startTime, endTime, tmpTime;
	struct AMCTimingEvent *pAmcEv = NULL;

	evutil_gettimeofday(&startTime, NULL);		/* callback start */

	/* update timers */
	_LOCK_IF_COND(&(pAmcBase->lock), useLock);		/* ---LOCK--- */
	if(1)
	{
		for (pAmcEv = pAmcBase->pEvents; pAmcEv; pAmcEv = pAmcEv->pNext)
		{
			if (evutil_timercmp(&(pAmcEv->currTimer), &(pAmcBase->timeIntvl), >=))
			{
				pAmcEv->isTimeoutFlag = FALSE;
				evutil_timersub(&(pAmcEv->currTimer), &(pAmcBase->timeIntvl), &tmpTime);
				_COPY_TIMEVAL(&(pAmcEv->currTimer), &tmpTime);
			}
			else
			{
				pAmcEv->isTimeoutFlag = TRUE;
				pAmcEv->currTimer.tv_sec  = 0;
				pAmcEv->currTimer.tv_usec = 0;
			}
		}
	}
	_UNLOCK_IF_COND(&(pAmcBase->lock), useLock);	/* --UNLOCK-- */
	

	/* invoke events */
	pAmcEv = pAmcBase->pEvents;
	while (pAmcEv)
	{
		if (pAmcEv->isTimeoutFlag)
		{
			(pAmcEv->callback)(pAmcEv, pAmcEv->arg);
			pAmcEv->isTimeoutFlag = FALSE;

			if ((pAmcEv->flag) & AMCTmEvCfg_Repeat)
			{
				AMCTimingEvent_ResetTimeout(pAmcEv);
				pAmcEv = pAmcEv->pNext;
			}
			else
			{
				struct AMCTimingEvent *pTmpEv = pAmcEv;
				pAmcEv = pAmcEv->pNext;
				AMCTimingEvent_Remove(pTmpEv);
			}
		}
		else
		{
			pAmcEv = pAmcEv->pNext;
		}
	}
	

	/* ends */
	evutil_gettimeofday(&endTime, NULL);		/* callbask ends */
	

	// TODO:
}


static void *_threadDispatchBase(void *arg)
{
	struct AMCTimingEventBase *pAmcBase = (struct AMCTimingEventBase *)arg;
	pthread_detach(pthread_self());

	/* dispatch */
	event_base_dispatch(pAmcBase->pTimingBase);

	/* deallocate */
	event_del(pAmcBase->pTimingEvent);
	event_free(pAmcBase->pTimingEvent);
	event_base_free(pAmcBase->pTimingBase);
	free(pAmcBase);

	return NULL;
}


struct AMCTimingEventBase *AMCTimingEventBase_New(AMCTimingEventBaseConf_t flag, const struct timeval *timeInterval)
{
	const struct timeval dftTimeIntvl = {.tv_sec = 1, .tv_usec = 0};
	struct AMCTimingEventBase *pBase = NULL;
	int callStat = 0;

	if (NULL == timeInterval)
	{
		timeInterval = &dftTimeIntvl;
	}

	pBase = malloc(sizeof(*pBase));
	if (NULL == pBase)
	{
		return NULL;
	}

	/* initialize parameters */
	pBase->flag = flag;
	pBase->isRequestedFree = FALSE;
	pBase->pEvents = NULL;
	pBase->pTimingBase = NULL;
	pBase->pTimingEvent = NULL;
	memcpy(&(pBase->timeIntvl), timeInterval, sizeof(*timeInterval));
	
	if (flag & AMCTmEvBseCfg_DoNotUseLock)
	{}
	else
	{
		callStat = pthread_mutex_init(&(pBase->lock), NULL);
		if (0 == callStat)
		{
			free(pBase);
			return NULL;
		}
	}

	if (0 == callStat)
	{
		pBase->pTimingBase = event_base_new();
		if (pBase->pTimingBase)
		{
			pBase->pTimingEvent = evtimer_new(pBase->pTimingBase, _timingEventCallback, pBase);
			if (pBase->pTimingEvent)
			{
				callStat = evtimer_add(pBase->pTimingEvent, timeInterval);
			}
			else
			{
				callStat = 0;
			}
		}	
		else
		{
			callStat = -1;
		}
	}	

	/* check and run dispatch thread */
	if (0 == callStat)
	{
		callStat = pthread_create(&(pBase->thread), NULL, _threadDispatchBase, (void *)pBase);
	}

	/* return */
	if (0 == callStat)
	{}
	else
	{
		if (flag & AMCTmEvBseCfg_DoNotUseLock)
		{}
		else
		{
			pthread_mutex_destroy(&(pBase->lock));
		}


		if (pBase->pTimingBase)
		{
			event_base_free(pBase->pTimingBase);
		}

		free(pBase);
		pBase = NULL;
	}
	return pBase;
}


int AMCTimingEventBase_Free(struct AMCTimingEventBase *timeEvBase)
{
	return -1;
}



struct AMCTimingEvent *AMCTimingEvent_New(AMCTimingEventConf_t flag, const struct timeval *timeout)
{
	return NULL;
}


int AMCTimingEvent_Free(struct AMCTimingEvent *timeEvent)
{
	return -1;
}


int AMCTimingEvent_ResetTimeout(struct AMCTimingEvent *timeEvent)
{
	return -1;
}


int AMCTimingEvent_SetTimeout(struct AMCTimingEvent *timeEvent, const struct timeval *timeout)
{
	return -1;
}


int AMCTimingEvent_TriggerTimeout(struct AMCTimingEvent *timeEvent)
{
	return -1;
}


int AMCTimingEvent_Add(struct AMCTimingEvent *timeEvent, struct AMCTimingEventBase *timeEvBase)
{
	return -1;
}


int AMCTimingEvent_Remove(struct AMCTimingEvent *timeEvent)
{
	return -1;
}



#endif




