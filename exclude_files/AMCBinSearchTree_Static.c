/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCBinSearchTree_Static.c
	Description: 	
			This file implement binary tree in file AMCTree.h.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-15: File created
		2015-01-19: File renamed from "AMCBinSearchTree.c" to "AMCBinSearchTree_Static.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCTree.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

//#define _DEBUG_FLAG


/********************/
#define __INTERNAL_STRUCTURE_DEFINITIONS
#ifdef __INTERNAL_STRUCTURE_DEFINITIONS

#ifdef _DEBUG_FLAG
#define _DEBUG(fmt, args...)		printf("##"__FILE__", %d: "fmt"\n", __LINE__, ##args);
#else
#define _DEBUG(fmt, args...)
#endif

#ifndef BOOL
#define BOOL	long
#define FALSE	(0)
#define TRUE	(!0)
#endif

#define _ULONG_CVT(val)		((unsigned long)(val))
#define _RETURN_IF_COND(retVal, condition)	if (condition) return (retVal)

/***** static ones *****/
typedef struct AMCStaticBstNode {
	struct AMCStaticBstNode         *pLeft;
	struct AMCStaticBstNode         *pRight;
	unsigned long                   id;
	// content is allocated in memory pool
} AMCStaticBstNode_st;


typedef struct AMCStaticBinSearchTree {
	struct AMCStaticBstNode         *entry;
	struct AMCMemPool               *memPool;
	pthread_rwlock_t                rwLock;
	unsigned long                   nodeCount;
	unsigned long                   contentSize;		// without header size
} AMCStaticBinSearchTree_st;


/***** dynamic ones *****/
typedef struct AMCDynamicBstNode {
	struct AMCDynamicBstNode        *pLeft;
	struct AMCDynamicBstNode        *pRight;
	unsigned long                   id;
	unsigned long                   contentSize;
	void                            *content;
} AMCDynamicBstNode_st;

typedef struct AMCDynamicBinSearchTree {
	struct AMCDynamicBstNode        *entry;
	pthread_rwlock_t                rwLock;
	unsigned long                   nodeCount;
} AMCDynamicBinSearchTree_st;


#endif


#define _SET_ERROR(pErr, error)		if (pErr) (*(pErr) = (error))


/********************/
#define __PRIVATE_FUNCTIONS
#ifdef __PRIVATE_FUNCTIONS

static inline int _lock_Read_STree(struct AMCStaticBinSearchTree *tree)
{
	return pthread_rwlock_rdlock(&(tree->rwLock));
}

static inline int _unlock_Read_STree(struct AMCStaticBinSearchTree *tree)
{
	return pthread_rwlock_unlock(&(tree->rwLock));
}

static inline int _lock_Write_STree(struct AMCStaticBinSearchTree *tree)
{
	return pthread_rwlock_wrlock(&(tree->rwLock));
}

static inline int _unlock_Write_STree(struct AMCStaticBinSearchTree *tree)
{
	return pthread_rwlock_unlock(&(tree->rwLock));
}


#endif


/********************/
#define __PRIVATE_STATIC_TREE_FUNCTIONS
#ifdef __PRIVATE_STATIC_TREE_FUNCTIONS


static struct AMCStaticBstNode *_sTree_AllocNode(struct AMCStaticBinSearchTree *tree, const unsigned long id)
{
	AMCStaticBstNode_st *ret = AMCMemPool_Alloc(tree->memPool);
	if (ret)
	{
		ret->id = id;
		ret->pLeft = NULL;
		ret->pRight = NULL;
		memset((void*)(_ULONG_CVT(ret) + sizeof(*ret)), 0, tree->contentSize);
	}
	return ret;
}


inline static int _sTree_FreeNode(struct AMCStaticBstNode *node)
{
	return AMCMemPool_Free(node);
}


static struct AMCStaticBstNode *_sTree_FindMinLeaf(struct AMCStaticBstNode *node, struct AMCStaticBstNode **parent)
{
	struct AMCStaticBstNode *ret = node;
	struct AMCStaticBstNode *parentNode = NULL;

