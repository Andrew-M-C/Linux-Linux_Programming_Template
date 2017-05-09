/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCHash.h
	Description: 	
			This file provide simple open-sourced hash functions.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-16: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_HASH_H
#define _AMC_HASH_H

unsigned int RSHash(char *str);
unsigned int JSHash(char *str);
unsigned int PJWHash(char *str);
unsigned int ELFHash(char *str);
unsigned int BKDRHash(char *str);
unsigned int SDBMHash(char *str);
unsigned int DJBHash(char *str);
unsigned int APHash(char *str);

#endif
