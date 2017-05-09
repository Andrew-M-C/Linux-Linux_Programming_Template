/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCLibevTools.h
	Description: 	
			This file provide simple interface for libev.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-05-08: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_LIBEV_TOOLS_H
#define _AMC_LIBEV_TOOLS_H

#include "AMCLibevTypes.h"

struct AMC_ev_io       *AMCEvIo_new(size_t argLen);
struct AMC_ev_timer    *AMCEvTimer_new(size_t argLen);
struct AMC_ev_periodic *AMCEvPeriodic_new(size_t argLen);
struct AMC_ev_signal   *AMCEvSignal_new(size_t argLen);
struct AMC_ev_child    *AMCEvChild_new(size_t argLen);
struct AMC_ev_stat     *AMCEvStat_new(size_t argLen);
struct AMC_ev_idle     *AMCEvIdle_new(size_t argLen);
struct AMC_ev_prepare  *AMCEvPrepare_new(size_t argLen);
struct AMC_ev_check    *AMCEvCheck_new(size_t argLen);
struct AMC_ev_fork     *AMCEvFork_new(size_t argLen);
struct AMC_ev_cleanup  *AMCEvCleanup_new(size_t argLen);
struct AMC_ev_embed    *AMCEvEmbed_new(size_t argLen);
struct AMC_ev_async    *AMCEvAsync_new(size_t argLen);

void AMCEvWatcher_free(void *watcher);

struct AMC_ev_io *AMCEvIo_TcpListenNew(int port, int listenBacklog, size_t argLen);


#endif	/* End of File */
