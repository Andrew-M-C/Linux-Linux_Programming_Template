/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang
	
	File name: AMCThreadPool.h
	Description: 	
			This file provide interfaces to create and use of POSIX thread pool.
			
	Author:		Andrew Chang
	History:
		2016-02-29: File created as "AMCThreadPool.h"

--------------------------------------------------------------
	Copyright information: 
			This file is open source and free to use. 
			I (author) promise nothing about this source file. If you use it in
		commercial application, please make sure you have done detailed test of
		this program to avoid possible bugs.
		    For any suggestions, problems and bug reports, please contact Andrew
		through either E-mails below:
		1. zhangmin@tp-link.com.cn  (as long as I am an employee of TP-LINK)
		2. laplacezhang@126.com
		
********************************************************************************/

#ifndef	__AMC_THREAD_POOL_H__
#define	__AMC_THREAD_POOL_H__

#include "stddef.h"
#include "errno.h"

#ifndef BOOL
#define BOOL int
#define FALSE 0
#define TRUE (!(FALSE))
#endif

struct AMCThreadPool;

typedef void (*AMCThreadPool_Routine)(void *arg);

struct AMCThreadPool *AMCThreadPool_Create(size_t threadCount);
int AMCThreadPool_Destroy(struct AMCThreadPool *pool);		/* this function is async */
int AMCThreadPool_AddTask(struct AMCThreadPool *pool, AMCThreadPool_Routine routine, void *arg);

size_t AMCThreadPool_runningThreadCount(struct AMCThreadPool *pool);
size_t AMCThreadPool_freeThreadCount(struct AMCThreadPool *pool);
size_t AMCThreadPool_totalThreadCount(struct AMCThreadPool *pool);


#endif


