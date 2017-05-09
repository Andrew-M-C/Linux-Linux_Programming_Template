/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCBmp.c
	Description: 	
			This file define function implements to create or modify a simple BMP file
			Please refer to "http://en.wikipedia.org/wiki/BMP_file_format"
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-08-22: File created as "simpleBmp.c"
		2012-08-23: Functions implemented
		2012-10-10: Change file name into "AMCBmp.c" from "simpleBmp.c"
		2012-10-10: Add "AMCBmp" prefix to functions and data types

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/* file functions */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/* memset */
#include <string.h>

/* stat() */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/* some bmp operation structure */
#include "AMCBmp.h"

/* drawing */
#include <math.h>

//#define	BMP_DEBUG

/* debug header */
#ifdef	BMP_DEBUG
#define	DEBUG
#include "AMCCommonLib.h"
#include "AMCTimeMark.h"
//#define	DB(x)	x
#else
#define	DB(x)
#endif


#ifndef	NULL
#define	NULL	((void *)0)
#endif


/* Bitmap file header */
#define	BMP_FILE_HEADER_TYPE_OFFSET		(0x0000)		/* 16-bits */
#define	BMP_FILE_HEADER_TYPE_HEADER_0		(0x42)	/* 'B' */
#define	BMP_FILE_HEADER_TYPE_HEADER_1		(0x4D)	/* 'M' */
#define	BMP_FILE_HEADER_FILE_SIZE_OFFSET		(0x0002)
#define	BMP_FILE_HEADER_DATA_START_OFFSET		(0x000A)
#define	BMP_FILE_MAGIC_NUM		(0x4D42)		/* "BM" */

/* DIB header */
#define	DIB_HEADER_DIB_SIZE_OFFSET		(0x000E)
#define	DIB_HEADER_IMAGE_WIDTH_OFFSET		(0x0012)
#define	DIB_HEADER_IMAGE_HEIGHT_OFFSET		(0x0016)
#define	DIB_HEADER_COLOR_PLANE_OFFSET		(0x001A)		/* 16-bits */
#define	DIB_HEADER_COLOR_DEPTH_OFFSET		(0x001C)		/* 16-bits */
#define	DIB_HEADER_COMPRESS_TYPE_OFFSET	(0x001E)
#define	DIB_HEADER_IMAGE_SIZE_OFFSET		(0x0022)
#define	DIB_HEADER_IMAGE_H_RESOLUTION_OFFSET		(0x0026)
#define	DIB_HEADER_IMAGE_V_RESOLUTION_OFFSET		(0x002A)

/* total header length */
#define	CFG_SIMPLE_BMP_HEADER_LEN		(0x0036)


/****************************************************
 * Functions to operate with BMP file or data buffer itself
 */
static int AMCBmpReadSingleData(void *buff, const void* source, size_t size, size_t offset)
{
	unsigned short tmpShort;
	unsigned int tmpLong;

	if (sizeof(char) == size)
	{
		*((unsigned char *)buff) = *(((unsigned char *)source) + offset);
		return 0;
	}
	else if(sizeof(short) == size)
	{
		tmpShort = (unsigned short)(*(((unsigned char *)source) + offset));
		tmpShort += ((unsigned short)(*(((unsigned char *)source) + offset + 1))) << 8;
		*((unsigned short *)buff) = tmpShort;
		return 0;
	}
	else if(sizeof(int) == size)
	{
		tmpLong = (unsigned int)(*(((unsigned char *)source) + offset));
		tmpLong += ((unsigned int)(*(((unsigned char *)source) + offset + 1))) << 8;
		tmpLong += ((unsigned int)(*(((unsigned char *)source) + offset + 2))) << 16;
		tmpLong += ((unsigned int)(*(((unsigned char *)source) + offset + 3))) << 24;
		*((unsigned int *)buff) = tmpLong;
		return 0;
	}
	else
	{
		errno = EINVAL;
		return -1;
	}
}