	while(ret->pLeft)
	{
		parentNode = ret;
		ret = ret->pLeft;
	}
	
	*parent = parentNode;
	return ret;
}

/*
static struct AMCStaticBstNode *_sTree_FindMaxLeaf(struct AMCStaticBstNode *node, struct AMCStaticBstNode **parent)
{
	struct AMCStaticBstNode *ret = node;
	struct AMCStaticBstNode *parentNode = NULL;

	while(ret->pRight)
	{
		parentNode = ret;
		ret = ret->pRight;
	}
	
	*parent = parentNode;
	return ret;
}
*/

static struct AMCStaticBstNode *_sTree_FindId(struct AMCStaticBinSearchTree *tree, const unsigned long id, struct AMCStaticBstNode **parentNode)
{
	AMCStaticBstNode_st *ret = tree->entry;
	AMCStaticBstNode_st *parent = NULL;
	BOOL isFound = FALSE;
	
	if (NULL == ret)
	{
		return NULL;
	}

	/* let's start searching! */
	do
	{
		if (id == ret->id)
		{
			isFound = TRUE;
		}
		else if (id < ret->id)
		{
			parent = ret;
			ret = ret->pLeft;
		}
		else
		{
			parent = ret;
			ret = ret->pRight;
		}
	}
	while((FALSE == isFound) && ret);

	*parentNode = parent;
	return ret;
}


static struct AMCStaticBstNode *_sTree_Insert(struct AMCStaticBinSearchTree *tree, const unsigned long id, BOOL *isDuplicated)
{
	AMCStaticBstNode_st *node = tree->entry;
	AMCStaticBstNode_st *ret = NULL;
	
	*isDuplicated = FALSE;

	if (NULL == tree->entry)
	{
		ret = _sTree_AllocNode(tree, id);
		_RETURN_IF_COND(NULL, 0 == ret);
		tree->entry = ret;
		tree->nodeCount ++;
		return ret;
	}

	/* let's find the correct position */
	do
	{
		if (id == node->id)
		{
			*isDuplicated = TRUE;
		}
		else if (id < node->id)
		{
			if (NULL == node->pLeft)		/* found a slot */
			{
				ret = _sTree_AllocNode(tree, id);
				_RETURN_IF_COND(NULL, 0 == ret);
				node->pLeft = ret;
				tree->nodeCount ++;
			}
			else
			{
				node = node->pLeft;
			}
		}
		else if (id > node->id)
		{
			if (NULL == node->pRight)		/* found a slot */
			{
				ret = _sTree_AllocNode(tree, id);
				_RETURN_IF_COND(NULL, 0 == ret);
				node->pRight = ret;
				tree->nodeCount ++;
			}
			else
			{
				node = node->pRight;
			}
		}
	}
	while((NULL == ret) && (FALSE == *isDuplicated));

	/* ends */
	return ret;
}


static void _sTree_NodeContentCopy(const struct AMCStaticBinSearchTree *tree, struct AMCStaticBstNode *node, const void *source)
{
	memcpy((void*)(_ULONG_CVT(node) + sizeof(*node)), 
			source, 
			tree->contentSize);
	return;
}


static BOOL _sTree_NodeIsLeaf(const struct AMCStaticBstNode *node)
{
	return ((NULL == node->pLeft) && (NULL == node->pRight));
}


static BOOL _sTree_NodeIsOneLeafBranch(const struct AMCStaticBstNode *node)
{
	if (NULL == node->pLeft)
	{
		return (NULL == node->pRight) ? FALSE : TRUE;
	}
	else
	{
		return (NULL == node->pRight) ? TRUE : FALSE;
	}
}


static int _sTree_DeleteLeaf(struct AMCStaticBinSearchTree *tree, struct AMCStaticBstNode *node, struct AMCStaticBstNode *parent)
{
	if (parent)
	{
		if (parent->pLeft == node)
		{
			parent->pLeft = NULL;
		}
		else
		{
			parent->pRight = NULL;
		}
		tree->nodeCount --;
		_sTree_FreeNode(node);
	}
	else
	{
		tree->entry = NULL;
		tree->nodeCount = 0;
		_sTree_FreeNode(node);
	}
	return 0;
}


