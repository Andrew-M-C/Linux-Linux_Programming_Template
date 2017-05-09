/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCDictionary.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provides mutable associative array support.
			
	History:
		2017-02-06: File created as AMCDictionary.h

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/


#ifndef	__AMC_DICTIONARY_H__
#define __AMC_DICTIONARY_H__

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#define CFG_AMC_DICT_DEFAULT_INITIAL_ITEM_COUNT	128
#define CFG_AMC_DICT_DEFAULT_GROW_ITEM_COUNT	32

#define CFG_AMC_DICT_DEFAULT_INITIAL_DICT_COUNT	16
#define CFG_AMC_DICT_DEFAULT_GROW_DICT_COUNT	8

#define CFG_AMC_DICT_KEY_LEN_MAX	63


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

struct AMCDict;
typedef struct AMCDict AMCDictionary_st;

enum {
	AMC_DICT_NO_ERROR		= 0,
	AMC_DICT_SYSTEM_ERROR,		// will set "sys_errno"
	AMC_DICT_AMC_DICT_INITIALIZED,
	AMC_DICT_INVALID_PARAMETER,
	AMC_DICT_MEMPOOL_ERROR,
	AMC_DICT_OBJECT_EXISTS,
	AMC_DICT_OBJECT_NOT_EXIST,
};

typedef struct {
	int16_t dict_error;
	int16_t sys_errno;
} AMCDictErrno_st;

AMCDictErrno_st AMCDictionary_GlobalConfig(ssize_t initDictCount, ssize_t growDictCount, ssize_t initItemCount, ssize_t growItemCount);

AMCDictionary_st *AMCDictionary_New(AMCDictErrno_st *pErrOut);
AMCDictErrno_st AMCDictionary_Free(AMCDictionary_st *dict, BOOL freeObjects);

size_t AMCDictionary_Count(const AMCDictionary_st *dict);
void *AMCDictionary_GetObject(AMCDictionary_st *dict, const char *key);
AMCDictErrno_st AMCDictionary_SetObject(AMCDictionary_st *dict, void *object, const char *key, BOOL freeOldObject, BOOL *pOverwritten);
AMCDictErrno_st AMCDictionary_AddObject(AMCDictionary_st *dict, void *object, const char *key);
AMCDictErrno_st AMCDictionary_UpdateObject(AMCDictionary_st *dict, void *object, const char *key, BOOL freeOldObject);

AMCDictErrno_st AMCDictionary_RemoveObject(AMCDictionary_st *dict, const char *key, BOOL freeObject);
void *AMCDictionary_DrainObject(AMCDictionary_st *dict, const char *key, AMCDictErrno_st *pErrOut);

void AMCDictionary_DumpStatus(AMCDictionary_st *dict, FILE *file);


#endif	/* end of file */

