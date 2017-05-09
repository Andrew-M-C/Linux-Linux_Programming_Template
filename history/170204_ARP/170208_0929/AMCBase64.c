/*******************************************************************************
	Copyright (C), 2011-2014, Andrew Min Chang
	
	File name: AMCBase64.c
	Description: 	
			This file provide implementations of AMCBase64.h.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2014-09-05: File created as "AMCBase64.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCBase64.h"
#include <string.h>


static const char _charTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char _charPadding = '=';

#define	_RFC822_NUM		76

/************************************************************
 *  Text:  |       M       |       a       |       n       |
 *  ASCII: |      77       |      97       |      110      |
 *  Binary: 0 1 0 0 1 1 0 1 0 1 1 0 0 0 0 1 0 1 1 0 1 1 1 0
 *  Index: |     19    |     22    |     5     |     46    |
 *  Code:  |     T     |     W     |     F     |     u     |
 *  Part:  |_____0_____|_1_|___2___|___3___|_4_|_____5_____|
 */

#define	_ENC_P0_MASK	0xFC
#define	_ENC_P0_BITS	0x3F

#define	_ENC_P1_MASK	0x03
#define	_ENC_P1_BITS	0x03

#define	_ENC_P2_MASK	0xF0
#define	_ENC_P2_BITS	0x0F

#define	_ENC_P3_MASK	0x0F
#define	_ENC_P3_BITS	0x0F

#define	_ENC_P4_MASK	0xC0
#define	_ENC_P4_BITS	0x03

#define	_ENC_P5_MASK	0x3F
#define	_ENC_P5_BITS	0x3F


/******** Private functions ********/
#define __PRVATE_FUNCTIONS
#ifdef __PRVATE_FUNCTIONS

static unsigned char _charToByte(char c)
{
	if ((c >= 'A') && c <= 'Z')
	{
		return (c - 'A');
	}
	else if ((c >= 'a') && (c <= 'z'))
	{
		return (c - 'a' + ('Z' - 'A' + 1));
	}
	else if ((c >= '0') && ( c <= '9'))
	{
		return (c - '0' + ('Z' - 'A' + 1) + ('z' - 'a' + 1));
	}
	else if ('+' == c)
	{
		return 62;
	}
	else if ('/' == c)
	{
		return 63;
	}
	else
	{
		return 0xFF;
	}
}


#endif



/******** Public interfaces ********/
#define	__PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

