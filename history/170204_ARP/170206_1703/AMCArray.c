/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCArray.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file implements mutable array support.
			
	History:
		2017-02-04: File created as AMCArp.c

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/********/
#define __HEADERS
#ifdef __HEADERS

#include "AMCArray.h"
#include "AMCMemPool.h"

#define DEBUG
#define CFG_LIB_MEM
#define CFG_LIB_THREAD
#define CFG_LIB_MATH
#define CFG_LIB_FILE
#include "AMCCommonLib.h"

#endif


/********/
#define __DATA_TYPES_AND_VARIABLES
#ifdef __DATA_TYPES_AND_VARIABLES


struct AMCArrayItem {
	struct AMCArrayItem  *prev;
	struct AMCArrayItem  *next;
	void                 *value;
};


struct AMCArray {
	pthread_mutex_t       lock;
	size_t                count;
	size_t                last_access_index;
	struct AMCArrayItem  *last_access_item;
	struct AMCArrayItem  *first;
	struct AMCArrayItem  *last;
};


#define _DB(fmt, args...)		AMCPrintf("<%s, %ld> "fmt, __FILE__, __LINE__, ##args)
#define _LOCK_ARRAY(array)			do{/*_DB("-- LOCK --");*/ pthread_mutex_lock(&((array)->lock));}while(0)
#define _UNLOCK_ARRAY(array)		do{/*_DB("--UNLOCK--");*/ pthread_mutex_unlock(&((array)->lock));}while(0)

static struct AMCMemPool *g_item_mem_pool = NULL;
static size_t g_item_init_size = CFG_AMC_ARRAY_DEFAULT_INITIAL_ITEM_COUNT;
static size_t g_item_inc_size = CFG_AMC_ARRAY_DEFAULT_INCREASE_ITEM_COUNT;

static struct AMCMemPool *g_array_mem_pool = NULL;
static size_t g_array_init_size = CFG_AMC_ARRAY_DEFAULT_INITIAL_ARRAY_COUNT;
static size_t g_array_inc_size = CFG_AMC_ARRAY_DEFAULT_INCREASE_ARRAY_COUNT;

#endif


/********/
#define __INIT_AND_DEALLOC_FUNCTIONS
#ifdef __INIT_AND_DEALLOC_FUNCTIONS

/* --------------------_make_error----------------------- */
AMCArrayErrno_st _make_error(int16_t error)
{
	AMCArrayErrno_st ret = {0, 0};

	if (AMC_ARRAY_SYSTEM_ERROR == error) {
		ret.sys_errno = (int16_t)errno;
	}

	ret.array_error = error;

	return ret;
}


/* --------------------_check_and_init_amc_array----------------------- */
static int _check_and_init_amc_array()
{
	if (NULL == g_item_mem_pool) {
		g_item_mem_pool = AMCMemPool_Create(sizeof(struct AMCArrayItem), g_item_init_size, g_item_inc_size, TRUE);
	}

	if (NULL == g_array_mem_pool) {
		g_array_mem_pool = AMCMemPool_Create(sizeof(struct AMCArray), g_array_init_size, g_array_inc_size, TRUE);
	}

	if (g_item_mem_pool && g_array_mem_pool) {
		return 0;
	}
	else {
		return -1;
	}
}


/* --------------------_array_free----------------------- */
AMCArrayErrno_st _array_free(AMCArray_st *array, BOOL freeObjects)
{
	struct AMCArrayItem *pItem = array->first;
	struct AMCArrayItem *pItemNext = NULL;

	/* clean array items */
	if (freeObjects)
	{
		while(pItem)
		{
			pItemNext = pItem->next;
			
			free(pItem->value);
			pItem->value = NULL;

			AMCMemPool_Free(pItem);
			pItem = pItemNext;
		}
	}
	else
	{
		while(pItem)
		{
			pItemNext = pItem->next;
			
			pItem->value = NULL;
			
			AMCMemPool_Free(pItem);
			pItem = pItemNext;
		}
	}

	/* remove cached data */
	array->first = NULL;
	array->last  = NULL;
	array->count = 0;
	array->last_access_index = 0;
	array->last_access_item  = NULL;

	/* ends */
	return _make_error(0);
}


#endif


/********/
#define __ARRAY_MODIFICATIONS
#ifdef __ARRAY_MODIFICATIONS

/* --------------------_object_at_index----------------------- */
enum {
	_READ_FROM_START = 0,
	_READ_FROM_CACHE,
	_READ_FROM_END,	
};
struct AMCArrayItem *_object_at_index(AMCArray_st *array, size_t index)
{
	if (index >= array->count) {
		return NULL;
	}

