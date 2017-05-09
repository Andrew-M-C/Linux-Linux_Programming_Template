/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCTimingTool.h
	Description: 	
			This file provides simple timing tools.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-08: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_TIMING_TOOL_H
#define _AMC_TIMING_TOOL_H

enum {
	TmToolErrno_Success = 0,
	TmToolErrno_ParamError,
	TmToolErrno_SystemCallError,
	TmToolErrno_MemPoolError,
	TmToolError_TmToolRunning,
	TmToolError_TmToolStopped,
	TmToolError_TmObjInvalid,
	TmToolError_InRountine,
	TmToolWarn_TimeoutTooSoon,
	
	TmToolErrno_UnknownError
};


#ifndef BOOL
#define BOOL	int
#endif

#ifndef FALSE
#define FALSE	(0)
#define TRUE	(!(FALSE))
#endif

#ifndef NULL
#define NULL	((void*)0)
#endif

#define AMC_TM_1_SECOND		1000000
#define AMC_TM_1_MINUTE		(60*AMC_TM_1_SECOND)
#define AMC_TM_1_MILISEC	1000

typedef enum {
	TmCfg_Repeated = 1 << 0,
	TmCfg_Default = 0
} AMCTiming_Config_t;


/****************/
/* general timing tool */

struct AMCTimingTool;
struct AMCTimingObject;
typedef void (*AMCTiming_Observation_Callback)(struct AMCTimingObject *obj, void *arg);

struct AMCTimingTool *AMCTiming_New(long intervalUsec, long expectedObjectCount, int *errnoOut);
int AMCTiming_Destroy(struct AMCTimingTool *tmTool);

int AMCTiming_Start(struct AMCTimingTool *tmTool);
int AMCTiming_Stop(struct AMCTimingTool *tmTool);
BOOL AMCTiming_IsRunning(const struct AMCTimingTool *tmTool, int *errnoOut);

struct AMCTimingObject *AMCTiming_AddObject(struct AMCTimingTool *tmTool, AMCTiming_Observation_Callback observation, long timeoutUsec, AMCTiming_Config_t flags, void *arg, int *errnoOut);
int AMCTiming_DelObject(struct AMCTimingObject *tmObj);

int AMCTiming_ResetTimeout(struct AMCTimingObject *tmObj, long timeoutUsec);
int AMCTiming_TriggerObject(struct AMCTimingObject *tmObj);

int AMCTiming_DebugStdout(const struct AMCTimingTool *tmTool);


/****************/
/* one-second tool */

struct AMCTimingSecTool;
struct AMCTimingSecObject;
typedef void (*AMCTimingSec_Observation_Callback)(struct AMCTimingSecObject *obj, void *arg);

struct AMCTimingSecTool *AMCTimingSecond_New(long expectedObjectCount, int *errnoOut);
int AMCTimingSecond_Destroy(struct AMCTimingSecTool *tmScTool);

int AMCTimingSecond_Start(struct AMCTimingSecTool *tmScTool);
int AMCTimingSecond_Stop(struct AMCTimingSecTool *tmScTool);
BOOL AMCTimingSecond_IsRunning(const struct AMCTimingSecTool *tmScTool, int *errorOut);

struct AMCTimingSecObject *AMCTimingSecond_AddObject(struct AMCTimingSecTool *tmScTool, AMCTimingSec_Observation_Callback observation, long timeoutSec, AMCTiming_Config_t flags, void *arg, int *errnoOut);
int AMCTimingSecond_DelObject(struct AMCTimingSecObject *tmScObj);

int AMCTimingSecond_ResetTimeout(struct AMCTimingSecObject *tmScObj, long timeoutSec);
int AMCTimingSecond_TriggerObject(struct AMCTimingSecObject *tmScObj);

int AMCTimingSecond_DebugStdout(const struct AMCTimingSecTool *tmTool);

#endif

