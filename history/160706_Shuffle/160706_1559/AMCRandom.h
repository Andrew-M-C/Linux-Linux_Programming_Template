/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCRandom.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide some covenient tool in calling library tools. One can easily include 
		library headers he wants by declaring the corresponding macros. 
			I hope this file is not only a header, but also a useful Linux library note.
			
	History:
		2016-07-06: File created

--------------------------------------------------------------
	License information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef __AMC_RANDOM_H__
#define __AMC_RANDON_H__

#include "stdint.h"

long AMCRandomLong(long min, long max);
int AMCRandomInt(int min, int max);
long long AMCRandomLongLong(long min, long max);
signed char AMCRandomChar(signed char min, signed char max);

int64_t AMCRandom64(int64_t min, int64_t max);
int32_t AMCRandom32(int32_t min, int32_t max);
int16_t AMCRandom16(int16_t min, int16_t max);
int8_t AMCRandom8(int8_t min, int8_t max);


#endif