static int _sTree_DeleteOneLeafBranch(struct AMCStaticBinSearchTree *tree, struct AMCStaticBstNode *node, struct AMCStaticBstNode *parent)
{
	if (parent)
	{
		AMCStaticBstNode_st *childNode = (node->pLeft) ? node->pLeft : node->pRight;
		if (parent->pLeft == node)
		{
			parent->pLeft = childNode;
		}
		else
		{
			parent->pRight = childNode;
		}
		tree->nodeCount --;
		_sTree_FreeNode(node);
	}
	else
	{
		if (node->pLeft)
		{
			tree->entry = node->pLeft;
		}
		else
		{
			tree->entry = node->pRight;
		}
		tree->nodeCount = 1;
		_sTree_FreeNode(node);
	}
	return 0;
}


static int _sTree_DeleteNode(struct AMCStaticBinSearchTree *tree, struct AMCStaticBstNode *node, struct AMCStaticBstNode *parent)
{
	if (_sTree_NodeIsLeaf(node))
	{
		return _sTree_DeleteLeaf(tree, node, parent);
	}
	else if (_sTree_NodeIsOneLeafBranch(node))
	{
		return _sTree_DeleteOneLeafBranch(tree, node, parent); 
	}
	else
	{
		struct AMCStaticBstNode *smallestNode = _sTree_FindMinLeaf(node->pRight, &parent);
		parent = parent ? parent : node;
		
		/* replace smallest node */
		node->id = smallestNode->id;
		_sTree_NodeContentCopy(tree, node, (void*)(_ULONG_CVT(smallestNode) + sizeof(*smallestNode)));

		/* start operation with the smallest node */
		return _sTree_DeleteNode(tree, smallestNode, parent);		// "parent" is updated in first line of this block
	}
}


static int _sTree_Delete(struct AMCStaticBinSearchTree *tree, const unsigned long id)
{
	struct AMCStaticBstNode *parent = NULL;
	struct AMCStaticBstNode *node = _sTree_FindId(tree, id, &parent);

	if (NULL == node)
	{
		return AMCTreeError_IdNotFound;
	}

	return _sTree_DeleteNode(tree, node, parent);
}


#endif


/********************/
#define __PRIVATE_DUMP_FUNCTIONS
#ifdef __PRIVATE_DUMP_FUNCTIONS
/*
typedef struct {
	char str[24];
} _IdStr_st;


static _IdStr_st _dump_IdStr(unsigned long id, unsigned long decWidth)
{
	_IdStr_st ret;

	switch(decWidth)
	{
		case 0:
		case 1:
			sprintf(ret.str, "%ld", id);
			break;
		case 2:
			sprintf(ret.str, "%02ld", id);
			break;
		case 3:
			sprintf(ret.str, "%03ld", id);
			break;
		case 4:
			sprintf(ret.str, "%04ld", id);
			break;
		case 5:
			sprintf(ret.str, "%05ld", id);
			break;
		case 6:
			sprintf(ret.str, "%06ld", id);
			break;
		case 7:
			sprintf(ret.str, "%07ld", id);
			break;
		case 8:
		default:
			sprintf(ret.str, "%08ld", id);
			break;
	}
	return ret;
}
*/

static void _sTree_DumpNode(const struct AMCStaticBstNode *node, unsigned long tabCount)
{
	if ((NULL == node->pLeft) && (NULL == node->pRight))
	{
		return;
	}

	unsigned long tmp;
	for (tmp = 0; tmp < tabCount; tmp++)
	{
		printf("\t");
	}

	/* node */
	printf("Node (%ld), ", node->id);

	/* left */
	if (node->pLeft)
	{
		printf("<(%ld), ", node->pLeft->id);
	}
	else
	{
		printf("<(_), ");
	}

	/* right */
	if (node->pRight)
	{
		printf("(%ld)>\n", node->pRight->id);
	}
	else
	{
		printf("(_)>\n");
	}

	/* call self */
	if (node->pLeft)
	{
		_sTree_DumpNode(node->pLeft, tabCount + 1);
	}

	if (node->pRight)
	{
		_sTree_DumpNode(node->pRight, tabCount + 1);
	}

	return;
}


