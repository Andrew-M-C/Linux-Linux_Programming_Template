/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCRedBlackTree_Static.c
	Description: 	
			This file implement Red-black tree in file AMCTree.h. 
			
	Reference: 
			http://zh.wikipedia.org/wiki/%E7%BA%A2%E9%BB%91%E6%A0%91
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-16: File created

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

#define _DEBUG_FLAG


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

typedef enum {
	_black = 0,
	_red = 1
} _color_t;

#define _ULONG_CVT(val)		((unsigned long)(val))
#define _RETURN_IF_COND(retVal, condition)	if (condition) return (retVal)

#define _IS_RED(pNode)		((pNode)->color)
#define _IS_BLACK(pNode)	(_black == ((pNode)->color))
#define _SET_RED(pNode)		if (pNode){((pNode)->color = _red);}do{}while(0)
#define _SET_BLACK(pNode)	if (pNode){((pNode)->color = _black);}do{}while(0)

typedef struct AMCStaticRbNode {
	struct AMCStaticRbNode          *pLeft;
	struct AMCStaticRbNode          *pRight;
	struct AMCStaticRbNode          *pParent;
	unsigned long                   id;
	_color_t                        color;
	// content is allocated in memory pool
} AMCStaticRbNode_st;

typedef struct AMCStaticRedBlackTree {
	struct AMCStaticRbNode         *entry;
	struct AMCMemPool               *memPool;
	pthread_rwlock_t                rwLock;
	unsigned long                   nodeCount;
	unsigned long                   contentSize;		// without header size
} AMCStaticRbTree_st;


#define _SET_ERROR(pErr, error)		if (pErr) (*(pErr) = (error))

