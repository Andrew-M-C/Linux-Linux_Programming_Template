/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCString.c
	Description: 	
			This file provides a safe string interfaces
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-10-10: File created as "AMCString.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCString.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifndef	NULL
#define	NULL	((void *)0)
#endif


AMCString_st *AMCStringCreate(size_t stringLen, const char *str)
{
	AMCString_st *string = NULL;
	char *dataBuff = NULL;

	if (((size_t)stringLen) >= MAX_DATA_BUFF_LEN)
	{
		errno = EINVAL;
		return NULL;
	}

	string = malloc(sizeof(*string));
	if (NULL == string)
	{
		return NULL;
	}

	dataBuff = malloc(stringLen+1);
	if (NULL == dataBuff)
	{
		free(string);
		return NULL;
	}

	dataBuff[stringLen] = '\0';
	string->textBuff = dataBuff;
	string->buffSize = (uint32_t)stringLen;

	if (NULL != str)
	{
		strncpy(dataBuff, str, stringLen - 1);
	}
	
	return string;
}


void AMCStringFree(AMCString_st *string)
{
	free(string->textBuff);
	free(string);
	return;
}




retval_t AMCStringCpy(AMCString_st *dest, const char *str)
{
	if ((NULL == dest) || (NULL == str))
	{
		errno = EINVAL;
		return -1;
	}

	strncpy(dest->textBuff, str, dest->buffSize);
	return 0;
}

retval_t AMCStringCopy(AMCString_st *dest, AMCString_st *src)
{
	uint32_t copyLimit;

	if ((NULL == dest) || (NULL == src))
	{
		errno = EINVAL;
		return -1;
	}

	if ((dest->buffSize) > (src->buffSize))
	{
		copyLimit = src->buffSize;
	}
	else
	{
		copyLimit = dest->buffSize;
	}

	strncpy(dest->textBuff, src->textBuff, copyLimit);
	return 0;
}





sint8_t AMCStringCmp(const AMCString_st *string, const char *str)
{
	int cmpResult;

	if ((NULL == string) || (NULL == str))
	{
		errno = EINVAL;
		return -1;
	}

	if (string->buffSize >= strlen(str))
	{
		return strncmp(string->textBuff, str, string->buffSize);
	}
	else
	{
		cmpResult = strncmp(string->textBuff, str, string->buffSize);
		if (0 != cmpResult)
		{
			return cmpResult;
		}
		else
		{
			return -1;
		}
	}
}

sint8_t AMCStringCompare(const AMCString_st *string1, const AMCString_st *string2)
{
	uint32_t cmpLimit;
	int cmpResult;

	if (NULL == string1)
	{
		errno = EINVAL;
		return -1;
	}
	else if (NULL == string2)
	{
		errno = EINVAL;
		return 1;
	}

	if ((string1->buffSize) > (string2->buffSize))
	{
		cmpLimit = string2->buffSize;
		cmpResult = strncmp(string1->textBuff, string2->textBuff, cmpLimit);
		if (0 != cmpResult)
		{
			return cmpResult;
		}
		else
		{
			if (strlen(string1->textBuff) <= cmpLimit)
			{
				return 0;
			}
			else
			{
				return 1;
			}
		}
	}
	else if ((string1->buffSize) < (string2->buffSize))
	{
		cmpLimit = string1->buffSize;
		cmpResult = strncmp(string1->textBuff, string2->textBuff, cmpLimit);
		if (0 != cmpResult)
		{
			return cmpResult;
		}
		else
		{
			if (strlen(string2->textBuff) <= cmpLimit)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		cmpLimit = string1->buffSize;
		return strncmp(string1->textBuff, string2->textBuff, cmpLimit);
	}
}





retval_t AMCStringCat(AMCString_st *dest, const char *str)
{
	utmp32_t catLimit;

	if ((NULL == dest) || (NULL == str))
	{
		errno = EINVAL;
		return -1;
	}

	catLimit = (dest->buffSize) - (strlen(dest->textBuff));
	strncat(dest->textBuff, str, catLimit);

	return 0;
}

retval_t AMCStringConcatenate(AMCString_st *dest, AMCString_st *src)
{
	utmp32_t catLimit;

	if ((NULL == dest) || (NULL == src))
	{
		errno = EINVAL;
		return -1;
	}

	catLimit = (dest->buffSize) - (strlen(dest->textBuff));
	strncat(dest->textBuff, src->textBuff, catLimit);
	return 0;
}






size_t AMCStringLength(const AMCString_st *string)
{
	if (NULL == string)
	{
		return 0;
	}

	return strlen(string->textBuff);
}






char AMCStringCharRead(const AMCString_st *string, uint32_t index)
{
	if (NULL == string)
	{
		return '\0';
	}

	if (index > (string->buffSize))
	{
		return '\0';
	}
	else
	{
		return (string->textBuff)[index];
	}
}

void AMCStringCharWrite(AMCString_st *string, uint32_t index, char character)
{
	if (NULL == string)
	{
		return;
	}

	if (index > (string->buffSize))
	{
		return;
	}
	else
	{
		(string->textBuff)[index] = character;
		return;
	}
}





char *AMCStringString(const AMCString_st *string)
{
	if (NULL == string)
	{
		return "";
	}
	else if (NULL == string->textBuff)
	{
		return "";
	}
	else
	{
		return string->textBuff;
	}
}

uint32_t AMCStringLimit(const AMCString_st *string)
{
	if (NULL == string)
	{
		return 0;
	}
	else if (NULL == string->textBuff)
	{
		return 0;
	}
	else
	{
		return string->buffSize;
	}
}


