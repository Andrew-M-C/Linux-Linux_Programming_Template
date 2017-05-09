/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCMemPool.c
	Description: 	
			This file provide implementations of AMCMemPool.h.
			
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

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>


#define _DEBUG(fmt, args...)		printf("##"__FILE__", %d: "fmt"\n", __LINE__, ##args);
//#define _DEBUG(fmt, args...)


/**********/
#define __PRIVATE_MACROS
#ifdef __PRIVATE_MACROS

#define _LOCK_IF_COND(pLock, condition)		if (condition) pthread_mutex_lock(pLock)
#define _UNLOCK_IF_COND(pLock, condition)	if (condition) pthread_mutex_unlock(pLock)

#define _LOCK_POOL(pPool)		_memPool_Lock(pPool)
#define _UNLOCK_POOL(pPool)		_memPool_Unlock(pPool)

#define _SIZE_T_CVT(v)		((size_t)(v))
#define _ULONG_CVT(v)			((unsigned long)(v))

#endif


/********************/
#define __STRUCTURE_DEFINITIONS
#ifdef __STRUCTURE_DEFINITIONS
typedef struct AMCMemBlock {
	struct AMCMemBlock   *pNext;
	unsigned long        totalUnitCount;
	unsigned long        freeUnitCount;
	unsigned long        validUnitIdx;
	// ......			 // pUnits
} AMCMemBlock_st;


typedef struct AMCMemPool {
	struct AMCMemBlock   *pBlocks;
	BOOL                 useLock;
	pthread_mutex_t      lock;
	unsigned long        unitSize;
	unsigned long        initUnitCount;
	unsigned long        growUnitCount;
} AMCMemPool_st;


typedef struct AMCMemUnit {
	struct AMCMemPool    *pOwnerPool;
	unsigned long        nextUnitIdx;
	// ......            // pData
} AMCMemUnit_st;


#endif


/********************/
#define __INTERNAL_FUNCTION_DECLARATIONS
#ifdef __INTERNAL_FUNCTION_DECLARATIONS

static BOOL _memUnit_IsInBlock(struct AMCMemUnit *unit, struct AMCMemBlock *block);
static struct AMCMemBlock *_memBlock_Attach(struct AMCMemPool * pool);
static struct AMCMemBlock *_memBlock_Attach_WithLock(struct AMCMemPool *pool);
static inline BOOL _memBlock_IsEmpty(struct AMCMemBlock *block);
static inline BOOL _memBlock_IsFull(struct AMCMemBlock *block);
static inline BOOL _memBlock_IsValid(struct AMCMemBlock *block);
static struct AMCMemUnit *_memBlock_Alloc(struct AMCMemBlock *block, unsigned long unitSize);
static void _memPool_Lock(struct AMCMemPool *pool);
static void _memPool_Unlock(struct AMCMemPool *pool);

#endif



/********************/
#define __UNIT_OPERATIONS
#ifdef __UNIT_OPERATIONS

static int _memUnit_Free(struct AMCMemUnit *unit)
{
	struct AMCMemBlock *ownerBlock = NULL;
	struct AMCMemPool *ownerPool = NULL;
	struct AMCMemBlock *block;
	unsigned long unitStructSize, unitIdx;

	if ((NULL == unit) || (NULL == unit->pOwnerPool))
	{
		return -1;
	}

	/* determine which block the unit belongs */
	ownerPool = unit->pOwnerPool;
	unitStructSize = sizeof(AMCMemUnit_st) + ownerPool->unitSize;
	
	for (block = ownerPool->pBlocks;
		block && (NULL == ownerBlock);
		block = block->pNext)
	{
		if (_memUnit_IsInBlock(unit, block))
		{
			/* unit belongs to current block */
			ownerBlock = block;
			unitIdx = (_ULONG_CVT(unit) - _ULONG_CVT(ownerBlock)) / unitStructSize;
		}
		else
		{}	// Continue
	}

	if (NULL == ownerBlock)
	{
		return -1;
	}


	/* "free" the unit */
	_DEBUG("[Block 0x%08lx] Free unit %ld", (unsigned long)block, unitIdx);
	unit->nextUnitIdx = ownerBlock->validUnitIdx;
	ownerBlock->validUnitIdx = unitIdx;

	return 0;
}