/****/
/* Base64 encoding */
int AMCBase64Encode(const void *pInput, char *pOut, unsigned long inLen, BOOL useRfc822)
{
	unsigned long fromIdx, toIdx;
	unsigned char c0, c1, c2;
	unsigned char i0, i1, i2, i3;
	const unsigned char *pIn = pInput;
	unsigned long nextRetIdx = _RFC822_NUM;


	if ((NULL == pIn) ||
		(NULL == pOut))
	{
		return -1;
	}


	if (0 == inLen)
	{
		/* Are you kidding me? */
		return 0;
	}


	for (fromIdx = 0, toIdx = 0; 
		(fromIdx + 2) < inLen; 
		fromIdx += 3, toIdx += 4)
	{
		c0 = pIn[fromIdx + 0];
		c1 = pIn[fromIdx + 1];
		c2 = pIn[fromIdx + 2];

		i0  = (((c0 & _ENC_P0_MASK) >> 2) & _ENC_P0_BITS) << 0;
		i1  = (((c0 & _ENC_P1_MASK) >> 0) & _ENC_P1_BITS) << 4;
		i1 += (((c1 & _ENC_P2_MASK) >> 4) & _ENC_P2_BITS) << 0;
		i2  = (((c1 & _ENC_P3_MASK) >> 0) & _ENC_P3_BITS) << 2;
		i2 += (((c2 & _ENC_P4_MASK) >> 6) & _ENC_P4_BITS) << 0;
		i3  = (((c2 & _ENC_P5_MASK) >> 0) & _ENC_P5_BITS) << 0;


		pOut[toIdx + 0]  = _charTable[i0];
		pOut[toIdx + 1]  = _charTable[i1];
		pOut[toIdx + 2]  = _charTable[i2];
		pOut[toIdx + 3]  = _charTable[i3];


		if (useRfc822 && (nextRetIdx == toIdx + 4))
		{
			pOut[toIdx + 4] = '\n';

			toIdx ++;
			nextRetIdx += _RFC822_NUM + 1;
		}
	}



	if (0 == inLen - fromIdx)
	{}
	else if (1 == inLen - fromIdx)
	{
		c0 = pIn[fromIdx + 0];

		i0  = (((c0 & _ENC_P0_MASK) >> 2) & _ENC_P0_BITS) << 0;
		i1  = (((c0 & _ENC_P1_MASK) >> 0) & _ENC_P1_BITS) << 4;

		pOut[toIdx + 0]  = _charTable[i0];
		pOut[toIdx + 1]  = _charTable[i1];
		pOut[toIdx + 2]  = _charPadding;
		pOut[toIdx + 3]  = _charPadding;

		toIdx += 4;
	}
	else
	{
		c0 = pIn[fromIdx + 0];
		c1 = pIn[fromIdx + 1];

		i0  = (((c0 & _ENC_P0_MASK) >> 2) & _ENC_P0_BITS) << 0;
		i1  = (((c0 & _ENC_P1_MASK) >> 0) & _ENC_P1_BITS) << 4;
		i1 += (((c1 & _ENC_P2_MASK) >> 4) & _ENC_P2_BITS) << 0;
		i2  = (((c1 & _ENC_P3_MASK) >> 0) & _ENC_P3_BITS) << 2;

		pOut[toIdx + 0]  = _charTable[i0];
		pOut[toIdx + 1]  = _charTable[i1];
		pOut[toIdx + 2]  = _charTable[i2];
		pOut[toIdx + 3]  = _charPadding;

		toIdx += 4;
	}


	pOut[toIdx] = '\0';
	return 0;
}