static void __rbTree_CheckDeleteNodeCase1(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void __rbTree_CheckDeleteNodeCase2(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void __rbTree_CheckDeleteNodeCase3(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void __rbTree_CheckDeleteNodeCase4(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void __rbTree_CheckDeleteNodeCase5(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void __rbTree_CheckDeleteNodeCase6(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node);
static void _rbTree_DumpNode(const AMCStaticRbNode_st *node, unsigned long tabCount);
static void _rbTree_Dump(const AMCStaticRbTree_st *tree);

#endif


/********************/
#define __PRIVATE_FUNCTIONS
#ifdef __PRIVATE_FUNCTIONS

static inline int _lock_Read_RbTree(AMCStaticRbTree_st *tree)
{
	return pthread_rwlock_rdlock(&(tree->rwLock));
}

static inline int _unlock_Read_RbTree(AMCStaticRbTree_st *tree)
{
	return pthread_rwlock_unlock(&(tree->rwLock));
}

static inline int _lock_Write_RbTree(AMCStaticRbTree_st *tree)
{
	return pthread_rwlock_wrlock(&(tree->rwLock));
}

static inline int _unlock_Write_RbTree(AMCStaticRbTree_st *tree)
{
	return pthread_rwlock_unlock(&(tree->rwLock));
}


static inline BOOL _rbTree_NodeIsRoot(const AMCStaticRbNode_st *node)
{
	return (NULL == node->pParent);
}

/*
static BOOL _rbTree_NodeHasChild(const AMCStaticRbNode_st *node)
{
	return ((node->pLeft) || (node->pRight));
}
*/
static struct AMCStaticRbNode *_rbTree_BrotherNode(AMCStaticRbNode_st *node)
{
	if (_rbTree_NodeIsRoot(node))
	{
		return NULL;
	}
	else
	{
		if (node == node->pParent->pLeft)
		{
			return node->pParent->pRight;
		}
		else
		{
			return node->pParent->pLeft;
		}
	}
}


static void _rbTree_NodeContentCopy(const AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node, const void *source)
{
	memcpy((void*)(_ULONG_CVT(node) + sizeof(*node)), 
			source, 
			tree->contentSize);
	return;
}


static inline void _rbTree_NodeContentCopyByNode(const AMCStaticRbTree_st *tree, AMCStaticRbNode_st *nodeTo, const AMCStaticRbNode_st *nodeFrom)
{
	_rbTree_NodeContentCopy(tree, nodeTo, (void*)(_ULONG_CVT(nodeFrom) + sizeof(*nodeFrom)));
}


#endif




/********************/
#define __PRIVATE_STATIC_TREE_FUNCTIONS
#ifdef __PRIVATE_STATIC_TREE_FUNCTIONS

static struct AMCStaticRbNode *_rbTree_AllocNode(AMCStaticRbTree_st *tree, const unsigned long id, _color_t color)
{
	struct AMCStaticRbNode *ret = AMCMemPool_Alloc(tree->memPool);
	if (ret)
	{
		ret->id = id;
		ret->pLeft = NULL;
		ret->pRight = NULL;
		ret->color = color;
		ret->pParent = NULL;
		memset((void*)(_ULONG_CVT(ret) + sizeof(*ret)), 0, tree->contentSize);
	}
	return ret;
}


inline static int _rbTree_FreeNode(AMCStaticRbNode_st *node)
{
	return AMCMemPool_Free(node);
}


static AMCStaticRbNode_st *_rbTree_FindMinLeaf(AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *ret = node;

	while(ret->pLeft)
	{
		ret = ret->pLeft;
	}
	
	return ret;
}


static struct AMCStaticRbNode *_rbTree_FindNode(AMCStaticRbTree_st *tree, const unsigned long id)
{
	AMCStaticRbNode_st *ret = tree->entry;
	AMCStaticRbNode_st *parent = NULL;
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

	return ret;
}


static void __rbTree_RotateLeft(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	struct AMCStaticRbNode *prevRight = node->pRight;
	struct AMCStaticRbNode *prevRightLeft = node->pRight->pLeft;
	_DEBUG("Rotate left %ld", node->id);
	_DEBUG("prevRight %ld, prevRightLeft %ld", prevRight ? prevRight->id : -1, prevRightLeft ? prevRightLeft->id : -1);

	if (_rbTree_NodeIsRoot(node))
	{
		tree->entry = prevRight;
		
		node->pParent = prevRight;
		node->pRight = prevRightLeft;
		if (prevRightLeft)
		{
			prevRightLeft->pParent = node;
		}
		
		prevRight->pLeft = node;
		prevRight->pParent = NULL;
	}
	else
	{
		AMCStaticRbNode_st *parent = node->pParent;
		_DEBUG("Test:");
		_rbTree_DumpNode(parent, 5);
		if (node == node->pParent->pLeft)
		{
			parent->pLeft = prevRight;
		}
		else
		{
			parent->pRight = prevRight;
		}

		node->pParent = prevRight;
		node->pRight = prevRightLeft;
		if (prevRightLeft)
		{
			prevRightLeft->pParent = node;
		}

		prevRight->pLeft = node;
		prevRight->pParent = parent;
		_DEBUG("Test:");
		_rbTree_DumpNode(parent, 5);
	}
	return;
}


static void __rbTree_RotateRight(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	struct AMCStaticRbNode *prevLeft = node->pLeft;
	struct AMCStaticRbNode *prevLeftRight = node->pLeft->pRight;
	_DEBUG("Rotate right %ld", node->id);

	if (_rbTree_NodeIsRoot(node))
	{
		tree->entry = prevLeft;
		
		node->pParent = prevLeft;
		node->pLeft = prevLeftRight;
		if (prevLeftRight)
		{
			prevLeftRight->pParent = node;
		}
		
		prevLeft->pRight = node;
		prevLeft->pParent = NULL;
	}
	else
	{
		AMCStaticRbNode_st *parent = node->pParent;
		if (node == node->pParent->pLeft)
		{
			parent->pLeft = prevLeft;
		}
		else
		{
			parent->pRight = prevLeft;
		}

		node->pParent = prevLeft;
		node->pLeft = prevLeftRight;
		if (prevLeftRight)
		{
			prevLeftRight->pParent = node;
		}

		prevLeft->pRight = node;
		prevLeft->pParent = parent;
	}
	return;
}


static void __rbTree_CheckInsert(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	struct AMCStaticRbNode *uncleNode = node->pParent ? _rbTree_BrotherNode(node->pParent) : NULL;

	if (_rbTree_NodeIsRoot(node))								// root node. This is impossible because it is done previously
	{_DEBUG("MARK (%ld) case 1", node->id);
		_SET_BLACK(node);
	}
	else if (_IS_BLACK(node->pParent))	// parent black. Nothing addition needed
	{_DEBUG("MARK (%ld) case 2", node->id);
		// nop
	}
	else if (uncleNode &&
			_IS_RED(uncleNode))	// parent and uncle node are both red
	{_DEBUG("MARK (%ld) case 3, uncle: %ld, parent: %ld", node->id, uncleNode->id, node->pParent->id);
		_SET_BLACK(node->pParent);
		_SET_BLACK(uncleNode);
		_SET_RED(node->pParent->pParent);
		__rbTree_CheckInsert(tree, node->pParent->pParent);
	}
	else				// parent red, meanwhile uncle is black or NULL 
	{
		// Step 1
		if ((node == node->pParent->pRight) &&
			(node->pParent == node->pParent->pParent->pLeft))	// node is its parent's left child, AND parent is grandparent's right child. left-rotation needed.
		{_DEBUG("MARK (%ld) case 4", node->id);
			__rbTree_RotateLeft(tree, node->pParent);
			node = node->pLeft;
			_DEBUG("MARK test:");
			_rbTree_Dump(tree);
		}
		else if ((node == node->pParent->pLeft) &&
				(node->pParent == node->pParent->pParent->pRight))
		{_DEBUG("MARK (%ld) case 4", node->id);
			__rbTree_RotateRight(tree, node->pParent);
			node = node->pRight;
			_DEBUG("MARK test:");
			_rbTree_Dump(tree);
		}
		else
		{}

		// Step 2
		_SET_BLACK(node->pParent);
		_SET_RED(node->pParent->pParent);

		if ((node == node->pParent->pLeft) &&
			(node->pParent == node->pParent->pParent->pLeft))
		{_DEBUG("MARK (%ld) case 5", node->id);
			__rbTree_RotateRight(tree, node->pParent->pParent);
		}
		else
		{_DEBUG("MARK (%ld) case 5", node->id);
			__rbTree_RotateLeft(tree, node->pParent->pParent);
			_DEBUG("MARK test:");
			_rbTree_Dump(tree);
		}
	}
	return;
}


static struct AMCStaticRbNode *_rbTree_Insert(AMCStaticRbTree_st *tree, const unsigned long id, BOOL *isDuplicated)
{
	AMCStaticRbNode_st *node = tree->entry;
	AMCStaticRbNode_st *ret = NULL;
	AMCStaticRbNode_st *parent = tree->entry;

	*isDuplicated = FALSE;
	if (NULL == tree->entry)	// root
	{
		ret = _rbTree_AllocNode(tree, id, _black);
		_RETURN_IF_COND(NULL, 0 == ret);
		tree->entry = ret;
		tree->nodeCount ++;
		return ret;
	}

	/* insert a node by binary search tree method */
	do
	{
		if (id == node->id)
		{
			*isDuplicated = TRUE;
		}
		else if (id < node->id)
		{
			if (NULL == node->pLeft)		/* found a slot */
			{_DEBUG("Found parent %ld", node->id);
				ret = _rbTree_AllocNode(tree, id, _red);
				_RETURN_IF_COND(NULL, 0 == ret);
				node->pLeft = ret;
				ret->pParent = node;
				tree->nodeCount ++;
			}
			else
			{
				parent = node;
				node = node->pLeft;
			}
		}
		else
		{
			if (NULL == node->pRight)		/* found a slot */
			{_DEBUG("Found parent %ld", node->id);
				ret = _rbTree_AllocNode(tree, id, _red);
				_RETURN_IF_COND(NULL, 0 == ret);
				node->pRight = ret;
				ret->pParent = node;
				tree->nodeCount ++;
			}
			else
			{
				parent = node;
				node = node->pRight;
			}
		}
	}
	while((NULL == ret) && (FALSE == *isDuplicated));

	/* special red-black tree operation */
	if (FALSE == *isDuplicated)
	{
		__rbTree_CheckInsert(tree, ret);
	}

	/* ends */
	return ret;
}


static void _rbTree_DeleteLeaf(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	if (_rbTree_NodeIsRoot(node))
	{
		tree->entry = NULL;
		tree->nodeCount = 0;
		_rbTree_FreeNode(node);
	}
	else
	{_DEBUG("DEL: %ld <- %ld", node->id, node->pParent->id)
		if (node == node->pParent->pLeft)
		{
			node->pParent->pLeft= NULL;
		}
		else
		{
			node->pParent->pRight = NULL;
		}

		tree->nodeCount --;
		_rbTree_FreeNode(node);
	}

	return;
}


static inline void _rbTree_ReadNodeStatus(const AMCStaticRbNode_st *node, BOOL *hasChild, BOOL *hasTwoChild, BOOL *hasLeftChild, BOOL *hasRightChild)
{
	if (node->pLeft)
	{
		if (node->pRight)
		{
			*hasChild      = TRUE;
			*hasTwoChild   = TRUE;
			*hasLeftChild  = TRUE;
			*hasRightChild = TRUE;
		}
		else
		{
			*hasChild      = TRUE;
			*hasTwoChild   = FALSE;
			*hasLeftChild  = TRUE;
			*hasRightChild = FALSE;
		}
	}
	else
	{
		if (node->pRight)
		{
			*hasChild      = TRUE;
			*hasTwoChild   = FALSE;
			*hasLeftChild  = FALSE;
			*hasRightChild = TRUE;
		}
		else
		{
			*hasChild      = FALSE;
			*hasTwoChild   = FALSE;
			*hasLeftChild  = FALSE;
			*hasRightChild = FALSE;
		}
	}
}


static void __rbTree_DeleteNodeAndReonnectWith(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *nodeToDel, AMCStaticRbNode_st *nodeToReplace)
{
	if (_rbTree_NodeIsRoot(nodeToDel))
	{
		tree->nodeCount --;
		tree->entry = nodeToReplace;
		nodeToReplace->pParent = NULL;
	}
	else
	{
		tree->nodeCount --;
		nodeToReplace->pParent = nodeToDel->pParent;

		if (nodeToDel == nodeToDel->pParent->pLeft)
		{
			nodeToDel->pParent->pLeft = nodeToReplace;
		}
		else
		{
			nodeToDel->pParent->pRight = nodeToReplace;
		}
	}
}


static void __rbTree_CheckDeleteNodeCase6(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *sibling = _rbTree_BrotherNode(node->pParent);

	sibling->color = node->pParent->color;
	_SET_BLACK(node->pParent);

	if (node == node->pParent->pLeft)
	{
		_SET_BLACK(sibling->pRight);
		__rbTree_RotateLeft(tree, node->pParent);
	}
	else
	{
		_SET_BLACK(sibling->pLeft);
		__rbTree_RotateRight(tree, node->pParent);
	}

	return;
}


static void __rbTree_CheckDeleteNodeCase5(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *sibling = _rbTree_BrotherNode(node->pParent);
	BOOL sblLeftIsBlack = (sibling->pLeft) ? _IS_BLACK(sibling->pLeft) : TRUE;
	BOOL sblRightIsBlack = (sibling->pRight) ? _IS_BLACK(sibling->pRight) : TRUE;

	if (_IS_RED(sibling))
	{}
	else if ((node == node->pParent->pLeft) &&
		(FALSE == sblLeftIsBlack) &&
		sblRightIsBlack)
	{
		_SET_RED(sibling);
		_SET_BLACK(sibling->pLeft);
		__rbTree_RotateRight(tree, sibling);
	}
	else if ((node == node->pParent->pRight) &&
			sblLeftIsBlack &&
			(FALSE == sblRightIsBlack))
	{
		_SET_RED(sibling);
		_SET_BLACK(sibling->pRight);
		__rbTree_RotateLeft(tree, sibling);
	}
	else
	{}

	__rbTree_CheckDeleteNodeCase6(tree, node);
}

static void __rbTree_CheckDeleteNodeCase4(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *sibling = _rbTree_BrotherNode(node->pParent);
	BOOL sblLeftIsBlack = (sibling->pLeft) ? _IS_BLACK(sibling->pLeft) : TRUE;
	BOOL sblRightIsBlack = (sibling->pRight) ? _IS_BLACK(sibling->pRight) : TRUE;

	if (_IS_RED(node->pParent) &&
		_IS_BLACK(sibling) &&
		sblLeftIsBlack && sblRightIsBlack)
	{_DEBUG("DEL: case 4");
		_SET_RED(sibling);
		_SET_BLACK(node->pParent);
	}
	else
	{
		__rbTree_CheckDeleteNodeCase5(tree, node);
	}
}

static void __rbTree_CheckDeleteNodeCase3(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *sibling = _rbTree_BrotherNode(node->pParent);
	BOOL sblLeftIsBlack = (sibling->pLeft) ? _IS_BLACK(sibling->pLeft) : TRUE;
	BOOL sblRightIsBlack = (sibling->pRight) ? _IS_BLACK(sibling->pRight) : TRUE;

	if (_IS_BLACK(node->pParent) &&
		_IS_BLACK(sibling) &&
		sblLeftIsBlack && sblRightIsBlack)
	{_DEBUG("DEL: case 3");
		_SET_RED(sibling);
		__rbTree_CheckDeleteNodeCase1(tree, node->pParent);
	}
	else
	{
		__rbTree_CheckDeleteNodeCase4(tree, node);
	}
}

static void __rbTree_CheckDeleteNodeCase2(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	AMCStaticRbNode_st *sibling = _rbTree_BrotherNode(node);
		
	if (_IS_RED(sibling))
	{_DEBUG("DEL: case 2");
		_SET_RED(node->pParent);
		_SET_BLACK(sibling);

		if (node == node->pParent->pLeft)
			__rbTree_RotateLeft(tree, node->pParent);
		else
			__rbTree_RotateRight(tree, node->pParent);
	}
	else
	{}

	return __rbTree_CheckDeleteNodeCase3(tree, node);
}

static void __rbTree_CheckDeleteNodeCase1(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	/* let's operate with diferent cases */
	if (_rbTree_NodeIsRoot(node))
	{_DEBUG("DEL: case 1");
		return;
	}
	else			// Right child version
	{
		__rbTree_CheckDeleteNodeCase2(tree, node);
	}
}

static int _rbTree_DeleteNode(AMCStaticRbTree_st *tree, AMCStaticRbNode_st *node)
{
	BOOL nodeHasChild, nodeHasTwoChildren, nodeHasLeftChild, nodeHasRightChild;
	_rbTree_ReadNodeStatus(node, &nodeHasChild, &nodeHasTwoChildren, &nodeHasLeftChild, &nodeHasRightChild);

	if (FALSE == nodeHasChild)
	{_DEBUG("DEL: node %ld is leaf", node->id);
		_rbTree_DeleteLeaf(tree, node);
		return 0;
	}
	else if (nodeHasTwoChildren)
	{_DEBUG("DEL: node %ld has 2 children", node->id);
		AMCStaticRbNode_st *smallestRightChild = _rbTree_FindMinLeaf(node->pRight);

		/* replace with smallest node */
		node->id = smallestRightChild->id;
		_rbTree_NodeContentCopy(tree, node, (void*)(_ULONG_CVT(smallestRightChild) + sizeof(*smallestRightChild)));

		/* start operation with the smallest one */
		return _rbTree_DeleteNode(tree, smallestRightChild);
	}
	else
	{
		AMCStaticRbNode_st *child = (node->pLeft) ? node->pLeft : node->pRight;

		if (_IS_RED(node))		// if node is red, both parent and child nodes are black. We could simply replace it with its child
		{_DEBUG("DEL: node %ld red", node->id);
			__rbTree_DeleteNodeAndReonnectWith(tree, node, child);
			_rbTree_FreeNode(node);
			return 0;
		}
		else if (_IS_RED(child))		// if node is black but its child is red. we could repace it with its child and refill the child as black
		{_DEBUG("DEL: node %ld black but child red", node->id);
			__rbTree_DeleteNodeAndReonnectWith(tree, node, child);
			_SET_BLACK(child);
			_rbTree_FreeNode(node);
			return 0;
		}
		else				// both node and its child are all black. This is the most complex one.
		{
			/* first of all, we should replace node with child */
			__rbTree_DeleteNodeAndReonnectWith(tree, node, child);
			_rbTree_FreeNode(node);		/* "node" val is useless */

			__rbTree_CheckDeleteNodeCase1(tree, child);
			return 0;
		}
	}
}



static int _rbTree_Delete(AMCStaticRbTree_st *tree, const unsigned long id)
{
	AMCStaticRbNode_st *node = _rbTree_FindNode(tree, id);

	if (NULL == node)
	{
		return AMCTreeError_IdNotFound;
	}

	return _rbTree_DeleteNode(tree, node);
}


#endif



/********************/
#define __PRIVATE_DUMP_FUNCTIONS
#ifdef __PRIVATE_DUMP_FUNCTIONS

static void _rbTree_DumpNode(const AMCStaticRbNode_st *node, unsigned long tabCount)
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
	if (_IS_BLACK(node))
	{
		printf("Node \033[0;43mBlack\033[0m (%ld), ", node->id);
	}
	else
	{
		printf("Node \033[0;41mRed\033[0m (%ld), ", node->id);
	}

	/* left */
	if (node->pLeft)
	{
		if (_IS_BLACK(node->pLeft))
			printf("<(\033[0;43m%ld\033[0m), ", node->pLeft->id);
		else
			printf("<(\033[0;41m%ld\033[0m), ", node->pLeft->id);;
	}
	else
	{
		printf("<(_), ");
	}

	/* right */
	if (node->pRight)
	{
		if (_IS_BLACK(node->pRight))
			printf("(\033[0;43m%ld\033[0m)>\n", node->pRight->id);
		else
			printf("(\033[0;41m%ld\033[0m)>\n", node->pRight->id);
	}
	else
	{
		printf("(_)>\n");
	}

	/* check child */
	if (node->pLeft)
	{
		if (node != node->pLeft->pParent)
		{
			printf("WARNING (\033[0;41m%ld <- %ld\033[0m)>\n", node->pLeft->id, node->pLeft->pParent->id);exit(1);
		}
	}
	if (node->pRight)
	{
		if (node != node->pRight->pParent)
		{
			printf("WARNING (\033[0;41m%ld <- %ld\033[0m)>\n", node->pRight->id, node->pRight->pParent->id);exit(1);
		}
	}

	/* call self */
	if (node->pLeft)
	{
		_rbTree_DumpNode(node->pLeft, tabCount + 1);
	}

	if (node->pRight)
	{
		_rbTree_DumpNode(node->pRight, tabCount + 1);
	}

	return;
}


static void _rbTree_Dump(const AMCStaticRbTree_st *tree)
{
	if (NULL == tree->entry)
	{
		printf("Tree 0x%08lx, size: 0\n", _ULONG_CVT(tree));
		return;
	}
	else if (1 == tree->nodeCount)
	{
		printf("Tree 0x%08lx, size: %ld\n\t", _ULONG_CVT(tree), tree->nodeCount);
		if (_IS_BLACK(tree->entry))
		{
			printf("Node \033[0;43mBlack\033[0m (%ld)\n", tree->entry->id);
		}
		else
		{
			printf("Node \033[0;41mRed\033[0m (%ld)\n", tree->entry->id);
		}
	}
	else
	{
		printf("Tree 0x%08lx, size: %ld\n", _ULONG_CVT(tree), tree->nodeCount);
		_rbTree_DumpNode(tree->entry, 1);
		return;
	}
}


#endif


/********************/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES


struct AMCStaticRedBlackTree *AMCStaticRbTree_New(unsigned long contentSize, unsigned long expectedSize, int *errorOut)
{
	AMCStaticRbTree_st *ret = malloc(sizeof(*ret));
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
	ret->memPool = AMCMemPool_Create(sizeof(AMCStaticRbTree_st) + contentSize, expectedSize, expectedSize, FALSE);
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


int AMCStaticRbTree_Destroy(struct AMCStaticRedBlackTree *tree)
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


int AMCStaticRbTree_Insert(struct AMCStaticRedBlackTree *tree, unsigned long id, const void *content)
{
	BOOL isDuplicated;
	AMCStaticRbNode_st *node;
	int ret = 0;

	if (NULL == tree)
	{
		return AMCTreeError_ParaError;
	}

	if (NULL == tree->memPool)
	{
		return AMCTreeError_TreeInvalid;
	}


	_lock_Write_RbTree(tree);

	node = _rbTree_Insert(tree, id, &isDuplicated);
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
			_rbTree_NodeContentCopy(tree, node, content);
		}
		else
		{}
	}

	_unlock_Write_RbTree(tree);

	return ret;
}


int AMCStaticRbTree_ForceInsert(struct AMCStaticRedBlackTree *tree, unsigned long id, const void *content)
{
	BOOL isDuplicated;
	AMCStaticRbNode_st *node;
	int ret = 0;

	if (NULL == tree)
	{
		return AMCTreeError_ParaError;
	}

	if (NULL == tree->memPool)
	{
		return AMCTreeError_TreeInvalid;
	}


	_lock_Write_RbTree(tree);

	node = _rbTree_Insert(tree, id, &isDuplicated);
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
		_rbTree_NodeContentCopy(tree, node, content);
	}
	else
	{}
	
	_unlock_Write_RbTree(tree);

	return ret;
}


int AMCStaticRbTree_Delete(struct AMCStaticRedBlackTree *tree, unsigned long id)
{
	int ret;

	_lock_Read_RbTree(tree);
	ret = _rbTree_Delete(tree, id);
	_unlock_Read_RbTree(tree);

	return ret;
}









void AMCStaticRbTree_DebugStdout(struct AMCStaticRedBlackTree *tree)
{
	if (NULL == tree)
	{
		return;
	}

	_lock_Read_RbTree(tree);
	_rbTree_Dump(tree);
	_unlock_Read_RbTree(tree);

	return;
}



#endif