	size_t tmp;
	struct AMCArrayItem *ret = NULL;

	int readMethod = _READ_FROM_START;

	/* determine the way for accccess */
	{
		size_t thisSearchCount = index;
		size_t predictSearchCount = index;

		thisSearchCount = abs(index - array->last_access_index);
		if ((thisSearchCount < predictSearchCount)
			&& array->last_access_item)
		{
			predictSearchCount = thisSearchCount;
			readMethod = _READ_FROM_CACHE;
		}

		thisSearchCount = abs(array->count - index - 1);
		if (thisSearchCount < predictSearchCount)
		{
			predictSearchCount = thisSearchCount;
			readMethod = _READ_FROM_END;
		}

		_DB("%p: Request object at %d, cached %d[%p], length %d, searh steps %d", 
			array, index, array->last_access_index, array->last_access_item, array->count, predictSearchCount);
	}

	/* search according different ways */
	switch(readMethod)
	{
	default:
	case _READ_FROM_START:
		{
			for (tmp = 0, ret = array->first;
				tmp < index;
				tmp++, ret = ret->next)
			{}

			array->last_access_index = index;
			array->last_access_item  = ret;

			_DB("%p: Search from start", array);
		}
		break;
		
	case _READ_FROM_CACHE:
		if (index > array->last_access_index)
		{
			for (tmp = array->last_access_index, ret = array->last_access_item;
				tmp < index;
				tmp++, ret = ret->next)
			{}

			array->last_access_index = index;
			array->last_access_item  = ret;

			_DB("%p: Search from cache", array);
		}
		else // (index <= array->last_access_index)
		{
			for (tmp = array->last_access_index, ret = array->last_access_item;
				tmp > index; 
				tmp--, ret = ret->prev)
			{}

			array->last_access_index = index;
			array->last_access_item  = ret;

			_DB("%p: Search from cache", array);
		}
		break;

	case _READ_FROM_END:
		{
			for (tmp = array->count - 1, ret = array->last;
				tmp > index;
				tmp--, ret = ret->prev)
			{}

			array->last_access_index = index;
			array->last_access_item  = ret;

			_DB("%p: Search from end", array);
		}
		break;
	}

	/* return */
	return ret;
}


/* --------------------_append_object----------------------- */
AMCArrayErrno_st _append_object(AMCArray_st *array, void *object)
{
	/* allocate from mempool */
	struct AMCArrayItem *newItem = AMCMemPool_Alloc(g_item_mem_pool);
	if (NULL == newItem) {
		return _make_error(AMC_ARRAY_MEMPOOL_ERROR);
	}

	_DB("%p: Append object %p[%p], cached %d[%p], length %d, last [%p]", 
		array, newItem, object, array->last_access_index, array->last_access_item, array->count, array->last);

	/* set item value */
	newItem->next = NULL;
	newItem->prev = NULL;
	newItem->value = object;

	/* append to array */
	if (NULL == array->last) {
		array->first = newItem;
		array->last  = newItem;
		array->count = 1;
		array->last_access_index = 0;
		array->last_access_item  = newItem;
	}
	else {
		array->last->next = newItem;
		newItem->prev = array->last;

		array->last = newItem;
		array->count ++;
	}

	/* ends */
	return _make_error(0);
}


/* --------------------_append_object----------------------- */
AMCArrayErrno_st _insert_object(AMCArray_st *array, void *object, size_t index)
{
	_DB("%p: Insert object at %d, cached %d, length %d", array, index, array->last_access_index, array->count);

	/****/
	/* add item at the beginning */
	if (0 == index)
	{
		/* array empty */
		if (0 == array->count) {
			return _append_object(array, object);
		}
		/* array not empty */
		else {
			struct AMCArrayItem *insItem = AMCMemPool_Alloc(g_item_mem_pool);
			if (NULL == insItem) {
				return _make_error(AMC_ARRAY_MEMPOOL_ERROR);
			}

			insItem->next = array->first;
			insItem->prev = NULL;
			insItem->value = object;

			array->first->prev = insItem;

			array->first = insItem;
			array->count ++;
			array->last_access_index ++;

			return _make_error(0);
		}
	}
	/****/
	/* insert in the end */
	else if (index >= array->count) {
		return _append_object(array, object);
	}
	/****/
	/* insert in the middle or in the end */
	else {
		struct AMCArrayItem *insItem = NULL;
		struct AMCArrayItem *prevItem = _object_at_index(array, index);
		if (NULL == prevItem) {
			return _make_error(AMC_ARRAY_EXCEED_ARRAY_SIZE);
		}

		insItem = AMCMemPool_Alloc(g_item_mem_pool);
		if (NULL == insItem) {
			return _make_error(AMC_ARRAY_MEMPOOL_ERROR);
		}

		insItem->next = prevItem;
		insItem->prev = prevItem->prev;
		insItem->value = object;

		insItem->next->prev = insItem;
		insItem->prev->next = insItem;

		array->count ++;
		array->last_access_index = index;
		array->last_access_item  = insItem;
		_DB("%p: After insert, cached %d[%p], length %d", array, array->last_access_index, array->last_access_item, array->count);

		return _make_error(0);
	}
}


