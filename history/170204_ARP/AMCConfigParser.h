/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCConfigParser.h
	Description: 	
			This file provide tools in reading configurations from file. 
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-10: File created as "configParser.h"
		2012-10-10: Change file name as "AMCConfigParser.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/
#ifndef	_AMC_CONFIG_PARSER
#define	_AMC_CONFIG_PARSER



#define	COMMON_CHAR		'#'
#define	EQUATION_CHAR		'='
#define	BLANK_CHARS		" \t\r\n"

#define	CONFIG_VALUE_BUFF_LEN	(128)
typedef struct
{
	char parameter[CONFIG_VALUE_BUFF_LEN];
	char value[CONFIG_VALUE_BUFF_LEN];
	char null;		/* prevent overflow */
}CONFIG_SPILIT_t;


int AMCConfigParser(CONFIG_SPILIT_t *config, const int configAmount, const char *fileName);





#endif
