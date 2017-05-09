/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCLibevTools.h
	Description: 	
			This file provide simple interface for libev.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-05-08: File created
		2016-05-03: Move all content from AMCLibevTypes.h here

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_LIBEV_TOOLS_H
#define _AMC_LIBEV_TOOLS_H

#include "ev.h"
#include <stdlib.h>
#include <errno.h>

#ifndef NULL
#define NULL ((void*)0)
#endif

#define PTR_LEN		sizeof(void*)

typedef enum {
	Libev_Success = 0,
	Libev_ConnectionBroken, 
	Libev_NoMoreData,

	Libev_OtherError
} AMCEvStatus_t;

struct AMC_ev_io {
	struct ev_io watcher;
	void *arg;
};

struct AMC_ev_timer {
	struct ev_timer watcher;
	void *arg;
};

struct AMC_ev_periodic {
	struct ev_periodic watcher;
	void *arg;
};

struct AMC_ev_signal {
	struct ev_signal watcher;
	void *arg;
};

struct AMC_ev_child {
	struct ev_child watcher;
	void *arg;
};

struct AMC_ev_stat {
	struct ev_stat watcher;
	void *arg;
};

struct AMC_ev_idle {
	struct ev_idle watcher;
	void *arg;
};

struct AMC_ev_prepare {
	struct ev_prepare watcher;
	void *arg;
};

struct AMC_ev_check {
	struct ev_check watcher;
	void *arg;
};

struct AMC_ev_fork {
	struct ev_fork watcher;
	void *arg;
};

struct AMC_ev_cleanup {
	struct ev_cleanup watcher;
	void *arg;
};

struct AMC_ev_embed {
	struct ev_embed watcher;
	void *arg;
};

struct AMC_ev_async {
	struct ev_async watcher;
	void *arg;
};

struct AMC_ev_io       *AMCEvIo_new(void);
struct AMC_ev_timer    *AMCEvTimer_new(void);
struct AMC_ev_periodic *AMCEvPeriodic_new(void);
struct AMC_ev_signal   *AMCEvSignal_new(void);
struct AMC_ev_child    *AMCEvChild_new(void);
struct AMC_ev_stat     *AMCEvStat_new(void);
struct AMC_ev_idle     *AMCEvIdle_new(void);
struct AMC_ev_prepare  *AMCEvPrepare_new(void);
struct AMC_ev_check    *AMCEvCheck_new(void);
struct AMC_ev_fork     *AMCEvFork_new(void);
struct AMC_ev_cleanup  *AMCEvCleanup_new(void);
struct AMC_ev_embed    *AMCEvEmbed_new(void);
struct AMC_ev_async    *AMCEvAsync_new(void);

void AMCEvWatcher_free(void *watcher);

struct AMC_ev_io *AMCEvIo_TcpListenNew(int port, int listenBacklog, void *arg);

AMCEvStatus_t AMCEvReadStatusResolve(int readReturn, int errNo, char **pErrStrOut);
AMCEvStatus_t AMCEvWriteStatusResolve(int readReturn, int errNo, char **pErrStrOut);


#endif	/* End of File */