static void _sTree_Dump(const struct AMCStaticBinSearchTree *tree)
{
	if (NULL == tree->entry)
	{
		printf("Tree 0x%08lx, size: 0\n", _ULONG_CVT(tree));
		return;
	}
	else if (1 == tree->nodeCount)
	{
		printf("Tree 0x%08lx, size: 1\n\tNode (%ld)\n", _ULONG_CVT(tree), tree->entry->id);
	}
	else
	{
		printf("Tree 0x%08lx, size: %ld\n", _ULONG_CVT(tree), tree->nodeCount);
		_sTree_DumpNode(tree->entry, 1);
		return;
	}
}


#endif




/********************/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

struct AMCStaticBinSearchTree *AMCStaticBsTree_New(unsigned long contentSize, unsigned long expectedSize, int *errorOut)
{
	AMCStaticBinSearchTree_st *ret = malloc(sizeof(*ret));
	int callStat;

	if (NULL == ret)
	{
		_SET_ERROR(errorOut, AMCTreeError_SysCallError);
		return NULL;
	}
	else if (0 == contentSize)
	{
		free(ret);
		_SET_ERROR(errorOut, AMCTreeError_ParaError);
		return NULL;
	}
	else
	{}
	
	if (expectedSize <= 1)
	{
		expectedSize = CFG_DEFAULT_EXPECTED_NODE_COUNT;
	}

	/* basic members */
	ret->contentSize = contentSize;
	ret->entry = NULL;
	ret->nodeCount = 0;

	/* memory pool */
	ret->memPool = AMCMemPool_Create(sizeof(AMCStaticBstNode_st) + contentSize, expectedSize, expectedSize, FALSE);
	if (NULL == ret->memPool)
	{
		_SET_ERROR(errorOut, AMCTreeError_MemPoolError);
		free(ret);
		return NULL;
	}

	/* readwrite lock */
	callStat = pthread_rwlock_init(&(ret->rwLock), NULL);
	if (0 != callStat)
	{
		_SET_ERROR(errorOut, AMCTreeError_SysCallError);
		AMCMemPool_Destory(ret->memPool);
		free(ret);
		return NULL;
	}

	/* success */
	return ret;
}


int AMCStaticBsTree_Destroy(struct AMCStaticBinSearchTree *tree)
{
	if (NULL == tree)
	{
		return AMCTreeError_ParaError;
	}

	pthread_rwlock_destroy(&(tree->rwLock));
	AMCMemPool_Destory(tree->memPool);
	tree->memPool = NULL;
	tree->nodeCount = 0;
	free(tree);

	return 0;
}


int AMCStaticBsTree_Insert(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *content)
{
	BOOL isDuplicated;
	struct AMCStaticBstNode *node;
	int ret = 0;

	if (NULL == tree)
	{
		return AMCTreeError_ParaError;
	}

	if (NULL == tree->memPool)
	{
		return AMCTreeError_TreeInvalid;
	}


	_lock_Write_STree(tree);

	node = _sTree_Insert(tree, id, &isDuplicated);
	if (NULL == node)
	{
		ret = AMCTreeError_MemPoolError;
	}
	else if (isDuplicated)
	{
		ret = AMCTreeError_IdExists;
	}
	else
	{
		if(content)
		{
			_sTree_NodeContentCopy(tree, node, content);
		}
		else
		{}
	}

	_unlock_Write_STree(tree);

	return ret;
}


