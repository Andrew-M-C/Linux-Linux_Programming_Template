/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCConfigParser.c
	Description: 	
			This file provide tools in reading configurations from file. 
	IMPORTANT:
			This tool returns including the '\n' or '\r' charcters. 
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-10: File created as "configParser.c"
		2012-10-10: Change file name as "AMCConfigParser.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/* This is a common file */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "AMCConfigParser.h"	/* some common macros */


//#define	PARSER_DEBUG

#ifdef	PARSER_DEBUG
#define	DB(x)	x
#else
#define	DB(x)
#endif

#define	FALSE	(0)
#define	TRUE	(!FALSE)

#ifndef	NULL
#define	NULL	((void *)0)
#endif

#ifndef	BOOL
#define	BOOL	int
#endif

#ifndef	LENOF
#define	LENOF(x)	(sizeof(x) / sizeof(*(x)))
#endif

#define	CFG_LINE_READ_BUFF_SIZE		(1024)

static const char constBlankChar[] = BLANK_CHARS;
#define	CONST_STRLEN(x)	(sizeof(x) - 1)


/* return blank amounts */
static int staticSkipBlank(char **str, size_t limit)
{
	int tmp;
	int charIndex;
	BOOL isSkipOK;
	const char blankChars[] = BLANK_CHARS;
	const int blankCharsAmount = LENOF(blankChars) - 1;

	for (tmp = 0, isSkipOK = FALSE;
		(tmp < limit) && (FALSE == isSkipOK); 
		tmp++, (*str)++)
	{
		/* The "LENOF(blankChars) - 1)" is necessary, ignoring the last '\0' is very important */
		for (charIndex = 0; 
			(charIndex < blankCharsAmount) && (blankChars[charIndex] != (**str)); 
			charIndex++)
		{
			/* simply search */
		}

		if (blankCharsAmount == charIndex)
		{
			isSkipOK = TRUE;
		}
	}

	(*str)--;
	if (tmp >= limit)
	{
		return -1;
	}
	else
	{
		return (tmp - 1);
	}
}

static void staticClearEnter(char *str, size_t limit)
{
	int index;
	int tmp;

	for (index = 0; (index < limit) && ('\0' != str[index]); index ++)
	{
		if (('\r' == str[index]) || ('\n' == str[index]))
		{
			for (tmp = index; (tmp < limit -1) && ('\0' != str[tmp]); tmp++)
			{
				str[tmp] = str[tmp + 1];
			}
		}
	}

	str[limit - 1] = '\0';

	return;
}



int AMCConfigParser(CONFIG_SPILIT_t *config, const int configAmount, const char *fileName)
{
	FILE *confFile;
	char readBuff[CFG_LINE_READ_BUFF_SIZE];
	char *pBuff;
	int tmp;
	int lineSize;
	

	/* check parameter */
	if (NULL == config || NULL == fileName || configAmount < 0)
	{
		errno = EADDRNOTAVAIL;
		return -1;
	}
	if (0 == configAmount)
	{
		return 0;
	}

	/* parameter OK, go on */
	memset(readBuff, 0, sizeof(readBuff));
	confFile = fopen(fileName, "r");
	if (NULL == confFile)
	{
		return -1;
	}

	/* read file */
	while(NULL != fgets(readBuff, sizeof(readBuff), confFile))
	{
		pBuff = readBuff;
		lineSize = sizeof(readBuff);
		
		/* skip blank characters */
		lineSize -= staticSkipBlank(&pBuff, sizeof(readBuff));
		DB(printf("Buff: \n\"%s\".\n", pBuff));

		/* ignore comments */
		if (COMMON_CHAR == *pBuff)
		{
			/* This line is ivalid */
		}
		else if ('\0' == *pBuff)
		{
			/* This line is empty */
		}
		else
		{
			/* now parse the configuration */	
			/* Firstly, get parameter name */
			for (tmp = 0; tmp < configAmount; tmp++)
			{
				if (0 == strncmp(pBuff, config[tmp].parameter, strlen(config[tmp].parameter)))
				{
					DB(printf("Get parameter \"%s\".\n", config[tmp].parameter));
					lineSize -= strlen(config[tmp].parameter);
					pBuff += strlen(config[tmp].parameter);
					lineSize -= staticSkipBlank(&pBuff, lineSize);

					/* check equation symbol */
					if (EQUATION_CHAR == *pBuff)
					{
						pBuff++;
						lineSize--;
						lineSize -= staticSkipBlank(&pBuff, lineSize);
						strncpy(config[tmp].value, pBuff, sizeof(config[tmp].value));
						staticClearEnter(config[tmp].value, sizeof(config[tmp].value));
						DB(printf("Value: \"%s\".\n", pBuff));
					}
					else
					{
						DB(printf("Value NULL.\n"));
						config[tmp].value[0] = '\0';
					}

					break;
				}
			}

			if (tmp >= configAmount)
			{
				if ('\n' == readBuff[strlen(readBuff) - 1])
				{
					readBuff[strlen(readBuff) - 1] = '\0';
				}
				fprintf(stderr, "Line \"%s\" unrecognized.\n", readBuff);
			}
		}
	}


	
	fclose(confFile);
	return 0;
}



