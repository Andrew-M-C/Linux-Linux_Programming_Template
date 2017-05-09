/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCBmp.h
	Description: 	
			This file provide interfaces to create or modify a simple BMP file
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-22: File created as "simpleBmp.h"
		2012-08-23: Functions implemented
		2012-10-10: Change file name into "AMCBmp.h" from "simpleBmp.h"
		2012-10-10: Add "AMCBmp" prefix to functions and data types

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_BMP_H
#define	_AMC_BMP_H


//typedef long double interval_t;

typedef unsigned char AMCBmpMatrix_t;

typedef struct{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;		/* reserved */
}AMCPixel_st;

typedef struct{
	unsigned short magicNum;
	unsigned short colorDepth;
	unsigned int fileSize;
	unsigned int imageSize;
	unsigned int bmpStart;
	signed int bmpWidth;
	signed int bmpHeight;
	signed int bmpLineLen;
}AMCBmpInfo_st;

typedef struct{
	signed int x;
	signed int y;
}AMCCtsCodn_st;		/* Cartesian coordinates */

typedef struct{
	long double len;
	long double angle;
}AMCPolSodn_st;		/* Polar coordinates */

typedef enum{
	BE_OK = 0,			/* BMP error: OK */
	BE_WRERR,

	BE_UKNOWN
}AMCBmpErrno_t;


extern AMCBmpErrno_t drawErrno;


FILE *AMCBmpFileOpen(const char *filePath, const char * mode, AMCBmpInfo_st *bmpInfo);
FILE *AMCBmpFileCreate(const char *filePath, AMCBmpInfo_st *bmpInfo);
int AMCBmpFileClose(FILE *bmpFile);

int AMCBmpPixelRead(FILE *bmpFile, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y);
int AMCBmpPixelWrite(FILE *bmpFile, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y);

AMCBmpMatrix_t *AMCBmpDataMatrixMalloc(AMCBmpInfo_st * bmpInfo);
int AMCBmpDataMatrixFree(AMCBmpMatrix_t *pBuff);

int AMCBmpDataMatrixLoad(const AMCBmpInfo_st *bmpInfo, AMCBmpMatrix_t *pBuff, FILE *bmpFile);
int AMCBmpDataMatrixSave(const AMCBmpInfo_st *bmpInfo, AMCBmpMatrix_t *pBuff, FILE *bmpFile);

int AMCBmpDataMatrixPixelRead(AMCBmpMatrix_t *pBuff, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y);
int AMCBmpDataMatrixPixelWrite(AMCBmpMatrix_t *pBuff, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y);

void AMCBmpPerror(const char *preStr, AMCBmpErrno_t errnoToRslv);

AMCPixel_st AMCBmp_Pixel(unsigned char red, unsigned char green, unsigned char blue);
AMCCtsCodn_st AMCBmp_CTS(int x, int y);
AMCPolSodn_st AMCBmp_POL(long double len, long double angleInDegree);

int AMCBmpDrawLine_point2point
		(const AMCBmpInfo_st *bmpInfo, 
		AMCBmpMatrix_t *pBuff, 
		AMCPixel_st *pixel, 
		AMCCtsCodn_st pointA, AMCCtsCodn_st pointB);
int AMCBmpDrawLine_pointInPolar
		(const AMCBmpInfo_st *bmpInfo, 
		AMCBmpMatrix_t *pBuff, 
		AMCPixel_st *pixel, 
		AMCCtsCodn_st start, AMCPolSodn_st polar);

#endif