static int AMCBmpWriteSingleData(const void *buff, void* target, size_t size, size_t offset)
{
	unsigned short tmpShort;
	unsigned int tmpLong;

	DB(printf("Write in %04X, data %d, 0x", offset, *((unsigned short *)buff)));

	if (sizeof(char) == size)
	{
		*(((unsigned char *)target) + offset) = *((unsigned char *)buff);
		DB(printf("%02X.\n", 
					*(((unsigned char *)target) + offset + 0)));
		return 0;
	}
	else if(sizeof(short) == size)
	{
		tmpShort = *((unsigned short *)buff);
		*(((unsigned char *)target) + offset + 0) = (unsigned char)((tmpShort & 0x00FF) >> 0);
		*(((unsigned char *)target) + offset + 1) = (unsigned char)((tmpShort & 0xFF00) >> 8);
		DB(printf("%02X%02X.\n", 
					*(((unsigned char *)target) + offset + 1), 
					*(((unsigned char *)target) + offset + 0)));
		return 0;
	}
	else if(sizeof(int) == size)
	{
		tmpLong = *((unsigned int *)buff);
		*(((unsigned char *)target) + offset + 0) = (unsigned char)((tmpLong & 0x000000FF) >> 0);
		*(((unsigned char *)target) + offset + 1) = (unsigned char)((tmpLong & 0x0000FF00) >> 8);
		*(((unsigned char *)target) + offset + 2) = (unsigned char)((tmpLong & 0x00FF0000) >> 16);
		*(((unsigned char *)target) + offset + 3) = (unsigned char)((tmpLong & 0xFF000000) >> 24);
		DB(printf("%02X%02X%02X%02X.\n", 
					*(((unsigned char *)target) + offset + 3), 
					*(((unsigned char *)target) + offset + 2), 
					*(((unsigned char *)target) + offset + 1), 
					*(((unsigned char *)target) + offset + 0)));
		return 0;
	}
	else
	{
		errno = EINVAL;
		return -1;
	}
}

