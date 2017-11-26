/******************************************************************
 * Copyright (c) 2017, Andrew Chang, All rights reserved.
 * File Name: AMCCoroutine.h
 * Create Time: 2017-11-23
 * License: BSD 2-clause "Simplified" License
 *
 * History:
 *     2017-11-23: File created as AMCCoroutine.h
 */

#ifndef __AMC_COROUTINE_H__
#define __AMC_COROUTINE_H__

#if defined (__cplusplus)
extern "C" {
#endif

#include <unistd.h>

struct _CoroutineInfo;
typedef struct _CoroutineInfo CoroutineInfo_st;
typedef void *(*CoroutineFunc_t)(void *);

CoroutineInfo_st *
    AMCCoroutineAdd(CoroutineFunc_t coroutineFunc, void *arg, size_t stackSize, const char *identifier);
int 
    AMCCoroutineRun(void);
int 
    AMCCoroutineSchedule(void);
void *
    AMCGetStackAddr(void);


#if defined (__cplusplus)
}
#endif

#endif  // end of __AMC_COROUTINE_H__

