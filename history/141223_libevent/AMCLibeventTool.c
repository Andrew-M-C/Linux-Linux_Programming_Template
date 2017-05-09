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

	flags |= shouldDeferCallbacks ? BEV_OPT_DEFER_CALLBACKS : 0;
	flags |= isThreadSafe ? BEV_OPT_THREADSAFE : 0;
	
	return bufferevent_socket_new(base, -1, flags);
}


struct bufferevent *AMCBufferevent_NewAndConfig(
		struct event_base *base, 
		BOOL shouldDeferCallbacks, 
		BOOL isThreadSafe, 
		bufferevent_data_cb readCallback, 
		bufferevent_data_cb writeCallback, 
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


#endif




