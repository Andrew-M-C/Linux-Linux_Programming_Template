/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCMutableBuffer.c
	Description: 	
			This file provide implementations of AMCMutableBuffer.h.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-05: File created as "AMCMemPool.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCMemPool.h"
#include "AMCMutableBuffer.h"

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define _CFG_DEFAULT_CLUSTER_SIZE		(2048)
#define _CFG_DEFAULT_INIT_SIZE			(_CFG_DEFAULT_CLUSTER_SIZE * 10)
#define _CFG_AMC_MUTABLE_BUFFER_MAGIC_NUM	(0x36FE20AC)		// no special meaning, I juse type in random

//#define _DEBUG_FLAG

#ifdef _DEBUG_FLAG
#define _DEBUG(fmt, args...)		printf("##"__FILE__", %d: "fmt"\n", __LINE__, ##args)
#define _MARK()				_DEBUG("MARK")
#else
#define _DEBUG(fmt, args...)
#define _MARK()
#endif

#define MEMMOVE		memmove


static const char *_error_msg[AMCMutableBufferError_Types] = {
	"success",
	"system API error",
	"mutex error",
	"mempool error",
	"parameter error",
	"specified offset exceeds buffer length",
};


/**********/
#define __PRIVATE_MACROS
#ifdef __PRIVATE_MACROS



#endif


/********************/
#define __STRUCTURE_DEFINITIONS
#ifdef __STRUCTURE_DEFINITIONS

struct _buffer_header {
	struct _buffer_header *pNext;
	size_t                 dataLen;
	/* remaining is data */
};

struct AMCMutableBuffer {
	uint32_t                classMagic;
	struct AMCMemPool      *pMemPool;
	struct _buffer_header  *pStart;
	struct _buffer_header  *pLast;
	size_t                  clusterSize;
	size_t                  clusterCount;
	BOOL                    useLock;
	pthread_mutex_t         lock;
};


#endif


/********************/
#define __INTERNAL_FUNCTION_DECLARATIONS
#ifdef __INTERNAL_FUNCTION_DECLARATIONS

static inline uint8_t *_buffer_cluster_get_data (struct _buffer_header *header)
{
	return ((uint8_t*)header) + sizeof(*header);
}


static inline size_t _buffer_cluster_get_capacity (struct _buffer_header *header)
{
	return AMCMemPool_DataSize(header) - sizeof(*header);
}


// returns appended data length
static size_t _buffer_cluster_append_data (struct _buffer_header *header, const void *data, size_t dataLen)
{
	uint8_t *pData = _buffer_cluster_get_data(header);
	size_t clusterCap = _buffer_cluster_get_capacity(header);
	size_t availLen = clusterCap - header->dataLen;
	size_t copyLen = (dataLen < availLen) ? dataLen : availLen;

	_DEBUG("Append data with len %d, requested len %d, mempool size %d", copyLen, dataLen, clusterCap);
	memcpy(pData + header->dataLen, data, copyLen);
	header->dataLen += copyLen;

	return copyLen;
}


// returns deleted data length
static size_t _buffer_cluster_delete_data (struct _buffer_header *header, const size_t offset, size_t deleteLen, BOOL *pIsClusterEmptyOut)
{
	uint8_t *pData = _buffer_cluster_get_data(header);
	size_t ret = 0;
	size_t remainLen = header->dataLen;

	_DEBUG("Cluster %p len %d, request delete len %d", header, remainLen, deleteLen);

	if (offset > remainLen) {
		goto ENDS;
	}

	pData     += offset;
	remainLen -= offset;

	if (remainLen > deleteLen)
	{
		remainLen       -= deleteLen;
		header->dataLen -= deleteLen;
		ret              = deleteLen;

		MEMMOVE(pData, pData + deleteLen, remainLen);
		_DEBUG("Delete cluster data from %d with length %d, remaining %d", offset, deleteLen, header->dataLen);
	}
	else
	{
		header->dataLen -= remainLen;
		ret              = remainLen;
		_DEBUG("Delete cluster data from %d with length %d, remaining %d", offset, remainLen, header->dataLen);
	}

	/****/
	/* return */
ENDS:
	if (pIsClusterEmptyOut)
	{
		*pIsClusterEmptyOut = (0 == header->dataLen);
	}

	return ret;
}


static struct _buffer_header *_buffer_alloc_new_cluster (struct AMCMutableBuffer *buffer)
{
	struct _buffer_header *ret = AMCMemPool_Alloc(buffer->pMemPool);

	if (ret)
	{
		ret->dataLen = 0;
		ret->pNext = NULL;
#ifdef _DEBUG_FLAG
		memset(ret, 0, sizeof(*ret) + buffer->clusterSize);
#endif

		if (NULL == buffer->pStart) {
			buffer->pStart = ret;
		}
		else {
			buffer->pLast->pNext = ret;
		}
		buffer->pLast = ret;
		buffer->clusterCount ++;

		_DEBUG("Allocate new buffer cluster, current cluster count %d", buffer->clusterCount);
	}