static BOOL _memUnit_IsInBlock(struct AMCMemUnit *unit, struct AMCMemBlock *block)
{
	unsigned long unitStructSize, blockStructSize;

	unitStructSize = sizeof(AMCMemUnit_st) + unit->pOwnerPool->unitSize;
	blockStructSize = sizeof(AMCMemBlock_st) + (block->totalUnitCount) * unitStructSize;

	if ((_ULONG_CVT(unit) > _ULONG_CVT(block)) &&
			(_ULONG_CVT(unit) < _ULONG_CVT(block) + blockStructSize))
	{
		/* unit belongs to current block */
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static int _memUnit_Free_WithLock(struct AMCMemUnit *unit)
{
	int ret = 0;
	if ((NULL == unit) || (NULL == unit->pOwnerPool))
	{
		return -1;
	}

	_LOCK_POOL(unit->pOwnerPool);
	ret = _memUnit_Free(unit);
	_UNLOCK_POOL(unit->pOwnerPool);

	_DEBUG("[Pool 0x%08lx] Free unit 0x%08lx %s", _ULONG_CVT(unit->pOwnerPool), _ULONG_CVT(unit), ret ? "OK" : "faild");
	return ret;
}



#endif


/********************/
#define __POOL_OPERATIONS
#ifdef __POOL_OPERATIONS

static void _memPool_Lock(struct AMCMemPool *pool)
{
	_LOCK_IF_COND(&(pool->lock), pool->useLock);
}


static void _memPool_Unlock(struct AMCMemPool *pool)
{
	_UNLOCK_IF_COND(&(pool->lock), pool->useLock);
}


static struct AMCMemPool *_memPool_Create_WithLock(unsigned long unitSize, unsigned long initUnitCount, unsigned long growUnitCount, BOOL useLock)
{
	struct AMCMemPool *pool = malloc(sizeof(AMCMemPool_st));
	int callStat;

	if (NULL == pool)
	{
		return NULL;
	}
	else if ((0 == unitSize) || 
			(0 == initUnitCount) ||
			(0 == growUnitCount))
	{
		free(pool);
		return NULL;
	}


	pool->pBlocks = NULL;
	pool->unitSize = unitSize;
	pool->initUnitCount = initUnitCount;
	pool->growUnitCount = growUnitCount;
	pool->useLock = useLock;

	if (useLock)
	{
		callStat = pthread_mutex_init(&(pool->lock), NULL);
		if (0 != callStat)
		{
			free(pool);
			_DEBUG("[Pool 0x%08lx] Cannot create mutex", _ULONG_CVT(pool));
			return NULL;
		}
	}
	

	if (0 != _memBlock_Attach_WithLock(pool))
	{
		free(pool);
		_DEBUG("[Pool 0x%08lx] Cannot attach block", _ULONG_CVT(pool));
		return NULL;
	}

	_DEBUG("[Pook 0x%08lx] Created", _ULONG_CVT(pool));
	return pool;
}


static int _memPool_Destory(struct AMCMemPool *pool)
{
	struct AMCMemBlock *pBlock, *pNext;

	if (NULL == pool)
	{
		return 0;
	}

	pBlock = pool->pBlocks;
	
	while (pBlock)
	{
		pNext = pBlock->pNext;
		free(pBlock);
		pBlock = pNext;
	}

	_DEBUG("[Pool 0x%08lx] Destroyed", _ULONG_CVT(pool));
	free(pool);
	return 0;
}


static BOOL _memPool_IsEmpty_WithLock(struct AMCMemPool *pool)
{
	BOOL ret;

	if (NULL == pool)
	{
		return FALSE;
	}

	_LOCK_POOL(pool);			/* --- LOCK --- */
	if (pool->pBlocks->pNext)
	{
		ret = FALSE;
	}
	else if (pool->pBlocks->freeUnitCount == pool->pBlocks->totalUnitCount)
	{
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}
	_UNLOCK_POOL(pool);			/* -- UNLOCK -- */

	return ret;
}


static struct AMCMemUnit *_memPool_Alloc_WithLock(struct AMCMemPool *pool)
{
	struct AMCMemUnit *retUnit = NULL;
	struct AMCMemBlock *block = NULL;

	/**/
	_LOCK_POOL(pool);			/* --- LOCK --- */

	/* find a valid block */
	for (block = pool->pBlocks; 
		block && (NULL == retUnit);
		block = block->pNext)
	{
		if (_memBlock_IsValid(block))
		{
			retUnit = _memBlock_Alloc(block, pool->unitSize);
		}
		else
		{}	// continue
	}

	/* no valid block? attach a new one */
	if (NULL == retUnit)
	{
		block = _memBlock_Attach(pool);
		retUnit = _memBlock_Alloc(block, pool->unitSize);
	}

	/**/
	_UNLOCK_POOL(pool);			/* -- UNLOCK -- */
	return retUnit;
}


#endif



/********************/
#define __BLOCK_OPERATIONS
#ifdef __BLOCK_OPERATIONS


static struct AMCMemBlock *_memBlock_Attach(struct AMCMemPool * pool)
{
	struct AMCMemBlock *block = NULL;
	struct AMCMemUnit *unit = NULL;
	struct AMCMemBlock *lastBlock;
	unsigned long blockStructSize, unitStructSize, unitCount, tmp;

	if (NULL == pool)
	{
		return NULL;
	}

	/* calculate memory requirement */
	unitStructSize = pool->unitSize + sizeof(AMCMemUnit_st);
	unitCount = (pool->pBlocks) ? pool->growUnitCount : pool->initUnitCount;
	blockStructSize = unitCount * unitStructSize + sizeof(AMCMemBlock_st);

	/* malloc */
	block = malloc(blockStructSize);
	if (NULL == block)
	{
		return NULL;
	}
	
	/* fill in memory pool information */
	for (tmp = 0; tmp < unitCount; tmp++)
	{
		unit = (AMCMemUnit_st *)(_ULONG_CVT(block) + sizeof(AMCMemBlock_st) + tmp * unitStructSize);
		unit->nextUnitIdx = tmp + 1;
		unit->pOwnerPool = pool;
	}
	block->pNext = NULL;
	block->totalUnitCount = unitCount;
	block->freeUnitCount = unitCount;
	block->validUnitIdx = 0;


	/* find and attach memory block */	
	if (pool->pBlocks)
	{
		do {
			lastBlock = lastBlock->pNext;
		} while(lastBlock->pNext);

		lastBlock = block;
	}
	else
	{
		pool->pBlocks = block;
	}

	/* OK */
	_DEBUG("[Pool 0x%08lx] Attach block 0x%08lx", _ULONG_CVT(pool), _ULONG_CVT(block));
	return block;
}

static struct AMCMemBlock *_memBlock_Attach_WithLock(struct AMCMemPool *pool)
{
	struct AMCMemBlock *block = NULL;
	struct AMCMemUnit *unit = NULL;
	struct AMCMemBlock *lastBlock;
	unsigned long blockStructSize, unitStructSize, unitCount, tmp;

	if (NULL == pool)
	{
		return NULL;
	}

	/* calculate memory requirement */
	unitStructSize = pool->unitSize + sizeof(AMCMemUnit_st);
	unitCount = (pool->pBlocks) ? pool->growUnitCount : pool->initUnitCount;
	blockStructSize = unitCount * unitStructSize + sizeof(AMCMemBlock_st);

	/* malloc */
	block = malloc(blockStructSize);
	if (NULL == block)
	{
		return NULL;
	}
	
	/* fill in memory pool information */
	for (tmp = 0; tmp < unitCount; tmp++)
	{
		unit = (AMCMemUnit_st *)(_ULONG_CVT(block) + sizeof(AMCMemBlock_st) + tmp * unitStructSize);
		unit->nextUnitIdx = tmp + 1;
		unit->pOwnerPool = pool;
	}
	block->pNext = NULL;
	block->totalUnitCount = unitCount;
	block->freeUnitCount = unitCount;
	block->validUnitIdx = 0;


	/* find and attach memory block */	
	if (pool->pBlocks)
	{
		_LOCK_POOL(pool);		/* ---LOCK--- */
		
		do {
			lastBlock = lastBlock->pNext;
		} while(lastBlock->pNext);

		lastBlock = block;
		_UNLOCK_POOL(pool);		/* --UNLOCK-- */
	}
	else
	{
		pool->pBlocks = block;
	}

	/* OK */
	_DEBUG("[Pool 0x%08lx] Attach block 0x%08lx", _ULONG_CVT(pool), _ULONG_CVT(block));
	return block;
}


static struct AMCMemUnit *_memBlock_Alloc(struct AMCMemBlock *block, unsigned long unitSize)
{
	unsigned long unitStructSize = unitSize + sizeof(AMCMemUnit_st);
	unsigned long freeIdx = block->validUnitIdx;
	struct AMCMemUnit *retUnit = (AMCMemUnit_st *)(_ULONG_CVT(block) + sizeof(AMCMemBlock_st) + freeIdx * unitStructSize);

	block->validUnitIdx = retUnit->nextUnitIdx;
	_DEBUG("[Block 0x%08lx] alloc unit 0x%08lx, index %ld", _ULONG_CVT(block), _ULONG_CVT(retUnit), freeIdx);
	return retUnit;
}


static inline BOOL _memBlock_IsEmpty(struct AMCMemBlock *block)
{
	return (block->totalUnitCount == block->freeUnitCount);
}


static inline BOOL _memBlock_IsFull(struct AMCMemBlock *block)
{
	return (0 == block->freeUnitCount);
}


static inline BOOL _memBlock_IsValid(struct AMCMemBlock *block)
{
	return (0 != block->freeUnitCount);
}


#endif



/********************/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

struct AMCMemPool *AMCMemPool_Create(unsigned long unitSize, unsigned long initUnitCount, unsigned long growUnitCount, BOOL useLock)
{
	return _memPool_Create_WithLock(unitSize, initUnitCount, growUnitCount, useLock);
}


int AMCMemPool_Destory(struct AMCMemPool *pool)
{
	return _memPool_Destory(pool);
}


BOOL AMCMemPool_IsAllFree(struct AMCMemPool *pool)
{
	return _memPool_IsEmpty_WithLock(pool);
}


struct AMCMemUnit *AMCMemPool_Alloc(struct AMCMemPool *pool)
{
	if (pool)
	{
		return _memPool_Alloc_WithLock(pool);
	}
	else
	{
		return NULL;
	}
}


int AMCMemPool_Free(struct AMCMemUnit *unit)
{
	return _memUnit_Free_WithLock(unit);
}


inline void *AMCMemPool_DataPtr(struct AMCMemUnit *unit)
{
	return (void*)(_ULONG_CVT(unit) + sizeof(*unit));
}


inline unsigned long AMCMemPool_UnitSize(struct AMCMemUnit *unit)
{
	return unit->pOwnerPool->unitSize;
}


inline unsigned long AMCMemPool_UnitSizeForPool(struct AMCMemPool *pool)
{	
	return pool->unitSize;
}



#endif