FILE *AMCBmpFileOpen(const char *filePath, const char * mode, AMCBmpInfo_st *bmpInfo)
{
	FILE *bmp;
	char readBuff[4];
	unsigned short u16Read;
	unsigned int u32Read;

	if ((NULL == bmpInfo) || (NULL) == filePath)
	{
		errno = EADDRNOTAVAIL;
		return NULL;
	}

	bmp = fopen(filePath, mode);
	if (NULL == bmp)
	{
		return NULL;
	}

	/* check magic number */
	fseek(bmp, BMP_FILE_HEADER_TYPE_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u16Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u16Read, readBuff, sizeof(u16Read), 0);
	bmpInfo->magicNum = u16Read;
	if (BMP_FILE_MAGIC_NUM != u16Read)
	{
		fprintf(stderr, "File format not supported.\n");
		goto ERR_RETURN;
	}

	/* check compression */
	fseek(bmp, DIB_HEADER_COMPRESS_TYPE_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	if (0 != u32Read)
	{
		fprintf(stderr, "BMP compression type %d not supported.\n", u32Read);
		goto ERR_RETURN;
	}

	/* check bmp color depth */
	fseek(bmp, DIB_HEADER_COLOR_DEPTH_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u16Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u16Read, readBuff, sizeof(u16Read), 0);
	bmpInfo->colorDepth = u16Read;
	if (24 != u16Read)
	{
		fprintf(stderr, "BMP color depth %d not supported.\n", u16Read);
		goto ERR_RETURN;
	}

	/* get BMP configurations */
	/* file size */
	fseek(bmp, BMP_FILE_HEADER_FILE_SIZE_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	bmpInfo->fileSize = u32Read;
	

	/* image data size */
	fseek(bmp, DIB_HEADER_IMAGE_SIZE_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	bmpInfo->imageSize = u32Read;
	
	
	/* bmp data start offset */
	fseek(bmp, BMP_FILE_HEADER_DATA_START_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	bmpInfo->bmpStart = u32Read;
	

	/* width */
	fseek(bmp, DIB_HEADER_IMAGE_WIDTH_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	bmpInfo->bmpWidth = (signed int)u32Read;
	

	/* height */
	fseek(bmp, DIB_HEADER_IMAGE_HEIGHT_OFFSET, SEEK_SET);
	if (0 == fread(readBuff, 1, sizeof(u32Read), bmp))
	{
		goto ERR_RETURN;
	}
	AMCBmpReadSingleData(&u32Read, readBuff, sizeof(u32Read), 0);
	bmpInfo->bmpHeight = (signed int)u32Read;

	if (0 == (bmpInfo->bmpHeight))
	{
		goto ERR_RETURN;
	}
	else
	{
		bmpInfo->bmpLineLen = (bmpInfo->imageSize) / (bmpInfo->bmpHeight);
	}

	DB(printf("File magic number: %d.\n", bmpInfo->magicNum));
	DB(printf("Color depth: %d bytes.\n", bmpInfo->colorDepth));
	DB(printf("BMP file size: %d bytes.\n", bmpInfo->fileSize));
	DB(printf("Data array size: %d bytes.\n", bmpInfo->imageSize));
	DB(printf("Data array starts: 0x%x.\n", bmpInfo->bmpStart));
	DB(printf("Image width: %d.\n", bmpInfo->bmpWidth));
	DB(printf("Image Height: %d.\n", bmpInfo->bmpHeight));
	DB(printf("Image data-len %d.\n", bmpInfo->bmpLineLen));
	
/* success return */
	return bmp;

ERR_RETURN:
	fclose(bmp);
	return NULL;
}


int AMCBmpPixelRead(FILE *bmpFile, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y)
{
	char readBuff[sizeof(AMCPixel_st)];
	int pixelPosition;

	if ((NULL == bmpFile) || (NULL == bmpInfo) || (NULL == pixel))
	{
		errno = EINVAL;
		return -1;
	}

	if ((x >= (bmpInfo->bmpWidth)) || (y >= (bmpInfo->bmpHeight)) ||
		(x < 0) || (y < 0))
	{
		errno = EINVAL;
		return -1;
	}

	pixelPosition = (x * (bmpInfo->colorDepth) / 8) + (y * bmpInfo->bmpLineLen);
	pixelPosition += bmpInfo->bmpStart;
	fseek(bmpFile, pixelPosition, SEEK_SET);
	if (0 == fread(readBuff, sizeof(unsigned char), (bmpInfo->colorDepth) / 8, bmpFile))
	{
		perror("read pixel");
		return -1;
	}
	AMCBmpReadSingleData(&(pixel->blue), readBuff, sizeof(pixel->blue), 0);
	AMCBmpReadSingleData(&(pixel->green), readBuff, sizeof(pixel->green), 1);
	AMCBmpReadSingleData(&(pixel->red), readBuff, sizeof(pixel->red), 2);
	AMCBmpReadSingleData(&(pixel->alpha), readBuff, sizeof(pixel->alpha), 3);

	//DB(printf("<%04X, %04X>", (y * bmpInfo->bmpLineLen), (x * (bmpInfo->colorDepth) / 8)));
	DB(printf("(%d, %d) R_%03d G_%03d B_%03d\n", x, y, pixel->red, pixel->green, pixel->blue));

	return 0;
}

int AMCBmpPixelWrite(FILE *bmpFile, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y)
{
	char writeBuff[sizeof(AMCPixel_st)];
	int pixelPosition;

	if ((NULL == bmpFile) || (NULL == bmpInfo) || (NULL == pixel))
	{
		errno = EINVAL;
		return -1;
	}

	if ((x >= (bmpInfo->bmpWidth)) || (y >= (bmpInfo->bmpHeight)) ||
		(x < 0) || (y < 0))
	{
		errno = EINVAL;
		return -1;
	}

	pixelPosition = (x * (bmpInfo->colorDepth) / 8) + (y * bmpInfo->bmpLineLen);
	pixelPosition += bmpInfo->bmpStart;
	fseek(bmpFile, pixelPosition, SEEK_SET);

	AMCBmpWriteSingleData(&(pixel->blue), writeBuff, sizeof(pixel->blue), 0);
	AMCBmpWriteSingleData(&(pixel->green), writeBuff, sizeof(pixel->green), 1);
	AMCBmpWriteSingleData(&(pixel->red), writeBuff, sizeof(pixel->red), 2);
	AMCBmpWriteSingleData(&(pixel->alpha), writeBuff, sizeof(pixel->alpha), 3);

	if (0 == fwrite(writeBuff, sizeof(unsigned char), (bmpInfo->colorDepth) / 8, bmpFile))
	{
		perror("write pixel");
		return -1;
	}

	//DB(printf("<%04X, %04X>", (y * bmpInfo->bmpLineLen), (x * (bmpInfo->colorDepth) / 8)));
	DB(printf("(%d, %d) R_%03d G_%03d B_%03d (write)\n", x, y, pixel->red, pixel->green, pixel->blue));

	return 0;
}


FILE *AMCBmpFileCreate(const char *filePath, AMCBmpInfo_st *bmpInfo)
{
	//struct stat dummyFileStat;
	unsigned char bmpHeader[CFG_SIMPLE_BMP_HEADER_LEN];
	size_t pad;
	unsigned short u16Tmp;
	unsigned int u32Tmp;
	FILE *bmpFile;
	unsigned char *array = NULL;

	if ((NULL == bmpInfo) || (NULL) == filePath)
	{
		errno = EADDRNOTAVAIL;
		return NULL;
	}

#if 0
	/* test if file exists */
	if (0 == stat(filePath, &dummyFileStat))
	{
		errno = EEXIST;
		free(dummyFileStat);
		return NULL;
	}
#endif

	/* generate information */
	bmpInfo->magicNum = BMP_FILE_MAGIC_NUM;
	bmpInfo->colorDepth = 24;

	pad = ((bmpInfo->bmpWidth) * bmpInfo->colorDepth / 8) % 4;
	if (0 == pad)
	{
		bmpInfo->bmpLineLen = ((bmpInfo->bmpWidth) * 3);
	}
	else
	{
		bmpInfo->bmpLineLen = ((bmpInfo->bmpWidth) * 3) + (4 - pad);
	}

	bmpInfo->imageSize = (bmpInfo->bmpLineLen) * (bmpInfo->bmpHeight);
	bmpInfo->fileSize = (bmpInfo->imageSize) + CFG_SIMPLE_BMP_HEADER_LEN;
	bmpInfo->bmpStart = CFG_SIMPLE_BMP_HEADER_LEN;

	/* fill in the data */
	memset(bmpHeader, 0, sizeof(bmpHeader));
	AMCBmpWriteSingleData(&(bmpInfo->magicNum), bmpHeader, sizeof(bmpInfo->magicNum), BMP_FILE_HEADER_TYPE_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->fileSize), bmpHeader, sizeof(bmpInfo->fileSize), BMP_FILE_HEADER_FILE_SIZE_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->bmpStart), bmpHeader, sizeof(bmpInfo->bmpStart), BMP_FILE_HEADER_DATA_START_OFFSET);
	u32Tmp = CFG_SIMPLE_BMP_HEADER_LEN - DIB_HEADER_DIB_SIZE_OFFSET;
	AMCBmpWriteSingleData(&u32Tmp, bmpHeader, sizeof(u32Tmp), DIB_HEADER_DIB_SIZE_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->bmpWidth), bmpHeader, sizeof(bmpInfo->bmpWidth), DIB_HEADER_IMAGE_WIDTH_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->bmpHeight), bmpHeader, sizeof(bmpInfo->bmpHeight), DIB_HEADER_IMAGE_HEIGHT_OFFSET);
	u16Tmp = 1;
	AMCBmpWriteSingleData(&u16Tmp, bmpHeader, sizeof(u16Tmp), DIB_HEADER_COLOR_PLANE_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->colorDepth), bmpHeader, sizeof(bmpInfo->colorDepth), DIB_HEADER_COLOR_DEPTH_OFFSET);
	u32Tmp = 0;
	AMCBmpWriteSingleData(&u32Tmp, bmpHeader, sizeof(u32Tmp), DIB_HEADER_COMPRESS_TYPE_OFFSET);
	AMCBmpWriteSingleData(&(bmpInfo->imageSize), bmpHeader, sizeof(bmpInfo->imageSize), DIB_HEADER_IMAGE_SIZE_OFFSET);

	bmpFile = fopen(filePath, "w");
	if (NULL == bmpFile)
	{	
		return NULL;
	}

	array = malloc((size_t)(bmpInfo->imageSize));
	if (NULL == array)
	{
		fclose(bmpFile);
		return NULL;
	}
	memset(array, 0, (size_t)(bmpInfo->imageSize));

	if (0 == fwrite(bmpHeader, sizeof(unsigned char), sizeof(bmpHeader), bmpFile))
	{
		goto ERR_RETURN;
	}

	if (0 == fwrite(array, sizeof(unsigned char), (size_t)(bmpInfo->imageSize), bmpFile))
	{
		goto ERR_RETURN;
	}

	DB(printf("File magic number: %d.\n", bmpInfo->magicNum));
	DB(printf("Color depth: %d bytes.\n", bmpInfo->colorDepth));
	DB(printf("BMP file size: %d bytes.\n", bmpInfo->fileSize));
	DB(printf("Data array size: %d bytes.\n", bmpInfo->imageSize));
	DB(printf("Data array starts: 0x%x.\n", bmpInfo->bmpStart));
	DB(printf("Image width: %d.\n", bmpInfo->bmpWidth));
	DB(printf("Image height: %d.\n", bmpInfo->bmpHeight));
	DB(printf("Image data-len %d.\n", bmpInfo->bmpLineLen));

	free(array);
	return bmpFile;

ERR_RETURN:
	free(array);
	fclose(bmpFile);
	return NULL;
}