	return ret;
}


static struct _buffer_header *_buffer_alloc_new_cluster_and_insert_after (struct AMCMutableBuffer *buffer, struct _buffer_header *prevCluster)
{
	struct _buffer_header *ret = AMCMemPool_Alloc(buffer->pMemPool);

	if (NULL == ret) {
		return NULL;
	}

	ret->dataLen = 0;
	ret->pNext = NULL;
#ifdef _DEBUG_FLAG
	memset(ret, 0, sizeof(*ret) + buffer->clusterSize);
#endif

	/* at the beginning */
	if (NULL == prevCluster)
	{
		ret->pNext = buffer->pStart;
		buffer->pStart = ret;

		if (NULL == buffer->pLast) {
			buffer->pLast = ret;
		}

		buffer->clusterCount ++;
	}
	else
	{
		ret->pNext = prevCluster->pNext;
		prevCluster->pNext = ret;

		if (prevCluster == buffer->pLast) {
			buffer->pLast = ret;
		}

		buffer->clusterCount ++;
	}

	_DEBUG("Allocate and insert new buffer cluster after %p, current cluster count %d", prevCluster, buffer->clusterCount);

	return ret;
}


static AMCMutableBufferError_t _buffer_free_cluster (struct AMCMutableBuffer *buffer, struct _buffer_header *cluster, struct _buffer_header *nullablePrevCluster)
{
	const AMCMutableBufferError_t ret = 0;

	/****/
	/* locate previous buffer cluster */
	struct _buffer_header *prevCluster = nullablePrevCluster;
	if (NULL == prevCluster)
	{
		if (cluster == buffer->pStart)
		{
			/* this cluster is the start cluster, no prev cluster */
		}
		else
		{
			prevCluster = buffer->pStart;
			while(cluster != prevCluster->pNext)
			{
				prevCluster = prevCluster->pNext;
			}
		}
	}

	_DEBUG("Free buffer cluster (%p), current cluster count %d", cluster, buffer->clusterCount - 1);

	/****/
	/* delete specified buffer cluster */
	if (NULL == prevCluster)
	{
		buffer->pStart = cluster->pNext;
		buffer->clusterCount -= 1;

#ifdef _DEBUG_FLAG
		memset(cluster, 0, AMCMemPool_DataSize(cluster));
#endif
		AMCMemPool_Free(cluster);
		cluster = NULL;
	}
	else
	{
		if (cluster == buffer->pLast) {
			buffer->pLast = prevCluster;
		}

		prevCluster->pNext = cluster->pNext;
		buffer->clusterCount -= 1;

#ifdef _DEBUG_FLAG
		memset(cluster, 0, AMCMemPool_DataSize(cluster));
#endif
		AMCMemPool_Free(cluster);
		cluster = NULL;
	}

	/****/
	/* ENDS */
	return ret;
}


static void _buffer_LOCK (struct AMCMutableBuffer *buffer)
{
	if (buffer->useLock) {
		pthread_mutex_lock(&(buffer->lock));
	}
	return;
}


static void _buffer_UNLOCK (struct AMCMutableBuffer *buffer)
{
	if (buffer->useLock) {
		pthread_mutex_unlock(&(buffer->lock));
	}
	return;
}


