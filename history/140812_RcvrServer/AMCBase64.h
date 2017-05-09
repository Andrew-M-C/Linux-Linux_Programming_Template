/*******************************************************************************
	Copyright (C), 2011-2014, Andrew Min Chang
	
	File name: AMCBase64.h
	Description: 	
			This file provide interfaces to make Base64 encoding and decoding.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2014-09-05: File created as "AMCBase64.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_BASE_64_H
#define	_AMC_BASE_64_H

unsigned long AMCBase64ReqEncBuffSize(unsigned long inLen);
unsigned long AMCBase64ReqDecBuffSize(unsigned long inLen);

unsigned long AMCBase64RequiredEncodeBufferSize(unsigned long inLen);
unsigned long AMCBase64RequiredDecodeBufferSize(unsigned long inLen);

int AMCBase64Enc(const unsigned char *pIn, char *pOut, unsigned long inLen, unsigned long outLenLimit);
int AMCBase64Dec(const char *pIn, unsigned char *pOut, unsigned long inLen, unsigned long outLenLimit);

int AMCBase64Encode(const unsigned char *pIn, char *pOut, unsigned long inLen, unsigned long outLenLimit);
int AMCBase64Decode(const char *pIn, unsigned char *pOut, unsigned long inLen, unsigned long outLenLimit);


#endif