int AMCBmpFileClose(FILE *bmpFile)
{
	return fclose(bmpFile);
}


AMCBmpMatrix_t *AMCBmpDataMatrixMalloc(AMCBmpInfo_st * bmpInfo)
{
	size_t pad;
	
	pad = ((bmpInfo->bmpWidth) * bmpInfo->colorDepth / 8) % 4;
	if (0 == pad)
	{
		bmpInfo->bmpLineLen = ((bmpInfo->bmpWidth) * 3);
	}
	else
	{
		bmpInfo->bmpLineLen = ((bmpInfo->bmpWidth) * 3) + (4 - pad);
	}

	bmpInfo->imageSize = (bmpInfo->bmpLineLen) * (bmpInfo->bmpHeight);

	return malloc((size_t)(bmpInfo->imageSize));
}

int AMCBmpDataMatrixFree(AMCBmpMatrix_t *pBuff)
{
	free(pBuff);
	pBuff = (void *)0;
	return 0;
}

int AMCBmpDataMatrixLoad(const AMCBmpInfo_st *bmpInfo, AMCBmpMatrix_t *pBuff, FILE *bmpFile)
{
	size_t readCount;

	fseek(bmpFile, bmpInfo->bmpStart, SEEK_SET);
	readCount = fread(pBuff, sizeof(AMCBmpMatrix_t), (size_t)(bmpInfo->imageSize), bmpFile);
	if ((size_t)(bmpInfo->imageSize) != readCount)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}