static BOOL _buffer_is_correct_type (struct AMCMutableBuffer *buffer)
{
	if (NULL == buffer)
	{
		_DEBUG("buffer NULL");
		return FALSE;
	}
	else if (_CFG_AMC_MUTABLE_BUFFER_MAGIC_NUM != buffer->classMagic)
	{
		_DEBUG("buffer is not buffer");
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


size_t _buffer_get_len(struct AMCMutableBuffer *buffer)
{
	if (0 == buffer->clusterCount)
	{
		return 0;
	}
	else if (1 == buffer->clusterCount)
	{
		return buffer->pStart->dataLen;
	}
	else
	{
		size_t ret = 0;
		struct _buffer_header *pHeader = buffer->pStart;
		
		while (pHeader)
		{
			ret += pHeader->dataLen;
			pHeader = pHeader ->pNext;
		}

		return ret;
	}
}


AMCMutableBufferError_t _buffer_append (struct AMCMutableBuffer *buffer, const void *data, size_t dataLen)
{
	AMCMutableBufferError_t ret = 0;

	if ((NULL == data) || (0 == dataLen))
	{
		return 0;
	}

	_DEBUG("Request append data length: %d", dataLen);

	/****/
	const uint8_t *pData = (const uint8_t *)data;
	size_t remainLen = dataLen;
	struct _buffer_header *pClusterCurr = NULL;

	/* buffer empty? */
	if (NULL == buffer->pLast)
	{
		pClusterCurr = _buffer_alloc_new_cluster(buffer);
		if (NULL == pClusterCurr)
		{
			ret = AMC_BUFFER_MEMPOOL_ERROR;
			goto ERROR;
		}
	}
	else
	{
		pClusterCurr = buffer->pLast;
	}

	/* copy into buffer */
	while (remainLen > 0)
	{
		size_t writtenLen = 0;
		writtenLen = _buffer_cluster_append_data(pClusterCurr, pData, remainLen);

		remainLen -= writtenLen;
		pData     += writtenLen;

		if (remainLen > 0)
		{
			pClusterCurr = _buffer_alloc_new_cluster(buffer);
			if (NULL == pClusterCurr)
			{
				ret = AMC_BUFFER_MEMPOOL_ERROR;
				goto ERROR;
			}
		}
	}

	/****/
	/* normal ends */
	return ret;

	/****/
	/* error return */
ERROR:
	return ret;
}


AMCMutableBufferError_t _buffer_delete (struct AMCMutableBuffer *buffer, size_t offset, const size_t len, size_t *pDeletedLenOut)
{
	AMCMutableBufferError_t ret = 0;
	BOOL shouldFreeCluster = FALSE;
	BOOL isDeleteToEnd = FALSE;
	struct _buffer_header *cluster = buffer->pStart;
	struct _buffer_header *prevCluster = NULL;
	ssize_t remainLen;

	_DEBUG("Request delete data from %d, len %d", offset, len);

	/****/
	/* prepare data length to read */
	remainLen = _buffer_get_len(buffer);			// This could be optimized, we just to read len once
	if (remainLen < offset) {
		return AMC_BUFFER_OFFSET_EXCEED_LEN;
	}
	else {
		remainLen -= offset;
	}
	
	if (remainLen >= len) {
		isDeleteToEnd = TRUE;
		remainLen = len;
	}

	/****/
	/* search for first buffer cluster */			// This could be optimized, we just to read len once as told above
	while (cluster
		&& (offset > cluster->dataLen))
	{
		offset -= cluster->dataLen;
		
		prevCluster = cluster;
		cluster     = cluster->pNext;
	}

	/****/
	/* delete in first cluster */
	if (cluster)
	{
		size_t currDeleteLen = _buffer_cluster_delete_data(cluster, offset, remainLen, &shouldFreeCluster);
		remainLen -= currDeleteLen;

		if (shouldFreeCluster)
		{
			struct _buffer_header *next = cluster->pNext;
			
			_buffer_free_cluster(buffer, cluster, prevCluster);
			cluster            = next;
		}
		else
		{
			prevCluster = cluster;
			cluster     = prevCluster->pNext;
		}
	}

	/****/
	/* delete in other clusters */
	while (remainLen > 0)
	{
		remainLen -= _buffer_cluster_delete_data(cluster, 0, remainLen, &shouldFreeCluster);

		if (shouldFreeCluster)
		{
			struct _buffer_header *next = cluster->pNext;
			
			_buffer_free_cluster(buffer, cluster, prevCluster);
			cluster            = next;
		}
		else
		{
			prevCluster = cluster;
			cluster     = prevCluster->pNext;
		}
	}


	/****/
	/* ENDS */
	if (pDeletedLenOut)
	{
		if (remainLen < 0) {
			remainLen = 0;
		}
		
		if (isDeleteToEnd) {
			*pDeletedLenOut = len;
		}
		else {
			*pDeletedLenOut = len - remainLen;
		}
	}
	
	return ret;
}


AMCMutableBufferError_t _buffer_read (struct AMCMutableBuffer *buffer, void *dataOut, size_t offset, const size_t len, size_t *pLenOut, BOOL shouldRemove)
{
	AMCMutableBufferError_t ret = 0;
	struct _buffer_header *cluster = buffer->pStart;
	struct _buffer_header *prevCluster = NULL;
	ssize_t remainLen = len;
	uint8_t *pData = (uint8_t*)dataOut;

	_DEBUG("Request read data from %d, len %d, need%s deletion after read", offset, len, shouldRemove ? "" : " no");

	/****/
	/* search for first cluster */
	while (cluster
		&& (offset > cluster->dataLen))
	{
		_DEBUG("Skip a cluster");
		
		offset -= cluster->dataLen;
		
		prevCluster = cluster;
		cluster     = cluster->pNext;
	}

	/****/
	/* read from clusters */
	while (cluster
		&& (remainLen > 0))
	{
		/* read cluster data to its end */
		if (remainLen > (cluster->dataLen - offset))
		{
			_DEBUG("remain %d, cluster available %d, read from %d", remainLen, cluster->dataLen, offset);
		
			size_t thisReadLen = (cluster->dataLen - offset);
			memcpy(pData, _buffer_cluster_get_data(cluster) + offset, thisReadLen);

			pData     += thisReadLen;
			remainLen -= thisReadLen;

			/* should we remove data? */
			if (shouldRemove)
			{
				/* remove the whole cluster */
				if (0 == offset)
				{
					_DEBUG("cluster len %d -> 0", cluster->dataLen);
					
					struct _buffer_header *nextCluster = cluster->pNext;

					_buffer_free_cluster(buffer, cluster, prevCluster);

					cluster = nextCluster;
					offset  = 0;
				}
				/* remove the tail of cluster */
				else
				{
					_DEBUG("cluster len %d -> %d", cluster->dataLen, cluster->dataLen - thisReadLen);
					cluster->dataLen -= thisReadLen;
					offset            = 0;

					prevCluster = cluster;
					cluster     = cluster->pNext;
				}
			}
			else
			{
				offset      = 0;
				prevCluster = cluster;
				cluster     = cluster->pNext;
			}
		}
		/* read part of cluster */
		else
		{
			_DEBUG("remain %d, cluster available %d, read from %d", remainLen, cluster->dataLen, offset);
		
			memcpy(pData, _buffer_cluster_get_data(cluster) + offset, remainLen);

			pData    += remainLen;

			/* should we remove data? */
			if (shouldRemove)
			{
				if (0 == offset)
				{
					if (remainLen  < cluster->dataLen)
					{
						_DEBUG("cluster len %d <- %d", cluster->dataLen, cluster->dataLen - remainLen);
					
						MEMMOVE(_buffer_cluster_get_data(cluster),
							_buffer_cluster_get_data(cluster) + remainLen,
							cluster->dataLen - remainLen);

						cluster->dataLen -= remainLen;
						prevCluster = cluster;
						cluster     = cluster->pNext;
						remainLen   = 0;
						offset      = 0;
					}
					else
					{
						_DEBUG("cluster len %d -> 0", cluster->dataLen);
					
						struct _buffer_header *nextCluster = cluster->pNext;

						_buffer_free_cluster(buffer, cluster, prevCluster);

						cluster     = nextCluster->pNext;
						remainLen   = 0;
						offset      = 0;
					}
				}
				else
				{
					_DEBUG("cluster len %d <- %d, offset %d", cluster->dataLen, cluster->dataLen - remainLen, offset);
				
					MEMMOVE(_buffer_cluster_get_data(cluster) + offset,
							_buffer_cluster_get_data(cluster) + offset + remainLen,
							cluster->dataLen - offset - remainLen);

					prevCluster = cluster;
					cluster     = cluster->pNext;
					remainLen   = 0;
					offset      = 0;
				}
			}
			else
			{
				_DEBUG("Read ends");
			
				offset      = 0;
				prevCluster = cluster;
				cluster     = cluster->pNext;
				remainLen   = 0;
			}
		}
	}

	/****/
	/* ENDS */
	if (pLenOut)
	{
		if (remainLen < 0) {
			remainLen = 0;
		}
		
		*pLenOut = len - remainLen;
	}

	return ret;
}


AMCMutableBufferError_t _buffer_insert (struct AMCMutableBuffer *buffer, size_t offset, const void *data, const size_t len)
{
	AMCMutableBufferError_t ret = 0;
	struct _buffer_header *cluster = buffer->pStart;
	struct _buffer_header *prevCluster = NULL;
	ssize_t remainLen = len;
	const uint8_t *pData = (const uint8_t*)data;

	_DEBUG("Request insert data at %d, len %d", offset, len);

	/****/
	/* locate first cluster */
	while (cluster
		&& (offset > cluster->dataLen))
	{
		_DEBUG("Skip a cluster");
		
		offset -= cluster->dataLen;
		
		prevCluster = cluster;
		cluster     = cluster->pNext;
	}

	/****/
	/* start inserting data */
	while ((remainLen > 0)
		&& cluster)
	{
		size_t availLen = _buffer_cluster_get_capacity(cluster) - cluster->dataLen;
		_DEBUG("Cluster %p availLen %d, remaining %d, offset %d", cluster, availLen, remainLen, offset);

		if (offset > 0)
		{		
			if (remainLen < availLen)
			{
				_DEBUG("insert data in cluster");
			
				MEMMOVE(_buffer_cluster_get_data(cluster) + offset + remainLen, 
					_buffer_cluster_get_data(cluster) + offset, 
					remainLen);
				
				memcpy(_buffer_cluster_get_data(cluster) + offset, pData, remainLen);

				cluster->dataLen += remainLen;
				remainLen = 0;
				offset    = 0;
				pData    += remainLen;
			}
			else
			{
				size_t writeLen = 0;
			
				/* we need to create a new cluster to achieve insert operation */
				struct _buffer_header *nextCluster = _buffer_alloc_new_cluster_and_insert_after(buffer, cluster);

				memcpy(_buffer_cluster_get_data(nextCluster), 
					_buffer_cluster_get_data(cluster) + offset, 
					cluster->dataLen - offset);
				
				nextCluster->dataLen = cluster->dataLen - offset;
				cluster->dataLen     = offset;

				/**/
				availLen = _buffer_cluster_get_capacity(cluster) - cluster->dataLen;
				writeLen = (remainLen < availLen) ? remainLen : availLen;
				
				memcpy(_buffer_cluster_get_data(cluster) + offset,
					pData, 
					writeLen);

				cluster->dataLen += writeLen;
				remainLen -= writeLen;
				offset     = 0;
				pData     += writeLen;

				prevCluster = cluster;
				cluster     = cluster->pNext;
			}
		}
		else
		{
			if (remainLen < availLen)
			{
				MEMMOVE(_buffer_cluster_get_data(cluster) + remainLen, 
					_buffer_cluster_get_data(cluster), 
					remainLen);
				
				memcpy(_buffer_cluster_get_data(cluster), pData, remainLen);

				cluster->dataLen += remainLen;
				remainLen = 0;
				//offset    = 0;		/* no need */
				pData    += remainLen;
			}
			else
			{
				/* allocate a new cluster */
				cluster = _buffer_alloc_new_cluster_and_insert_after(buffer, prevCluster);
				availLen = _buffer_cluster_get_capacity(cluster);

				size_t writeLen = (remainLen < availLen) ? remainLen : availLen;
				memcpy(_buffer_cluster_get_data(cluster), pData, writeLen);

				cluster->dataLen = writeLen;
				remainLen -= writeLen;
				//offset     = 0;		/* no need */
				pData     += writeLen;

				prevCluster = cluster;
				cluster     = cluster->pNext;
			}
		}
	}

	if (remainLen && (NULL == cluster))
	{
		ret = _buffer_append(buffer, pData, remainLen);
	}

	return ret;
}


static AMCMutableBufferError_t _buffer_dump_to_file (struct AMCMutableBuffer *buffer, int fd)
{
	struct _buffer_header *cluster = buffer->pStart;
	int callStat;
	AMCMutableBufferError_t ret = 0;

	while (cluster)
	{
		callStat = write (fd, _buffer_cluster_get_data(cluster), cluster->dataLen);
		if (callStat < 0) {
			ret = AMC_BUFFER_SYSTEM_API_ERROR;
			goto ENDS;
		}
		else {
			cluster = cluster->pNext;
		}
	}

ENDS:
	return ret;
}


static char _char_from_byte(uint8_t byte)
{
	if ((byte >= '!') && (byte <= 0x7F))
	{
		return (char)byte;
	}
	else if ('\n' == byte)
	{
		return '.';
	}
	else if ('\r' == byte)
	{
		return '.';
	}
	else if (' ' == byte)
	{
		return ' ';
	}
	else
	{
		return '.';
	}
}


static AMCMutableBufferError_t _buffer_dump_to_shell (struct AMCMutableBuffer *buffer, int fd)
{
	size_t dataIndex = 0;
	const size_t COLUMN_MAX = 16;		// do not modify this
	size_t lineRemains = COLUMN_MAX;
	size_t clusterReadOffset = 0;
	char line[128] = "";
	char lineString[64] = "";
	char lineChar[24] = "";
	uint8_t lineBytes[16] = {0};
	struct _buffer_header *cluster = buffer->pStart;
	AMCMutableBufferError_t ret = 0;

	/****/
	/* print cluster informations */
	snprintf(line, sizeof(line), "===========================================================================\n");
	write (fd, line, strlen(line));

	int tmp = 0;
	while (cluster)
	{
		snprintf(line, sizeof(line), "[%03d, %p] length: %ld (0x%lX)\n", tmp, cluster, (long)(cluster->dataLen), (long)(cluster->dataLen));
		write (fd, line, strlen(line));
		
		cluster = cluster->pNext;
		tmp ++;
	}

	/****/
	/* print header */
	size_t totalDataLen = _buffer_get_len(buffer);
	cluster = buffer->pStart;
	
	snprintf(line, sizeof(line), "---------------------------------------------------------------------------\n");
	write (fd, line, strlen(line));
	snprintf(line, sizeof(line), "AMCMutableBuffer: 0x%08lx, length %ld(0x%04lx)\n", (unsigned long)(buffer), (long)totalDataLen, (long)totalDataLen);
	write (fd, line, strlen(line));
	snprintf(line, sizeof(line), "----  +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F    01234567 89ABCDEF\n");
	write (fd, line, strlen(line));

	/****/
	/* print actual data */
	while (cluster)
	{
		/* read data */
		if (lineRemains < (cluster->dataLen - clusterReadOffset))
		{
			memcpy(lineBytes + (COLUMN_MAX - lineRemains), 
					_buffer_cluster_get_data(cluster) + clusterReadOffset, 
					lineRemains);
			
			clusterReadOffset += lineRemains;
			lineRemains        = 0;
		}
		else
		{
			memcpy(lineBytes + (COLUMN_MAX - lineRemains), 
					_buffer_cluster_get_data(cluster) + clusterReadOffset, 
					(cluster->dataLen - clusterReadOffset));

			lineRemains -= (cluster->dataLen - clusterReadOffset);

			cluster = cluster->pNext;
			clusterReadOffset = 0;
		}

		/* dump complete 16-bytes data */
		if (0 == lineRemains)
		{
			uint8_t byte;
			size_t tmp;
			size_t lineLen = 0;
			
			memset(lineString, 0, sizeof(lineString));
			memset(lineChar,   0, sizeof(lineChar));

			for (tmp = 0; tmp < COLUMN_MAX; tmp++)
			{
				byte = lineBytes[tmp];
				sprintf(lineString + lineLen, "%02X ", byte & 0xFF);

				lineLen += 3;

				if (tmp < 7)
				{
					lineChar[tmp] = _char_from_byte(byte);
				}
				else if (7 == tmp)
				{
					lineChar[tmp]     = _char_from_byte(byte);
					lineChar[tmp + 1] = ' ';
					sprintf(lineString + lineLen, " ");
					lineLen += 1;
				}
				else
				{
					lineChar[tmp + 1] = _char_from_byte(byte);
				}
			}

			lineLen = snprintf(line, sizeof(line), "%04lX: %s   %s\n", (long)dataIndex, lineString, lineChar);
			dataIndex += COLUMN_MAX;
			lineRemains = COLUMN_MAX;

			write(fd, line, lineLen);
		}

	}

	/****/
	/* dump final line */
	if (lineRemains < COLUMN_MAX)
	{
		uint8_t byte;
		size_t tmp;
		size_t lineLen = 0;

		memset(lineString, 0, sizeof(lineString));
		memset(lineChar,   0, sizeof(lineChar));

		for (tmp = 0; tmp < (COLUMN_MAX - lineRemains); tmp++)
		{
			byte = lineBytes[tmp];
			sprintf(lineString + lineLen, "%02X ", byte & 0xFF);
			lineLen += 3;

			if (tmp < 7)
			{
				lineChar[tmp] = _char_from_byte(byte);
			}
			else if (7 == tmp)
			{
				lineChar[tmp]     = _char_from_byte(byte);
				lineChar[tmp + 1] = ' ';
				sprintf(lineString + lineLen, " ");
				lineLen += 1;
			}
			else
			{
				lineChar[tmp + 1] = _char_from_byte(byte);
			}
		}

		for (/* null */; tmp < COLUMN_MAX; tmp ++)
		{
			sprintf(lineString + lineLen, "   ");
			lineLen += 3;
		
			if (7 == tmp)
			{
				sprintf(lineString + lineLen, " ");
				lineLen += 1;
			}
		}

		lineLen = snprintf(line, sizeof(line), "%04lX: %s   %s\n", (long)dataIndex, lineString, lineChar);
		dataIndex += COLUMN_MAX;
		lineRemains = COLUMN_MAX;

		write(fd, line, lineLen);
	}

	return ret;
}


static AMCMutableBufferError_t _buffer_defragment (struct AMCMutableBuffer *buffer)
{
	AMCMutableBufferError_t ret = 0;
	struct _buffer_header *cluster = NULL;
	struct _buffer_header *clusterNext = NULL;
	size_t framentLen = 0;

	cluster = buffer->pStart;
	if (cluster->pNext) {
		clusterNext = cluster->pNext;
	}

	/* examine and delete clusters if necessary */
	while (cluster && clusterNext)
	{
		framentLen = _buffer_cluster_get_capacity(cluster) - (cluster->dataLen);
	
		/* determine whether current cluster is fully used */
		if (framentLen <= 0)
		{
			cluster     = cluster->pNext;
			clusterNext = clusterNext->pNext;
		}
		/* let's defragment! */
		else
		{
			if (framentLen >= (clusterNext->dataLen))
			{
				/* combine next cluster here */
				memcpy(_buffer_cluster_get_data(cluster) + (cluster->dataLen),
						_buffer_cluster_get_data(clusterNext), 
						clusterNext->dataLen);
				cluster->dataLen += clusterNext->dataLen;

				_buffer_free_cluster(buffer, clusterNext, cluster);
				clusterNext = cluster->pNext;
			}
			else
			{
				/* copy part data of next cluster */
				memcpy(_buffer_cluster_get_data(cluster) + (cluster->dataLen),
						_buffer_cluster_get_data(clusterNext),
						framentLen);
				cluster->dataLen = _buffer_cluster_get_capacity(cluster);

				MEMMOVE(_buffer_cluster_get_data(clusterNext),
						_buffer_cluster_get_data(clusterNext) + framentLen,
						framentLen);
				clusterNext->dataLen -= framentLen;

				cluster     = cluster->pNext;
				clusterNext = clusterNext->pNext;
			}
		}
	}

	return ret;
}


AMCMutableBufferError_t _buffer_check_each_cluster (struct AMCMutableBuffer *buffer, 
											void (*each_segment_cb)(void *, size_t, size_t, void *, BOOL),
											void *byteBuff, size_t byteBuffLen, void *arg)
{
	struct _buffer_header *cluster = buffer->pStart;
	uint8_t *pOut = (uint8_t*)byteBuff;
	size_t lenInOut = 0;
	size_t remainsInOut = byteBuffLen;
	size_t clusterReadOffset = 0;
	size_t offset = 0;

	while(cluster)
	{
		/* read data */
		if (remainsInOut < (cluster->dataLen - clusterReadOffset))
		{
			memcpy(pOut + lenInOut, 
					_buffer_cluster_get_data(cluster) + clusterReadOffset, 
					remainsInOut);

			clusterReadOffset += remainsInOut;
			lenInOut          += remainsInOut;
			remainsInOut       = 0;
		}
		else
		{
			memcpy(pOut + lenInOut, 
					_buffer_cluster_get_data(cluster) + clusterReadOffset, 
					(cluster->dataLen - clusterReadOffset));

			remainsInOut -= (cluster->dataLen - clusterReadOffset);
			lenInOut     += (cluster->dataLen - clusterReadOffset);
			clusterReadOffset = 0;

			cluster = cluster->pNext;
		}
		

		/* invoke callback */
		if (0 == remainsInOut)
		{
			(*each_segment_cb)(pOut, offset, lenInOut, arg, (cluster) ? FALSE : TRUE);

			offset      += lenInOut;
			remainsInOut = byteBuffLen;
			lenInOut     = 0;
		}
	}

	if (lenInOut)
	{
		(*each_segment_cb)(pOut, offset, lenInOut, arg, TRUE);

		offset      += lenInOut;
		remainsInOut = byteBuffLen;
		lenInOut     = 0;
	}

	return 0;
}


#endif


/********************/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

struct AMCMutableBuffer *AMCMutableBuffer_New (size_t clusterSize, size_t initSize, AMCMutableBufferError_t *pErrOut, BOOL threadSafe)
{
	/****/
	/* check parameters */
	if (0 == clusterSize) {
		clusterSize = _CFG_DEFAULT_CLUSTER_SIZE;
	}

	if (0 == initSize) {
		initSize = _CFG_DEFAULT_INIT_SIZE;
	}

	if (0 != (clusterSize & 0x3))	/* ensure address alignment */
	{
		clusterSize = (clusterSize + 4) & ~0x3;
	}

	/****/
	/* prepare spaces */
	AMCMutableBufferError_t error = 0;
	struct AMCMutableBuffer *ret = NULL;
	size_t initClusterCount = (initSize / clusterSize) + 1;

	ret = malloc(sizeof(*ret));
	if (NULL == ret) {
		error = AMC_BUFFER_SYSTEM_API_ERROR;
		goto ERROR;
	}
	else {
		memset(ret, 0, sizeof(*ret));
		ret->classMagic = _CFG_AMC_MUTABLE_BUFFER_MAGIC_NUM;
	}

	ret->useLock = threadSafe;
	ret->pMemPool = AMCMemPool_Create (
								clusterSize + sizeof(struct _buffer_header),
								initClusterCount, 
								initClusterCount, 
								FALSE);
	if (NULL == ret->pMemPool) {
		error = AMC_BUFFER_MEMPOOL_ERROR;
		goto ERROR;
	}


	if (ret->useLock) {
		int callStat = pthread_mutex_init(&(ret->lock), NULL);
		if (0 != callStat) {
			error = AMC_BUFFER_MUTEX_ERROR;
			goto ERROR;
		}
	}

	/****/
	/* set init tools */
	ret->pStart = NULL;
	ret->pLast  = NULL;
	ret->clusterSize = clusterSize;
	ret->clusterCount   = 0;

	/****/
	/* return */
	if (pErrOut) {
		*pErrOut = error;
	}

	_DEBUG("A new buffer created, cluster size %d, init size %d", clusterSize, initSize);
	return ret;


	/****/
	/* error return */
ERROR:
	if (pErrOut) {
		*pErrOut = error;
	}
	
	if (ret)
	{
		if (ret->pMemPool) {
			AMCMemPool_Destory(ret->pMemPool);
			ret->pMemPool = NULL;
		}

		free(ret);
		ret = NULL;
	}
	
	return NULL;
}


AMCMutableBufferError_t AMCMutableBuffer_Destroy (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer)) {
		return AMC_BUFFER_PARA_ERROR;
	}

	/* destroy */
	if (buffer->useLock) {
		buffer->useLock = FALSE;
		pthread_mutex_destroy(&(buffer->lock));
	}
	
	if (buffer->pMemPool) {
		AMCMemPool_Destory(buffer->pMemPool);
		buffer->pMemPool = NULL;
	}

	free(buffer);
	buffer = NULL;

	return 0;
}


