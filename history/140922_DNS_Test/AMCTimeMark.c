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
#include <stdio.h>
#include <sys/time.h>

static struct timeval startTime = {0, 0};

static void staticPrintTime()
{
	struct timeval currTime;
	struct timezone dummyTZ;

	if (0 == startTime.tv_sec)
	{
		gettimeofday(&startTime, &dummyTZ);
		printf("000.000000sec\n");
	}
	else
	{
		gettimeofday(&currTime, &dummyTZ);
		currTime.tv_usec -= startTime.tv_usec;
		if ((signed int)(currTime.tv_usec) < 0)
		{
			currTime.tv_usec += 1000000;
			currTime.tv_sec -= (startTime.tv_sec + 1);
		}
		else
		{
			currTime.tv_sec -= startTime.tv_sec;
		}
		printf("%03ld.%06ldsec\n", currTime.tv_sec, currTime.tv_usec);
	}
}


void AMCTimeMark()
{
	printf("##TIME: ");
	staticPrintTime();
	return;
}

void AMCTimeMarkDetailed(char *file, int line)
{
	printf("##TIME: %s, Line %d: ", file, line);
	staticPrintTime();
	return;
}

