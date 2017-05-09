/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCDictionary.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file implements mutable associative array support.
			
	History:
		2017-02-04: File created as AMCDictionary.c

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/********/
#define __HEADERS
#ifdef __HEADERS

#include "AMCDictionary.h"
#include "AMCMemPool.h"

#define DEBUG
#define CFG_LIB_MEM
#define CFG_LIB_THREAD
#define CFG_LIB_MATH
#define CFG_LIB_FILE
#include "AMCCommonLib.h"

#endif


/********/
#define __DATA_TYPES_AND_VARIABLES
#ifdef __DATA_TYPES_AND_VARIABLES


struct AMCDictItem {

};


struct AMCDict {
	pthread_mutex_t       lock;
	size_t                count;
};


#define _DB(fmt, args...)		AMCPrintf("<%s, %ld> "fmt, __FILE__, __LINE__, ##args)
#define _LOCK_DICT(dict)			do{/*_DB("-- LOCK --");*/ pthread_mutex_lock(&((dict)->lock));}while(0)
#define _UNLOCK_DICT(dict)		do{/*_DB("--UNLOCK--");*/ pthread_mutex_unlock(&((dict)->lock));}while(0)

static struct AMCMemPool *g_item_mem_pool = NULL;
static size_t g_item_init_size = CFG_AMC_DICT_DEFAULT_INITIAL_ITEM_COUNT;
static size_t g_item_inc_size = CFG_AMC_DICT_DEFAULT_GROW_ITEM_COUNT;

static struct AMCMemPool *g_dict_mem_pool = NULL;
static size_t g_dict_init_size = CFG_AMC_DICT_DEFAULT_INITIAL_DICT_COUNT;
static size_t g_dict_inc_size = CFG_AMC_DICT_DEFAULT_GROW_DICT_COUNT;

#endif


/********/
#define __INIT_AND_DEALLOC_FUNCTIONS
#ifdef __INIT_AND_DEALLOC_FUNCTIONS

/* --------------------_make_error----------------------- */
AMCArrayErrno_st _make_error(int16_t error)
{
	AMCArrayErrno_st ret = {0, 0};

	if (AMC_ARRAY_SYSTEM_ERROR == error) {
		ret.sys_errno = (int16_t)errno;
	}

	ret.array_error = error;

	return ret;
}




#endif


/********/
#define __ARRAY_MODIFICATIONS
#ifdef __ARRAY_MODIFICATIONS




#endif


/********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES




#endif

/********/
/* end of file */

