/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCCpuUsage.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file implements CPU usage monitor tool.
			
	History:
		2017-02-14: File created as AMCCpuUsage.c

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/********/
#define __HEADERS
#ifdef __HEADERS

#include "AMCCpuUsage.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#endif


/********/
#define __MACROS_AND_DEFINITIONS
#ifdef __MACROS_AND_DEFINITIONS

struct AMCCpuUsage
{
	long     proc_pid;

	uint64_t last_sys_cpu_time;
	uint64_t this_sys_cpu_time;
	
	uint64_t last_proc_cpu_time;
	uint64_t this_proc_cpu_time;

	uint64_t million_usage;
};


#define _NO_ERROR		{0, 0}
#define _PRINT_ERR(fmt, args...)		fprintf(stderr, "[CPU usage] "fmt"\n", ##args)

//#define _DEBUG_FLAG
#ifdef _DEBUG_FLAG
extern ssize_t AMCPrintf(const char *format, ...);
#define _DEBUG(fmt, args...)		AMCPrintf("#-DBG-# %s(), %d: "fmt, __FUNCTION__, __LINE__, ##args)
#else
#define _DEBUG(fmt, args...)
#endif


#endif



/********/
#define __INTERNAL_FUNCTIONS
#ifdef __INTERNAL_FUNCTIONS

/* --------------------_make_error----------------------- */
static AMCCpuUsageErrno_st _make_error(int16_t error)
{
	AMCCpuUsageErrno_st ret = {0, 0};

	if (AMC_CPU_USAGE_SYSTEM_ERROR == error) {
		ret.sys_errno = (int16_t)errno;
	}

	ret.obj_error = error;

	return ret;
}


/* --------------------_find_next_num_str----------------------- */
static char *_locate_next_num_str(char *str)
{
	char *ret = str;

	if (NULL == str) {
		return NULL;
	}

	if ('\0' == *str) {
		return NULL;
	}

	/* is current str a number? if so, skip it */
	while (*ret && isdigit(*ret))
	{
		ret ++;
	}

	/* search for next number */
	while (*ret && (0 == isdigit(*ret)))
	{
		ret ++;
	}

	return (*ret) ? ret : NULL;
}


/* --------------------_str_to_uint64----------------------- */
static uint64_t _str_to_uint64(const char *str)
{
	uint64_t ret = 0;
	const char *chr = str;

	if (NULL == str) {
		return 0;
	}

	while(isdigit(*chr))
	{
		ret = ret * 10 + (*chr - '0');
		chr ++;
	}

	return ret;
}


