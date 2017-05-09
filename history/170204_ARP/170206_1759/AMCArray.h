/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCArray.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provides mutable array support.
			
	History:
		2017-02-04: File created as AMCArray.h

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/


#ifndef	__AMC_ARRAY_H__
#define __AMC_ARRAY_H__

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#define CFG_AMC_ARRAY_DEFAULT_INITIAL_ITEM_COUNT	128
#define CFG_AMC_ARRAY_DEFAULT_INCREASE_ITEM_COUNT	32

#define CFG_AMC_ARRAY_DEFAULT_INITIAL_ARRAY_COUNT	16
#define CFG_AMC_ARRAY_DEFAULT_INCREASE_ARRAY_COUNT	8


#ifndef BOOL
#ifndef DO_NOT_DEF_BOOL
#define BOOL	int
#define FALSE	(0)
#define TRUE	(!FALSE)
#endif
#endif

#ifndef NULL
#ifndef DO_NOT_DEF_NULL
#define NULL	((void*)0)
#endif
#endif

struct AMCArray;
typedef struct AMCArray AMCArray_st;

enum {
	AMC_ARRAY_NO_ERROR		= 0,
	AMC_ARRAY_SYSTEM_ERROR,		// will set "sys_errno"
	AMC_ARRAY_AMC_ARRAY_INITIALIZED,
	AMC_ARRAY_INVALID_PARAMETER,
	AMC_ARRAY_MEMPOOL_ERROR,
	AMC_ARRAY_EXCEED_ARRAY_SIZE,
};

typedef struct {
	int16_t array_error;
	int16_t sys_errno;
} AMCArrayErrno_st;

AMCArrayErrno_st AMCArray_GlobalConfig(ssize_t initArrayCount, ssize_t incrementArrayCount, ssize_t initItemCount, ssize_t incrementItemCount);

AMCArray_st *AMCArray_New(AMCArrayErrno_st *pErrOut);
AMCArray_st *AMCArray_NewWithObjects(void **pObjects, size_t count, AMCArrayErrno_st *pErrOut);
AMCArrayErrno_st AMCArray_Free(AMCArray_st *array, BOOL freeObjects);

size_t AMCArray_Count(const AMCArray_st *array);
void *AMCArray_ObjectAtIndex(AMCArray_st *array, size_t index);

AMCArrayErrno_st AMCArray_AddObject(AMCArray_st *array, void *object);
AMCArrayErrno_st AMCArray_InsertObject(AMCArray_st *array, void *object, size_t index);
AMCArrayErrno_st AMCArray_RemoveObject(AMCArray_st *array, size_t index, BOOL freeObject);

void AMCArray_DumpStatus(AMCArray_st *array, FILE *file);


#endif	/* end of file */

