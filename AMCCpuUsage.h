/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCCpuUsage.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provides CPU usage monitor tool.
			
	History:
		2017-02-14: File created as AMCCpuUsage.h

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/


#ifndef	__AMC_CPU_USAGE_H__
#define __AMC_CPU_USAGE_H__

#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>

struct AMCCpuUsage;
typedef struct AMCCpuUsage AMCCpuUsage_st;

enum {
	AMC_CPU_USAGE_SUCCESS = 0,
	AMC_CPU_USAGE_SYSTEM_ERROR,
	AMC_CPU_USAGE_OS_ERROR,
	AMC_CPU_USAGE_PARAMETER_ERROR,
	AMC_CPU_USAGE_PROCESS_NOT_EXIST,
};


typedef struct {
	int16_t obj_error;
	int16_t sys_errno;
} AMCCpuUsageErrno_st;


AMCCpuUsage_st *AMCCpuUsage_New(pid_t pid, AMCCpuUsageErrno_st *pErrOut);
AMCCpuUsageErrno_st AMCCpuUsage_Free(AMCCpuUsage_st *pObj);

pid_t AMCCpuUsage_GetPid(AMCCpuUsage_st *pObj);
void AMCCpuUsage_SetPid(AMCCpuUsage_st *pObj, pid_t pid);

AMCCpuUsageErrno_st AMCCpuUsage_Update(AMCCpuUsage_st *pObj);

uint64_t AMCCpuUsage_GetCpuTime(AMCCpuUsage_st *pObj);
double AMCCpuUsage_GetCpuUsageDouble(AMCCpuUsage_st *pObj);
long AMCCpuUsage_GetCpuUsagePercent(AMCCpuUsage_st *pObj);


#endif	/* end of file */