/* --------------------_update_CPU_usage----------------------- */
static AMCCpuUsageErrno_st _update_CPU_usage(AMCCpuUsage_st *obj)
{
	AMCCpuUsageErrno_st error = _NO_ERROR;
	
	char *cpuReadStr = NULL;
	size_t cpuReadLen = 0;

	char *procReadStr = NULL;
	size_t procReadLen = 0;

	char filePathBuff[128];

	FILE *cpuFile;
	FILE *procFile;

	/* get ready */
	obj->last_sys_cpu_time  = obj->this_sys_cpu_time;
	obj->last_proc_cpu_time = obj->this_proc_cpu_time;

	if (obj->proc_pid <= 0) {
		error = _make_error(AMC_CPU_USAGE_PROCESS_NOT_EXIST);
		goto ENDS;
	}

	/* read system CPU time */
	cpuFile = fopen("/proc/stat", "r");
	if (NULL == cpuFile) {
		_PRINT_ERR("Failed to read CPU status, your OS may not capatible.");
		error = _make_error(AMC_CPU_USAGE_OS_ERROR);
		goto ENDS;
	}

	getline(&cpuReadStr, &cpuReadLen, cpuFile);
	if (NULL == cpuReadStr) {
		error = _make_error(AMC_CPU_USAGE_SYSTEM_ERROR);
		_PRINT_ERR("Failed to read CPU status: %s", strerror(errno));
		goto ENDS;
	}
	else {
		/* parse string */
		char *chr = cpuReadStr;
		long tmp;

		/* The CPU file is useless */
		fclose(cpuFile);
		cpuFile = NULL;

		if (chr && isdigit(*chr)) {
			/* OK */
		}
		else {
			chr = _locate_next_num_str(chr);
		}

		obj->this_sys_cpu_time = 0;

		/* the 1st to 7th numbers should be added */
		for (tmp = 0; (tmp < 7) && chr; tmp++)
		{
			obj->this_sys_cpu_time += _str_to_uint64(chr);
			chr = _locate_next_num_str(chr);
		}
	}

	/* read process CPU time */
	snprintf(filePathBuff, sizeof(filePathBuff), "/proc/%ld/stat", obj->proc_pid);
	procFile = fopen(filePathBuff, "r");
	if (NULL == procFile) {
		_PRINT_ERR("Failed to read process %ld status.", obj->proc_pid);
		error = _make_error(AMC_CPU_USAGE_PROCESS_NOT_EXIST);
		goto ENDS;
	}

	getline(&procReadStr, &procReadLen, procFile);
	if (NULL == procReadStr) {
		error = _make_error(AMC_CPU_USAGE_SYSTEM_ERROR);
		_PRINT_ERR("Failed to read process %ld status: %s", obj->proc_pid, strerror(errno));
		goto ENDS;
	}
	else {
		/* parse string */
		char *chr = procReadStr;
		long tmp;

		/* The process file is useless */
		fclose(procFile);
		procFile = NULL;

		if (chr && isdigit(*chr)) {
			/* OK */
		}
		else {
			chr = _locate_next_num_str(chr);
		}

		obj->this_proc_cpu_time = 0;

		/* the 12th to 15th numbers should be added */
		for (tmp = 0; (tmp < 15) && chr; tmp++)
		{
			switch(tmp)
			{
			case 11:
			case 12:
			case 13:
			case 14:
				obj->this_proc_cpu_time += _str_to_uint64(chr);
				break;
			default:
				break;
			}

			chr = _locate_next_num_str(chr);
		}
	}

	_DEBUG("last_proc_cpu_time = %lld", obj->last_proc_cpu_time);
	_DEBUG("this_proc_cpu_time = %lld", obj->this_proc_cpu_time);
	_DEBUG("last_sys_cpu_time = %lld", obj->last_sys_cpu_time);
	_DEBUG("this_sys_cpu_time = %lld", obj->this_sys_cpu_time);

ENDS:
	/* ends */
	if (cpuFile) {
		fclose(cpuFile);
		cpuFile = NULL;
	}
	if (procFile) {
		fclose(procFile);
		procFile = NULL;
	}
	if (cpuReadStr) {
		free(cpuReadStr);
		cpuReadStr = NULL;
	}
	if (procReadStr) {
		free(procReadStr);
		procReadStr = NULL;
	}

	/* calculate result */
	if (0 != error.obj_error) {
		obj->this_sys_cpu_time  = obj->last_sys_cpu_time;
		obj->this_proc_cpu_time = obj->last_proc_cpu_time;
	}
	else {
		uint64_t sysCpuDiff = obj->this_sys_cpu_time - obj->last_sys_cpu_time;
		uint64_t procCpuDiff = obj->this_proc_cpu_time - obj->last_proc_cpu_time;

		if (procCpuDiff <= 0) {
			obj->million_usage = 0;
		}
		else if (0 == sysCpuDiff) {
			obj->million_usage = 0;
		}
		else {
			obj->million_usage = procCpuDiff * 1000000 / sysCpuDiff;
		}
	}
	return error;
}


#endif


/********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

/* --------------------AMCCpuUsage_New----------------------- */
AMCCpuUsage_st *AMCCpuUsage_New(pid_t pid, AMCCpuUsageErrno_st *pErrOut)
{
	AMCCpuUsageErrno_st error = _NO_ERROR;
	AMCCpuUsage_st *retObj = malloc(sizeof(*retObj));
	if (retObj) {
		memset(retObj, 0, sizeof(*retObj));
		retObj->proc_pid = pid;
	}

	if (pErrOut) {
		memcpy(pErrOut, &error, sizeof(error));
	}
	return retObj;
}


/* --------------------AMCCpuUsage_Free----------------------- */
AMCCpuUsageErrno_st AMCCpuUsage_Free(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		free(pObj);
		pObj = NULL;
		return _make_error(0);
	}
	else {
		return _make_error(AMC_CPU_USAGE_PARAMETER_ERROR);
	}
}


/* --------------------AMCCpuUsage_GetPid----------------------- */
pid_t AMCCpuUsage_GetPid(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		return (pid_t)(pObj->proc_pid);
	}
	else {
		return -1;
	}
}


/* --------------------AMCCpuUsage_SetPid----------------------- */
void AMCCpuUsage_SetPid(AMCCpuUsage_st *pObj, pid_t pid)
{
	if (pObj) {
		pObj->proc_pid = (long)pid;
	}
}


/* --------------------AMCCpuUsage_Update----------------------- */
AMCCpuUsageErrno_st AMCCpuUsage_Update(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		return _update_CPU_usage(pObj);
	}
	else {
		return _make_error(AMC_CPU_USAGE_PARAMETER_ERROR);
	}
}


/* --------------------AMCCpuUsage_GetCpuTime----------------------- */
uint64_t AMCCpuUsage_GetCpuTime(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		return pObj->this_proc_cpu_time;
	}
	else {
		return 0;
	}
}


/* --------------------AMCCpuUsage_GetCpuUsageDouble----------------------- */
double AMCCpuUsage_GetCpuUsageDouble(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		return ((double)(pObj->million_usage)) / 1000000.0;
	}
	else {
		return 0.0;
	}
}


/* --------------------AMCCpuUsage_GetCpuUsagePercent----------------------- */
long AMCCpuUsage_GetCpuUsagePercent(AMCCpuUsage_st *pObj)
{
	if (pObj) {
		return (pObj->million_usage / 10000);
	}
	else {
		return 0;
	}
}




#endif


/********/
/* end of file */