int AMCBmpDataMatrixSave(const AMCBmpInfo_st *bmpInfo, AMCBmpMatrix_t *pBuff, FILE *bmpFile)
{
	size_t writeCount;

	fseek(bmpFile, bmpInfo->bmpStart, SEEK_SET);
	writeCount = fwrite(pBuff, sizeof(AMCBmpMatrix_t), (size_t)(bmpInfo->imageSize), bmpFile);
	if ((size_t)(bmpInfo->imageSize) != writeCount)
	{
		return -1;
	}
	else
	{
		return fflush(bmpFile);
	}
}


int AMCBmpDataMatrixPixelRead(AMCBmpMatrix_t *pBuff, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y)
{
	int pixelPosition;

	if ((NULL == pBuff) || (NULL == bmpInfo) || (NULL == pixel))
	{
		errno = EINVAL;
		return -1;
	}

	if ((x >= (bmpInfo->bmpWidth)) || (y >= (bmpInfo->bmpHeight)) ||
		(x < 0) || (y < 0))
	{
		errno = EINVAL;
		return -1;
	}

	pixelPosition = (x * (bmpInfo->colorDepth) / 8) + (y * bmpInfo->bmpLineLen);

	pixel->blue = *(pBuff + pixelPosition + 0);
	pixel->green = *(pBuff + pixelPosition + 1);
	pixel->red = *(pBuff + pixelPosition + 2);
	if ((bmpInfo->colorDepth) > 24)
	{
		pixel->alpha = *(pBuff + pixelPosition + 3);
	}

	DB(printf("(%d, %d) R_%03d G_%03d B_%03d\n", x, y, pixel->red, pixel->green, pixel->blue));

	return 0;
}