int AMCStaticBsTree_ForceInsert(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *content)
{
	BOOL isDuplicated;
	struct AMCStaticBstNode *node;
	int ret = 0;

	if (NULL == tree)
	{
		return AMCTreeError_ParaError;
	}

	if (NULL == tree->memPool)
	{
		return AMCTreeError_TreeInvalid;
	}


	_lock_Write_STree(tree);

	node = _sTree_Insert(tree, id, &isDuplicated);
	if (NULL == node)
	{
		ret = AMCTreeError_MemPoolError;
	}
	else if (isDuplicated)
	{
		ret = AMCTreeError_IdExists;
	}
	else
	{}

	if(content && node)
	{
		_sTree_NodeContentCopy(tree, node, content);
	}
	else
	{}
	
	_unlock_Write_STree(tree);

	return ret;
}


int AMCStaticBsTree_Delete(struct AMCStaticBinSearchTree *tree, unsigned long id)
{
	int ret;

	_lock_Write_STree(tree);
	ret = _sTree_Delete(tree, id);
	_unlock_Write_STree(tree);

	return ret;
}


unsigned long AMCStaticBsTree_NodeSize(struct AMCStaticBinSearchTree *tree)
{
	return tree->contentSize;
}


void *AMCStaticBsTree_GetContent(struct AMCStaticBinSearchTree *tree, unsigned long id, int *errorOut)
{
	struct AMCStaticBstNode *node = NULL;
	struct AMCStaticBstNode *parent = NULL;

	if (NULL == tree)
	{
		_SET_ERROR(errorOut, AMCTreeError_ParaError);
		return NULL;
	}

	_lock_Read_STree(tree);
	node = _sTree_FindId(tree, id, &parent);
	_unlock_Read_STree(tree);

	if (node)
	{
		_SET_ERROR(errorOut, 0);
		return (void*)(_ULONG_CVT(node) + sizeof(*node));
	}
	else
	{
		_SET_ERROR(errorOut, AMCTreeError_IdNotFound);
		return NULL;
	}
}


int AMCStaticBsTree_ReadContent(struct AMCStaticBinSearchTree *tree, unsigned long id, void *contentOut, unsigned long bufferSize)
{
	int ret = 0;
	struct AMCStaticBstNode *node = NULL;
	struct AMCStaticBstNode *parent = NULL;

	if ((NULL == tree) || (NULL == contentOut))
	{
		return AMCTreeError_ParaError;
	}

	if (bufferSize < tree->contentSize)
	{
		return AMCTreeError_BufferTooSmall;
	}

	_lock_Read_STree(tree);
	node = _sTree_FindId(tree, id, &parent);
	if (node)
	{
		memcpy(contentOut, (void*)(_ULONG_CVT(node) + sizeof(*node)), tree->contentSize);
	}
	else
	{
		ret = AMCTreeError_IdNotFound;
	}
	_unlock_Read_STree(tree);

	return ret;
}


int AMCStaticBsTree_WriteContent(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *contentIn, unsigned long bufferSize)
{
	int ret = 0;
	struct AMCStaticBstNode *node = NULL;
	struct AMCStaticBstNode *parent = NULL;

	if ((NULL == tree) || (NULL == contentIn))
	{
		return AMCTreeError_ParaError;
	}

	if (bufferSize > tree->contentSize)
	{
		return AMCTreeError_BufferTooSmall;
	}

	_lock_Write_STree(tree);
	node = _sTree_FindId(tree, id, &parent);
	if (node)
	{
		memcpy((void*)(_ULONG_CVT(node) + sizeof(*node)), contentIn, bufferSize);
		memset((void*)(_ULONG_CVT(node) + sizeof(*node) + bufferSize), 0, tree->contentSize - bufferSize);
	}
	else
	{
		ret = AMCTreeError_IdNotFound;
	}
	_lock_Write_STree(tree);

	return ret;
}


void AMCStaticBsTree_DebugStdout(struct AMCStaticBinSearchTree *tree)
{
	if (NULL == tree)
	{
		return;
	}

	_lock_Read_STree(tree);
	_sTree_Dump(tree);
	_unlock_Read_STree(tree);

	return;
}




#endif