/****/
/* Base64 Decoding */
int AMCBase64Decode(const char *pIn, void *pOutput, unsigned long *pOutLen)
{
	unsigned char c0, c1, c2, c3;
	unsigned char b0, b1, b2;
	unsigned long fromIdx, toIdx;
	unsigned long inLen;
	unsigned char *pOut = pOutput;
	int ret = 0;

	if ((NULL == pIn) ||
		(NULL == pOut) || 
		(NULL == pOutLen))
	{
		return -1;
	}

	inLen = strlen(pIn);
	if (0 == inLen)
	{
		pOut[0] = '\0';
		return 0;
	}


	fromIdx = 0;
	toIdx = 0;
	while (((fromIdx + 4) < inLen) &&
			(0 == ret))
	{
		c0 = _charToByte(pIn[fromIdx++]);
		fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;
		
		c1 = _charToByte(pIn[fromIdx++]);
		fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;

		c2 = _charToByte(pIn[fromIdx++]);
		fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;

		c3 = _charToByte(pIn[fromIdx++]);
		fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;


		if ((0xFF == c0) ||
			(0xFF == c1) ||
			(0xFF == c2) ||
			(0xFF == c3))
		{
			ret = -1;
		}
		else
		{
			b0  = (c0 & 0x3F) << 2;
			b0 += (c1 & 0x30) >> 4;
			b1  = (c1 & 0x0F) << 4;
			b1 += (c2 & 0x3C) >> 2;
			b2  = (c2 & 0x03) << 6;
			b2 += (c3 & 0x3F) << 0;

			pOut[toIdx + 0] = b0;
			pOut[toIdx + 1] = b1;
			pOut[toIdx + 2] = b2;

			toIdx += 3;
		}
	}



	if (0 == ret)
	{
		unsigned long remain = 0;
	
		/* ??__ */
		if (0 == inLen - fromIdx)
		{
			remain = 0;
		}
		else if (2 == inLen - fromIdx)
		{
			remain = 2;
		}
		/* ???_ */
		else if (3 == inLen - fromIdx)
		{
			remain = 3;
		}
		else
		{
			/* ??=* */
			if (_charPadding == pIn[fromIdx + 2])
			{
				remain = 2;
			}
			/* ???= */
			else if (_charPadding == pIn[fromIdx + 3])
			{
				remain = 3;
			}
			else if (4 == inLen - fromIdx)
			{
				remain = 4;
			}
			else
			{
				remain = -1;
			}
		}

		//printf ("remain = %ld (%ld - %ld)\n", remain, inLen, fromIdx);
		switch(remain)
		{
			case 0:
				break;
		
			case 2:
				c0 = _charToByte(pIn[fromIdx + 0]);
				c1 = _charToByte(pIn[fromIdx + 1]);

				b0  = (c0 & 0x3F) << 2;
				b0 += (c1 & 0x30) >> 4;

				pOut[toIdx + 0] = b0;

				toIdx += 1;
				break;

				
			case 3:
				c0 = _charToByte(pIn[fromIdx + 0]);
				c1 = _charToByte(pIn[fromIdx + 1]);
				c2 = _charToByte(pIn[fromIdx + 2]);

				b0  = (c0 & 0x3F) << 2;
				b0 += (c1 & 0x30) >> 4;
				b1  = (c1 & 0x0F) << 4;
				b1 += (c2 & 0x3C) >> 2;

				pOut[toIdx + 0] = b0;
				pOut[toIdx + 1] = b1;

				toIdx += 2;
				break;


			case 4:
				c0 = _charToByte(pIn[fromIdx++]);
				fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;
				
				c1 = _charToByte(pIn[fromIdx++]);
				fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;

				c2 = _charToByte(pIn[fromIdx++]);
				fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;

				c3 = _charToByte(pIn[fromIdx++]);
				fromIdx += (('\n' == pIn[fromIdx]) || ('\r' == pIn[fromIdx])) ? 1 : 0;

				if ((0xFF == c0) ||
					(0xFF == c1) ||
					(0xFF == c2) ||
					(0xFF == c3))
				{
					ret = -1;
				}
				else
				{
					b0  = (c0 & 0x3F) << 2;
					b0 += (c1 & 0x30) >> 4;
					b1  = (c1 & 0x0F) << 4;
					b1 += (c2 & 0x3C) >> 2;
					b2  = (c2 & 0x03) << 6;
					b2 += (c3 & 0x3F) << 0;

					pOut[toIdx + 0] = b0;
					pOut[toIdx + 1] = b1;
					pOut[toIdx + 2] = b2;

					toIdx += 3;
				}
				break;

			default:
				ret = -1;
				break;
		}
	}
	


	if (0 == ret)
	{
		*pOutLen = toIdx;
	}
	else
	{
		*pOutLen = 0;
	}
	return ret;
}



/****/
/* get required encode buffer size */
unsigned long AMCBase64RequiredEncodeBufferSize(unsigned long inLen, BOOL useRfc822)
{
	unsigned long ret;

	if (0 == inLen)
	{
		ret = 0;
	}
	else
	{
		ret = (inLen + 1) * 4 / 3;

		if (useRfc822)
		{
			ret += (inLen / _RFC822_NUM);
		}
	}

	return ret + 1;
}


#endif


/******** Duplicated function deciaration ********/
#define __DULICATED_FUNCTION_NAME
#ifdef __DULICATED_FUNCTION_NAME

unsigned long AMCBase64ReqEncBuffSize(unsigned long inLen, BOOL useRfc822)
{
	return AMCBase64RequiredEncodeBufferSize(inLen, useRfc822);
}


int AMCBase64Enc(const void *pIn, char *pOut, unsigned long inLen, BOOL useRfc822)
{
	return AMCBase64Encode(pIn, pOut, inLen, useRfc822);
}


int AMCBase64Dec(const char *pIn, void *pOut, unsigned long *pOutLen)
{
	return AMCBase64Decode(pIn, pOut, pOutLen);
}


#endif