/* --------------------_remove_object----------------------- */
AMCArrayErrno_st _remove_object(AMCArray_st *array, const size_t index, BOOL freeItem)
{
	_DB("%p: Remove object at %d, cached %d, length %d", array, index, array->last_access_index, array->count);	

	struct AMCArrayItem *itemToDel = _object_at_index(array, index);
	if (NULL == itemToDel) {
		return _make_error(AMC_ARRAY_EXCEED_ARRAY_SIZE);	// This will not happern
	}

	_DB("%p: Remove [%p]", array, itemToDel);
	array->count --;
	array->last_access_item = itemToDel->next;

	/* Is this first object in the array? */
	if (NULL == itemToDel->prev) {
		array->first = itemToDel->next;
	}
	else {
		itemToDel->prev->next = itemToDel->next;
	}

	/* Is this last object in the array? */
	if (NULL == itemToDel->next) {
		_DB("%p: Remove last object", array);
		array->last = itemToDel->prev;
		array->last_access_item = itemToDel->prev;
		array->last_access_index --;
	}
	else {
		itemToDel->next->prev = itemToDel->prev;
	}

	/* free item */
	if (freeItem && itemToDel->value) {MARK();
		free(itemToDel->value);
	}
	memset(itemToDel, 0, sizeof(*itemToDel));
	AMCMemPool_Free(itemToDel);
	itemToDel= NULL;

	/* return */
	return _make_error(0);
}


#endif


/********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

/* --------------------AMCArray_GlobalConfig----------------------- */
AMCArrayErrno_st AMCArray_GlobalConfig(ssize_t initArrayCount, ssize_t incrementArrayCount, ssize_t initItemCount, ssize_t incrementItemCount)
{
	AMCArrayErrno_st ret = {0, 0};

	if (g_item_mem_pool || g_array_mem_pool) {
		ret.array_error = AMC_ARRAY_AMC_ARRAY_INITIALIZED;
	}
	else {
		if ((initItemCount <= 0)
			|| (incrementItemCount <= 0)
			|| (initArrayCount <= 0)
			|| (incrementArrayCount <= 0)
			) {
			ret.array_error = AMC_ARRAY_INVALID_PARAMETER;
		}
		else {
			g_array_init_size = (size_t)initArrayCount;
			g_array_inc_size  = (size_t)incrementArrayCount;
			g_item_init_size = (size_t)initItemCount;
			g_item_inc_size  = (size_t)incrementItemCount;
		}
	}

	return ret;
}


/* --------------------AMCArray_New----------------------- */
AMCArray_st *AMCArray_New(AMCArrayErrno_st *pErrOut)
{
	AMCArrayErrno_st error = {0, 0};
	AMCArray_st *ret = NULL;

	if (0 != _check_and_init_amc_array())
	{
		error = _make_error(AMC_ARRAY_SYSTEM_ERROR);
		goto ENDS;
	}

	ret = AMCMemPool_Alloc(g_array_mem_pool);
	if (NULL == ret) {
		error = _make_error(AMC_ARRAY_MEMPOOL_ERROR);
		goto ENDS;
	}

	memset(ret, 0, sizeof(ret));
	pthread_mutex_init(&(ret->lock), NULL);

ENDS:
	if (pErrOut) {
		memcpy(pErrOut, &error, sizeof(error));
	}
	_DB("%p: created", ret);
	return ret;
}


/* --------------------AMCArray_NewWithObjects----------------------- */
AMCArray_st *AMCArray_NewWithObjects(void **pObjects, size_t count, AMCArrayErrno_st *pErrOut)
{
	if (pObjects && count) {
		/* OK */
	}
	else {
		pObjects = NULL;
		count = 0;
	}

	AMCArrayErrno_st error = {0, 0};
	AMCArray_st *ret = AMCArray_New(&error);

	if (ret) {
		size_t tmp = 0;

		for (tmp = 0; tmp < count; tmp++)
		{
			_append_object(ret, pObjects[tmp]);
		}
	}

	if (pErrOut) {
		memcpy(pErrOut, &error, sizeof(error));
	}
	_DB("%p: created with %d object(s)", ret, count);
	return ret;	
}