int AMCBmpDataMatrixPixelWrite(AMCBmpMatrix_t *pBuff, const AMCBmpInfo_st *bmpInfo, AMCPixel_st *pixel, const int x, const int y)
{
	int pixelPosition;

	if ((NULL == pBuff) || (NULL == bmpInfo) || (NULL == pixel))
	{
		errno = EINVAL;
		return -1;
	}

	if ((x >= (bmpInfo->bmpWidth)) || 
		(y >= (bmpInfo->bmpHeight)) || 
		(x < 0) || (y < 0))
	{
		errno = EINVAL;
		return -1;
	}

	pixelPosition = (x * (bmpInfo->colorDepth) / 8) + (y * bmpInfo->bmpLineLen);

	*(pBuff + pixelPosition + 0) = pixel->blue;
	*(pBuff + pixelPosition + 1) = pixel->green;
	*(pBuff + pixelPosition + 2) = pixel->red;
	if ((bmpInfo->colorDepth) > 24)
	{
		*(pBuff + pixelPosition + 3) = pixel->alpha;
	}

	//DB(printf("<%d, %d: 0x%X>", (x * (bmpInfo->colorDepth) / 8), y, pixelPosition));
	DB(printf("(%d, %d) R_%03d G_%03d B_%03d (write)\n", x, y, pixel->red, pixel->green, pixel->blue));

	return 0;
}




/****************************************************
 * Functions to draw in BMP buffer
 * 	(no operation directly with file supporded)
 */

AMCBmpErrno_t drawErrno;

const char *bmpErrStr[] = 
{
	"Success",
	"Write into data buffer error",

	/* should NOT change this string */
	"Unknown error"
};


AMCPixel_st AMCBmp_Pixel(unsigned char red, unsigned char green, unsigned char blue)
{
	AMCPixel_st value;
	value.red = red;
	value.green = green;
	value.blue = blue;
	return value;
}

AMCCtsCodn_st AMCBmp_CTS(int x, int y)
{
	AMCCtsCodn_st value;
	value.x = x;
	value.y = y;
	return value;
}

AMCPolSodn_st AMCBmp_POL(long double len, long double angleInDegree)
{
	AMCPolSodn_st value;
	value.len = len;
	value.angle = M_PI * angleInDegree / 180.0;
	return value;
}

static int staticMaxInt(int a, int b)
{
	return ((a > b)? a : b);
}

static int staticMinInt(int a, int b)
{
	return ((a > b)? b : a);
}

