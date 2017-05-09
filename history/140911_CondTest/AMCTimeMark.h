/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: timeMark.h
	Description: 	
			This file provide simple interface to mark process running time within the program.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-24: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/
#ifndef	_AMC_TIME_MARK
#define	_AMC_TIME_MARK

void AMCTimeMark(void);
void AMCTimeMarkDetailed(char *file, int line);

#ifdef	DEBUG
#define	AMC_MARK_TIME()	AMCTimeMarkDetailed(__FILE__, __LINE__)
#else
#define	AMC_MARK_TIME()
#endif



#endif
