/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCString.h
	Description: 	
			This file provides a safe string interfaces
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-10-10: File created as "AMCString.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/
#ifndef	_AMC_STRING
#define	_AMC_STRING


#include "AMCDataTypes.h"
#include <stdio.h>

#define	MAX_DATA_BUFF_LEN	(65534)		/* Please do not change this value */


typedef struct _amc_string_s{
	char 	*textBuff;
	uint32_t	buffSize;
}AMCString_st;


AMCString_st *AMCStringCreate(size_t stringLen, const char *str);
void AMCStringFree(AMCString_st *string);

retval_t AMCStringCpy(AMCString_st *dest, const char *str);
retval_t AMCStringCopy(AMCString_st *dest, AMCString_st *src);

sint8_t AMCStringCmp(const AMCString_st *string, const char *str);
sint8_t AMCStringCompare(const AMCString_st *string1, const AMCString_st *string2);

retval_t AMCStringCat(AMCString_st *dest, const char *str);
retval_t AMCStringConcatenate(AMCString_st *dest, AMCString_st *src);

size_t AMCStringLength(const AMCString_st *string);

char AMCStringCharRead(const AMCString_st *string, uint32_t index);
void AMCStringCharWrite(AMCString_st *string, uint32_t index, char character);

char *AMCStringString(const AMCString_st *string);
uint32_t AMCStringLimit(const AMCString_st *string);


#endif