void AMCBmpPerror(const char *preStr, AMCBmpErrno_t errnoToRslv)
{
	int errorArraySize;

	if (NULL == preStr)
	{
		fprintf(stderr, "%s.\n", bmpErrStr[errnoToRslv]);
	}
	else
	{
		/* get error size */
		errorArraySize = 0;
		while (0 != strcmp(bmpErrStr[errorArraySize], "Unknown error"))
		{
			errorArraySize++;
		}

		if (errnoToRslv <= errorArraySize)
		{
			fprintf(stderr, "%s: %s.\n", preStr, bmpErrStr[errnoToRslv]);
		}
		else
		{
			fprintf(stderr, "%s: %s.\n", preStr, bmpErrStr[errorArraySize]);
		}
	}
}

int AMCBmpDrawLine_point2point
		(const AMCBmpInfo_st *bmpInfo, 
		AMCBmpMatrix_t *pBuff, 
		AMCPixel_st *pixel, 
		const AMCCtsCodn_st pointA, const AMCCtsCodn_st pointB)
{
#define	BMP_LINE_INTER_SWAP(x, y)		swapBuf = (x); (x) = (y); (y) = swapBuf

	/* http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm */
	/* http://www.cnblogs.com/soroman/archive/2006/07/27/509602.html */
	int tmpX, tmpY;
	int deltaX, deltaY,error;
	int swapBuf;
	int x0, y0, x1, y1;
	int isError = 0;

	x0 = pointA.x;
	y0 = pointA.y;
	x1 = pointB.x;
	y1 = pointB.y;


	AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, pointA.x, pointA.y);
	AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, pointB.x, pointB.y);

	if ((x0 == x1) && (y0 == y1))
	{
		return 0;
	}
	else if (x0 == x1)
	{
		tmpX = x0;
		for (tmpY = staticMinInt(y0, y1) + 1; tmpY < staticMaxInt(y0, y1); tmpY++)
		{
			if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
			{
				isError = -1;
			}
		}
	}
	else if (y0 == y1)
	{
		tmpY = y0;
		for (tmpX = staticMinInt(x0, x1) + 1; tmpX < staticMaxInt(x0, x1); tmpX++)
		{
			if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
			{
				isError = -1;
			}
		}
	}
	else if (abs(x1 - x0) > abs(y1 - y0))
	{
		if (x0 > x1)
		{
			BMP_LINE_INTER_SWAP(x0, x1);
			BMP_LINE_INTER_SWAP(y0, y1);
		}

		tmpX = x0 + 1; 
		tmpY = y0;

		if (y1 > y0)
		{
			DB(printf("##MARK: %s, %d\n", __FILE__, __LINE__));
			deltaX = x1 - x0;
			deltaY = y1 - y0;
			error = deltaY * 2 - deltaX;	/* doubled */
			DB(printf("deltaX=%d, deltaY=%d\n", deltaX, deltaY));
		
			while (tmpX < x1)
			{
				DB(printf("error=%d, ", error));
				if (error > 0)
				{
					error += (deltaY + deltaY - deltaX - deltaX);
					tmpY++;
				}
				else
				{
					error += (deltaY + deltaY);
				}

				if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
				{
					isError = -1;
				}

				tmpX++;
			}
		}
		else
		{
			DB(printf("##MARK: %s, %d\n", __FILE__, __LINE__));
			deltaX = x1 - x0;
			deltaY = y0 - y1;
			error = deltaY * 2 - deltaX;	/* doubled */
			DB(printf("deltaX=%d, deltaY=%d, x1=%d\n", deltaX, deltaY, x1));
		
			while (tmpX < x1)
			{
				

				DB(printf("error=%d, ", error));
				if (error > 0)
				{
					error += (deltaY + deltaY - deltaX - deltaX);
					tmpY --;
				}
				else
				{
					error += (deltaY + deltaY);
				}

				if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
				{
					isError = -1;
				}

				tmpX++;
			}
		}
	}
	else
	{
		if (y0 > y1)
		{
			BMP_LINE_INTER_SWAP(x0, x1);
			BMP_LINE_INTER_SWAP(y0, y1);
		}

		tmpX = x0; 
		tmpY = y0 + 1;

		if (x1 > x0)
		{
			DB(printf("##MARK: %s, %d\n", __FILE__, __LINE__));
			deltaX = x1 - x0;
			deltaY = y1 - y0;
			error = deltaX * 2 - deltaY;	/* doubled */
			DB(printf("deltaX=%d, deltaY=%d\n", deltaX, deltaY));
		
			while (tmpY < y1)
			{
				DB(printf("error=%d, ", error));
				if (error > 0)
				{
					error += (deltaX + deltaX - deltaY - deltaY);
					tmpX++;
				}
				else
				{
					error += (deltaX + deltaX);
				}

				if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
				{
					isError = -1;
				}

				tmpY++;
			}
		}
		else
		{
			DB(printf("##MARK: %s, %d\n", __FILE__, __LINE__));
			deltaX = x0 - x1;
			deltaY = y1 - y0;
			error = deltaX * 2 - deltaY;	/* doubled */
			DB(printf("deltaX=%d, deltaY=%d, x1=%d\n", deltaX, deltaY,x1));
		
			while (tmpY < y1)
			{
				

				DB(printf("error=%d, ", error));
				if (error > 0)
				{
					error += (deltaX + deltaX - deltaY - deltaY);
					tmpX --;
				}
				else
				{
					error += (deltaX + deltaX);
				}

				if (0 != AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY))
				{
					isError = -1;
				}

				tmpY++;
			}
		}
	}

