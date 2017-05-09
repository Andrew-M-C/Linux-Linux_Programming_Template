/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCLibevTypes.h
	Description: 	
			This file provide data structures for AMC libev tools.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-05-08: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_LIBEV_TYPES_H
#define _AMC_LIBEV_TYPES_H

#include <stdlib.h>
#include "ev.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

struct AMC_ev_io {
	struct ev_io watcher;
	void         *arg;
	size_t       argLen;
};

struct AMC_ev_timer {
	struct ev_timer watcher;
	void            *arg;
	size_t          argLen;
};

struct AMC_ev_periodic {
	struct ev_periodic watcher;
	void               *arg;
	size_t             argLen;
};

struct AMC_ev_signal {
	struct ev_signal watcher;
	void             *arg;
	size_t           argLen;
};

struct AMC_ev_child {
	struct ev_child watcher;
	void            *arg;
	size_t          argLen;
};

struct AMC_ev_stat {
	struct ev_stat watcher;
	void           *arg;
	size_t         argLen;
};

struct AMC_ev_idle {
	struct ev_idle watcher;
	void           *arg;
	size_t         argLen;
};

struct AMC_ev_prepare {
	struct ev_prepare watcher;
	void              *arg;
	size_t            argLen;
};

struct AMC_ev_check {
	struct ev_check watcher;
	void            *arg;
	size_t          argLen;
};

struct AMC_ev_fork {
	struct ev_fork watcher;
	void           *arg;
	size_t         argLen;
};

struct AMC_ev_cleanup {
	struct ev_cleanup watcher;
	void              *arg;
	size_t            argLen;
};

struct AMC_ev_embed {
	struct ev_embed watcher;
	void            *arg;
	size_t          argLen;
};

struct AMC_ev_async {
	struct ev_async watcher;
	void            *arg;
	size_t          argLen;
};




#endif		/* End of file */