const char *AMCMutableBuffer_StrError (AMCMutableBufferError_t error)
{
	if ((error < 0) || (error >= AMCMutableBufferError_Types))
	{
		return "Illegal error number";
	}
	else if (AMC_BUFFER_SYSTEM_API_ERROR == error)
	{
		return strerror(errno);
	}
	else
	{
		return _error_msg[(size_t)error];
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Dump (struct AMCMutableBuffer *buffer, int fd)
{
	if ((FALSE == _buffer_is_correct_type(buffer))
		|| (fd < 0))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
	
		_buffer_LOCK(buffer);

		if ((1 == fd)
			|| (2 == fd))
		{
			ret = _buffer_dump_to_shell(buffer, fd);
		}
		else
		{
			ret = _buffer_dump_to_file(buffer, fd);
		}

		_buffer_UNLOCK(buffer);
		return ret;
	}
}


size_t AMCMutableBuffer_FramentSize (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return 0;
	}
	else
	{
		size_t ret = 0;
		struct _buffer_header *cluster = buffer->pStart;
		
		_buffer_LOCK(buffer);

		while (cluster
			&& cluster->pNext)
		{
			ret += _buffer_cluster_get_capacity(cluster) - (cluster->dataLen);
			cluster = cluster->pNext;
		}

		_buffer_UNLOCK(buffer);
		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Defragment (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_defragment(buffer);
		_buffer_UNLOCK(buffer);
		
		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_CheckAllData (struct AMCMutableBuffer *buffer, 
											void (*each_segment_cb)(void *, size_t, size_t, void *arg, BOOL),
											void *byteBuff, size_t byteBuffLen, void *arg)
{
	if ((FALSE == _buffer_is_correct_type(buffer))
		|| (NULL == each_segment_cb)
		|| (NULL == byteBuff))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_check_each_cluster(buffer, each_segment_cb, byteBuff, byteBuffLen, arg);
		_buffer_UNLOCK(buffer);
		
		return ret;
	}
}


size_t AMCMutableBuffer_GetLen(struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return 0;
	}
	else
	{
		size_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_get_len(buffer);
		_buffer_UNLOCK(buffer);

		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Append (struct AMCMutableBuffer *buffer, const void *data, size_t dataLen)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_append(buffer, data, dataLen);
		_buffer_UNLOCK(buffer);

		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Delete (struct AMCMutableBuffer *buffer, size_t offset, size_t len, size_t *pDeleteLenOut)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_delete(buffer, offset, len, pDeleteLenOut);
		_buffer_UNLOCK(buffer);

		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_DeleteAll (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		struct _buffer_header *cluster = buffer->pStart;
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);

		buffer->pLast  = NULL;
		buffer->pStart = NULL;
		buffer->clusterCount = 0;

		while(cluster)
		{
			AMCMemPool_Free(cluster);
			cluster = cluster->pNext;
		}		

		/****/
		/* ENDS */
		_buffer_UNLOCK(buffer);
		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Read (struct AMCMutableBuffer *buffer, 
						void *dataOut, size_t offset, size_t len, size_t *pLenOut, BOOL shouldRemove)
{
	if ((FALSE == _buffer_is_correct_type(buffer))
		|| (NULL == dataOut))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_read(buffer, dataOut, offset, len, pLenOut, shouldRemove);
		_buffer_UNLOCK(buffer);

		return ret;
	}
}


AMCMutableBufferError_t AMCMutableBuffer_Insert (struct AMCMutableBuffer *buffer, size_t offset, const void *data, size_t dataLen)
{
	if ((FALSE == _buffer_is_correct_type(buffer))
		|| (NULL == data))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		AMCMutableBufferError_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = _buffer_insert(buffer, offset, data, dataLen);
		_buffer_UNLOCK(buffer);

		return ret;
	}
}


size_t AMCMutableBuffer_GetUnitSize (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		return buffer->clusterSize;
	}
}


size_t AMCMutableBuffer_GetUnitCount (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		return buffer->clusterCount;
	}
}


size_t AMCMutableBuffer_GetMemoryUsage (struct AMCMutableBuffer *buffer)
{
	if (FALSE == _buffer_is_correct_type(buffer))
	{
		return AMC_BUFFER_PARA_ERROR;
	}
	else
	{
		size_t ret = 0;
		
		_buffer_LOCK(buffer);
		ret = sizeof(*buffer) + AMCMemPool_MemoryUsage(buffer->pMemPool);		
		_buffer_UNLOCK(buffer);

		return ret;
	}

}


#endif


/********************/
/* EOF */