/* --------------------AMCArray_Free----------------------- */
AMCArrayErrno_st AMCArray_Free(AMCArray_st *array, BOOL freeObjects)
{
	if (NULL == array) {
		return _make_error(AMC_ARRAY_INVALID_PARAMETER);
	}

	AMCArrayErrno_st ret = {0, 0};

	_LOCK_ARRAY(array);
	ret = _array_free(array, freeObjects);
	_UNLOCK_ARRAY(array);

	if (0 == ret.array_error) {
		pthread_mutex_destroy(&(array->lock));
		AMCMemPool_Free(array);
		array = NULL;
	}

	return ret;
}


/* --------------------AMCArray_Count----------------------- */
size_t AMCArray_Count(const AMCArray_st *array)
{
	if (NULL == array) {
		return 0;
	}
	else {
		return array->count;
	}
}


/* --------------------AMCArray_ObjectAtIndex----------------------- */
void *AMCArray_ObjectAtIndex(AMCArray_st *array, size_t index)
{
	if (NULL == array) {
		return NULL;
	}

	struct AMCArrayItem *item = NULL;
	void *ret = NULL;

	_LOCK_ARRAY(array);
	
	item = _object_at_index(array, index);
	if (item) {
		ret = item->value;
	}
	_UNLOCK_ARRAY(array);
	

	return ret;
}


/* --------------------AMCArray_AddObject----------------------- */
AMCArrayErrno_st AMCArray_AddObject(AMCArray_st *array, void *object)
{
	if (NULL == array) {
		return _make_error(AMC_ARRAY_INVALID_PARAMETER);
	}

	AMCArrayErrno_st ret = {0, 0};
	
	_LOCK_ARRAY(array);
	ret = _append_object(array, object);
	_UNLOCK_ARRAY(array);

	return ret;
}


/* --------------------AMCArray_InsertObject----------------------- */
AMCArrayErrno_st AMCArray_InsertObject(AMCArray_st *array, void *object, size_t index)
{
	if (array && object) {
		/* OK */
	}
	else {
		return _make_error(AMC_ARRAY_INVALID_PARAMETER);
	}

	AMCArrayErrno_st ret = {0, 0};

	_LOCK_ARRAY(array);
	ret = _insert_object(array, object, index);
	_UNLOCK_ARRAY(array);

	return ret;
}


/* --------------------AMCArray_RemoveObject----------------------- */
AMCArrayErrno_st AMCArray_RemoveObject(AMCArray_st *array, size_t index, BOOL freeObject)
{
	if (NULL == array) {
		return _make_error(AMC_ARRAY_INVALID_PARAMETER);
	}

	AMCArrayErrno_st ret = {0, 0};

	_LOCK_ARRAY(array);
	ret = _remove_object(array, index, freeObject);
	_UNLOCK_ARRAY(array);

	return ret;
}


/* --------------------AMCArray_RemoveObject----------------------- */
void AMCAray_DumpStatus(AMCArray_st *array, FILE *file)
{
	if (NULL == array) {
		_DB("Array NULL");
		return;
	}

	if (NULL == file) {
		file = stdout;
	}

	struct AMCArrayItem *item = array->first;
	long tmp = 0;

	_LOCK_ARRAY(array);
	
	_DB("%p: Dump status:", array);
	
	fprintf(file, "Array %p, length %d, cached buffer %p(%d)\n", 
			array, array->count, array->last_access_item, array->last_access_index);
	fprintf(file, "First item: %p\n", array->first);
	fprintf(file, "Last item:  %p\n", array->last);

	while(item)
	{
		/* number */
		fprintf(file, "\t[%03ld] ", tmp);

		/* prev */
		if (NULL == item->prev) {
			fprintf(file, "     START -> ");
		}
		else {
			fprintf(file, "0x%08x -> ", TO_INT(item->prev));
		}

		/* this */
		fprintf(file, "0x%08x[0x%08x] -> ", TO_INT(item), TO_INT(item->value));
		
		/* next */
		if (NULL == item->next) {
			fprintf(file, "END\n");
		}
		else {
			fprintf(file, "0x%08x\n", TO_INT(item->next));
		}

		item = item->next;
		tmp ++;
		if (tmp > array->count) {
			_DB("Exception !!!");
			exit(-1);
		}
	}
	
	_UNLOCK_ARRAY(array);

	return;
}


#endif

/********/
/* end of file */

