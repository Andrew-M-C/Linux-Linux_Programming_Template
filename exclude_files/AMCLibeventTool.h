/*******************************************************************************
	Copyright (C), 2011-2014, Andrew Min Chang
	
	File name: AMCLibeventTool.h
	Description: 	
			This file provide interfaces to use simple TCP socket with libevent.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2014-12-25: File created as "AMCLibeventTool.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_LIBEVENT_TOOL_H
#define	_AMC_LIBEVENT_TOOL_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>


#ifndef BOOL
#define BOOL	long
#define FALSE	(0)
#define TRUE	(!0)
#endif

#ifndef NULL
#define NULL	((void*)0)
#endif

typedef enum {
	Direction_Input = 0,
	Direction_Output,
	
	Direction_Illegal
} AMCDataDirection_t;



/**/
struct event_base *AMCEventBase_New(void);
int AMCEventBase_Free(struct event_base *base);
int AMCEventBase_Dispatch(struct event_base *base);
int AMCEventBase_Stop(struct event_base *base, BOOL immediately);
int AMCEventBase_DumpStdout(struct event_base *base);


/**/
struct event *AMCEvent_TcpListenNew(struct event_base *base, int bindPort, int listenBacklog, BOOL shouldPersist, event_callback_fn acceptCallback, void *callbackArg);
int AMCEvent_Add(struct event *event, const struct timeval *timeout);
int AMCEvent_Del(struct event *event);
int AMCEvent_Free(struct event *event);
int AMCEvent_DumpString(struct event *event, char *string, size_t lenLimit);
int AMCEvent_DumpStdout(struct event *event);


/**/
struct bufferevent *AMCBufferevent_New(struct event_base *base, BOOL shouldDeferCallbacks, BOOL isThreadSafe);
struct bufferevent *AMCBufferevent_NewAndConfig(struct event_base *base, BOOL shouldDeferCallbacks, BOOL isThreadSafe, bufferevent_data_cb readCallback, const struct timeval *timeoutRead, bufferevent_data_cb writeCallback, const struct timeval *timeoutWrite, bufferevent_event_cb eventCallback, void *callbackArg);
int AMCBufferevent_Connect(struct bufferevent *buffEv, int targetPort, char *targetIPv4);
int AMCBufferevent_Free(struct bufferevent *buffEv);


/**/
typedef enum {
	AMCTmEvBseCfg_DoNotUseLock = 1 << 0,
	AMCTmEvBseCfg_Default = 0
}AMCTimingEventBaseConf_t;

typedef enum {
	AMCTmEvCfg_Repeat = 1 << 0,
	AMCTmEvCfg_Default = 0
}AMCTimingEventConf_t;

struct AMCTimingEvent;
struct AMCTimingEventBase;
typedef void (*timing_callback_fn)(struct AMCTimingEvent *event, void *arg);

struct AMCTimingEventBase *AMCTimingEventBase_New(AMCTimingEventBaseConf_t flag, const struct timeval *timeInterval);
int AMCTimingEventBase_Free(struct AMCTimingEventBase *timeEvBase);

struct AMCTimingEvent *AMCTimingEvent_New(AMCTimingEventConf_t flag, const struct timeval *timeout);
int AMCTimingEvent_Free(struct AMCTimingEvent *timeEvent);
int AMCTimingEvent_ResetTimeout(struct AMCTimingEvent *timeEvent);
int AMCTimingEvent_SetTimeout(struct AMCTimingEvent *timeEvent, const struct timeval *timeout);
int AMCTimingEvent_TriggerTimeout(struct AMCTimingEvent *timeEvent);
int AMCTimingEvent_Add(struct AMCTimingEvent *timeEvent, struct AMCTimingEventBase *timeEvBase);
int AMCTimingEvent_Remove(struct AMCTimingEvent *timeEvent);
	

#endif