#if 0
	int tmpX, tmpY;
	interval_t slope;
	interval_t offset;

	AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, pointA.x, pointA.y);
	AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, pointB.x, pointB.y);

	if ((pointB.x == pointA.x) && (pointB.y == pointA.y))
	{
		return 0;
	}

	if ((staticMaxInt(pointA.x, pointB.x) - staticMinInt(pointA.x, pointB.x)) >= 
		(staticMaxInt(pointA.y, pointB.y) - staticMinInt(pointA.y, pointB.y)))
	{
		slope = ((interval_t)(pointB.y - pointA.y)) / ((interval_t)(pointB.x - pointA.x));
		offset = ((interval_t)(pointA.y * pointB.x - pointB.y * pointA.x)) / 
				((interval_t)(pointB.x - pointA.x));

		for (tmpX = staticMinInt(pointA.x, pointB.x) + 1; 
			tmpX < staticMaxInt(pointA.x, pointB.x);
			tmpX++)
		{
			tmpY = (int)(tmpX * slope + offset);
			AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY);
		}
	}
	else
	{
		slope = ((interval_t)(pointB.x - pointA.x)) / ((interval_t)(pointB.y - pointA.y));
		offset = ((interval_t)(pointA.x * pointB.y - pointB.x * pointA.y)) / 
				((interval_t)(pointB.y - pointA.y));

		for (tmpY = staticMinInt(pointA.y, pointB.y) + 1;
			tmpY < staticMaxInt(pointA.y, pointB.y);
			tmpY++)
		{
			tmpX = (int)(tmpY * slope + offset);
			AMCBmpDataMatrixPixelWrite(pBuff, bmpInfo, pixel, tmpX, tmpY);
		}
	}
#endif

	return isError;
#undef	BMP_LINE_INTER_SWAP
}


int AMCBmpDrawLine_pointInPolar
		(const AMCBmpInfo_st *bmpInfo, 
		AMCBmpMatrix_t *pBuff, 
		AMCPixel_st *pixel, 
		AMCCtsCodn_st start, AMCPolSodn_st polar)
{
	AMCCtsCodn_st end;

	end.x = start.x + (int)(polar.len * cosl(polar.angle));
	end.y = start.y + (int)(polar.len * sinl(polar.angle));

	return AMCBmpDrawLine_point2point(bmpInfo, pBuff, pixel, start, end);
}



