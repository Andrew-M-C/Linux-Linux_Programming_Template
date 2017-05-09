/**/

#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define CFG_LIB_MEM
#define CFG_LIB_DNS
#define CFG_LIB_NET
#define CFG_LIB_SOCKET
#define CFG_LIB_MALLOC
#define CFG_LIB_STRING
#define CFG_LIB_SIGNAL
#define CFG_LIB_SYSTEM
#include "AMCCommonLib.h"

#include "AMCTimeMark.h"
#include "AMCLibeventTool.h"

#define _SERVER_PORT		1070

#define SRVLog(fmt, args...)	AMCPrintf("[SERVER %03d] "fmt, __LINE__, ##args)


static void _printStatus(struct event_base *base)
{
	SRVLog("event_base status:");
	AMCEventBase_DumpStdout(base);
}


static void _cbRead(struct bufferevent *buffEv, void *arg)
{
	struct evbuffer *pReadData = bufferevent_get_input(buffEv);

	if (pReadData)
	{
		void *pData = NULL;
		size_t dataLen = evbuffer_get_length(pReadData);

		pData = malloc(dataLen);

		if (pData)
		{
			dataLen = evbuffer_remove(pReadData, pData, dataLen);

			SRVLog("Get data from file descriptor %d:", bufferevent_getfd(buffEv));
			printData(pData, dataLen);

			free(pData);
			pData = NULL;
		}
		else
		{
			SRVLog("Failed to malloc memory.");
		}
	}
	else
	{
		SRVLog("Cannot read data from bufferevent.");
	}

	return;
}


static void _cbEvent(struct bufferevent *buffEv, short events, void *arg)
{
	if (IS_ALL_BITS_SET(events, BEV_EVENT_TIMEOUT))
	{
		SRVLog("[fd %d] Timeout on bufferevent.", bufferevent_getfd(buffEv));
		bufferevent_free(buffEv);
		_printStatus((struct event_base *)arg);
	}
	else if (IS_ALL_BITS_SET(events, BEV_EVENT_EOF))
	{
		SRVLog("[fd %d] EOF on bufferevent.", bufferevent_getfd(buffEv));
		bufferevent_free(buffEv);
		_printStatus((struct event_base *)arg);
	}
	else
	{
		SRVLog("[fd %d] Get event 0x%04x", bufferevent_getfd(buffEv), events);
	}

	//_printStatus((struct event_base *)arg);

	return;
}


static void _cbAccept(evutil_socket_t fd, short what, void *arg)
{
	struct event_base *pBase = arg;
	struct sockaddr_in sockAddr;
	struct timeval timeout;
	int socket = 0;
	socklen_t addrLen = sizeof(sockAddr);

	timeout.tv_sec  = 5;
	timeout.tv_usec = 0;

	socket = accept(fd, &sockAddr, &addrLen);

	if (socket < 0)
	{
		SRVLog("accept() failed.");
	}
	else if (fd > FD_SETSIZE)
	{
		SRVLog("fd > FD_SETSIZE.");
		close(fd);
	}
	else
	{
		struct bufferevent *bev;
		evutil_make_socket_nonblocking(socket);

		//_printStatus((struct event_base *)arg);

		bev = bufferevent_socket_new(pBase, socket, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);

		if (bev)
		{
			bufferevent_setcb(bev, _cbRead, NULL, _cbEvent, pBase);
			bufferevent_set_timeouts(bev, &timeout, NULL);
			bufferevent_enable(bev, EV_READ);
			SRVLog("Accept file descriptor %d.", socket);
			//_printStatus((struct event_base *)arg);
		}
		else
		{
			close(socket);
			SRVLog("Cannot create buffer event.");
		}
	}

	return;
}


struct event *_tmpEvent = NULL;

static void _cbSignal(evutil_socket_t fd, short what, void *arg)
{
	static int sigintCount = 5;

	if (SIGPIPE == fd)
	{
		SRVLog("Get SIGPIPE");
	}
	else if (SIGINT == fd)
	{
		if (--sigintCount <= 0)
		{
			sigintCount = 0;
			SRVLog("SIGINT: Exit server base 0x%08lx.", (unsigned long)arg);
			event_base_loopexit((struct event_base *)arg, NULL);
		}
		else
		{
			if (_tmpEvent)
			{
				event_del(_tmpEvent);
				_tmpEvent = NULL;
			}
			SRVLog("SIGINT: Please send SIGINT %d time(s) to exit...", sigintCount);
			AMCEventBase_DumpStdout((struct event_base *)arg);
		}
	}
	else
	{
		SRVLog("Get signal %d", fd);
	}

	return;
}





/**********/
/* main */
#define _MARK_ERR(condition)		if (condition){ret = -1; errLine = __LINE__;}do{}while(0)
int serverMain()
{
	int ret = 0;
	int errLine = 0;
	struct event_base *base = NULL;
	struct event *listenEvent = NULL;

	/******/
	/* allocate event_base */
	base = AMCEventBase_New();
	_MARK_ERR(NULL == base);


	/******/
	/* set signal event */

	if (0 == ret)
	{
		SRVLog("Get event base 0x%08lx", (unsigned long)base);
	
		struct event *pSignalEvent = evsignal_new(base, SIGPIPE, _cbSignal, base);
		if (pSignalEvent)
		{
			evsignal_add(pSignalEvent, NULL);
		}
	}

	if (0 == ret)
	{
		struct event *pSignalEvent = evsignal_new(base, SIGINT, _cbSignal, base);
		if (pSignalEvent)
		{
			evsignal_add(pSignalEvent, NULL);
		}
	}
	
	
	/******/
	/* set listen event */
	if (0 == ret)
	{
		listenEvent = AMCEvent_TcpListenNew(base, _SERVER_PORT, 128, TRUE, _cbAccept, base);
		_MARK_ERR(NULL == listenEvent);
	}

	/******/
	/* add event */
	if (0 == ret)
	{
		_tmpEvent = listenEvent;
		ret = AMCEvent_Add(listenEvent, NULL);
		_MARK_ERR(0 != ret);
	}

	/******/
	/* dispatch event_base */
	if (0 == ret)
	{
		//SRVLog("Initial event base status:");
		//_printStatus(pRcvBase);
		AMCEventBase_Dispatch(base);
	}


	/******/
	/* clean up resources */ 
	if (listenEvent)
	{
		AMCEvent_Del(listenEvent);
		AMCEvent_Free(listenEvent);
		listenEvent = NULL;
	}

	if (base)
	{
		AMCEventBase_Free(base);
		base = NULL;
	}

	if (0 != ret)
	{
		SRVLog("Server failed in line %d.", errLine);
	}
	else
	{
		SRVLog("Server ends successfully.");
	}
	
	return ret;
}
#undef _MARK_ERR



