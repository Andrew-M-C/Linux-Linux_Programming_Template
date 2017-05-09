/*******************************************************************************
	Copyright (C), 2011-2013, Andrew Min Chang

	File name: 	AMCList.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide imlementions described in AMCList.h
			
	History:
		2012-09-23: Create File

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#define	DEBUG
#define	CFG_LIB_ERROUT
#define	CFG_LIB_STDOUT
#define	CFG_LIB_MALLOC
#define	CFG_LIB_ERRNO
#include "AMCCommonLib.h"

#include "AMCList.h"


/***********************************************************************
 * Defination for listInfoX parameters and their access functions
 *
 * listInfo01:
 * 		[--:00] - last accesed node pointer
 * listInfo02: 
 * 		[31:00] - Current list size (node count)
 * listInfo03:
 * 		[31:00] - last accesed node index
 * listInfo04:
 * 		[31:00] - node item size
 */
#define	LAST_ACCESSED_NODE_POINTER	listInfo01
#define	CURRENT_LIST_SIZE				listInfo02
#define	LAST_ACCESSED_NODE_INDEX		listInfo03
#define	NODE_ITEM_SIZE				listInfo04

size_t _nodeSize (AMCList_st *list)
{
	return (size_t)(list->NODE_ITEM_SIZE);
}

size_t _maximumNodeSize ()
{
	return 0x0000FFFF;
}

size_t _maximumNodeCount()
{
	return 0x0FFFFFFF;
}

BOOL _setNodesize (AMCList_st *list, size_t size)
{
	if (size > _maximumNodeSize())
	{
		return FALSE;
	}
	else
	{
		list->NODE_ITEM_SIZE = (uint32_t)size;
		return TRUE;
	}
}

BOOL _setNodeCount(AMCList_st *list, size_t count)
{
	if (count > _maximumNodeCount())
	{
		return FALSE;
	}
	else
	{
		list->CURRENT_LIST_SIZE = (uint32_t)count;
		return TRUE;
	}
}

size_t _nodeCount(AMCList_st *list)
{
	return (size_t)(list->CURRENT_LIST_SIZE);
}

size_t _lastAccessedNodeIndex(AMCList_st *list)
{
	return (size_t)(list->LAST_ACCESSED_NODE_INDEX);
}

AMCNode_st *_lastAccessedNodePointer(AMCList_st *list)
{
	return (list->LAST_ACCESSED_NODE_POINTER);
}

void _setLastAccessedNode(AMCList_st *list, AMCNode_st* nodeItem, size_t index)
{
	list->LAST_ACCESSED_NODE_POINTER = nodeItem;
	list->LAST_ACCESSED_NODE_INDEX = index;
}

#undef	LAST_ACCESSED_NODE_POINTER
#undef	CURRENT_LIST_SIZE
#undef	LAST_ACCESSED_NODE_INDEX
#undef	NODE_ITEM_SIZE



/***********************************************************************
 *    Private Functions
 */
/**/
void _listFree(AMCList_st *list)
{
#ifdef	DEBUG
	size_t tmp = 0;
#endif
	if (!list)
	{
		return;
	}

	/* free nodes */
	AMCNode_st *pNodeNext = list->pFirstNode;
	AMCNode_st *pNodeTemp;
	while(pNodeNext)
	{
		pNodeTemp = pNodeNext;
		pNodeNext = pNodeTemp->pNext;
		free(pNodeTemp);
#ifdef	DEBUG
		tmp++;
#endif
	}

	DB_LOG("Deleted %d nodes for (0x%08X).\n", tmp, (uint32_t)list);

	/* free header */
	free(list);
}


/**/
AMCList_st *_listCreate(size_t nodeSize, size_t count)
{
	size_t tmp;
	AMCList_st *list;
	AMCNode_st *pNext, *pTemp;

	/* check parameter */
	if ((count > _maximumNodeCount()) ||
		(nodeSize > _maximumNodeSize()))
	{
		DB(LOG("Parameter error!\n"));
		return NULL;
	}

	/* generate list structure */
	list = malloc(sizeof(AMCList_st));
	if (!list)
	{
		DB(SYS_PERROR());
		return NULL;
	}

	/* initialize header */
	_setNodesize(list, nodeSize);
	_setNodeCount(list, count);
	list->pFirstNode = NULL;
	pNext = (AMCNode_st*)list;

	/* malloc pointers */
	for (tmp = 0; tmp < count; tmp++)
	{
		//LOG("tmp = %d of %d\n", tmp, count);
		pTemp = malloc(nodeSize);
		if (!pTemp)
		{
			_listFree(list);
			DB(SYS_PERROR());
			//DB(LOG("%d list member required but only %d allocated.\n", count, tmp));
			return NULL;
		}
		else
		{
			memset(pTemp, 0, nodeSize);
			pNext->pNext = pTemp;
			pNext = pTemp;
			//DB(printf("Add 0x%08X\n", (uint32_t)pTemp));
		}
	}

	/* set last accessed node */
	_setLastAccessedNode(list, list->pFirstNode, 0);

	/* return */
	return list;
}



AMCNode_st *_getNode(AMCList_st *list, size_t index)
{
	size_t buffIndex;
	AMCNode_st *pNode;

	if (!list)
	{
		return NULL;
	}

	if (0 == index)
	{
		return list->pFirstNode;
	}
	else if (index > _nodeCount(list) - 1)
	{
		return NULL;
	}
	else
	{}

	buffIndex = _lastAccessedNodeIndex(list);
	if (index >= buffIndex)
	{
		/* start from cache */
		for (pNode = _lastAccessedNodePointer(list);
				buffIndex < index;
				buffIndex ++)
		{
			pNode = pNode->pNext;
		}
	}
	else
	{
		for (pNode = (AMCNode_st*)list, buffIndex = 0;
				buffIndex < index;
				buffIndex ++)
		{
			pNode = pNode->pNext;
		}
	}

	_setLastAccessedNode(list, pNode, index);

	return pNode;
}

void _insertNode(AMCList_st *list, AMCNode_st *node, size_t atIndex)
{
	AMCNode_st *pNodeAhead, *pNodeAfter;

	/* parameter check */
	if ((!list) || (!node))
	{
		return;
	}

	/* index 0 check */
	if (0 == atIndex)
	{
		pNodeAfter = list->pFirstNode;
		list->pFirstNode = node;
		node->pNext = pNodeAfter;

		_setNodeCount(list, _nodeCount(list) + 1);
		_setLastAccessedNode(list, 
					_lastAccessedNodePointer(list), 
					_lastAccessedNodeIndex(list) + 1);
		return;
	}
	else if (atIndex > _nodeCount(list))
	{
		atIndex = _nodeCount(list);
	}

	/* fetch node where to insert */
	pNodeAhead = _getNode(list, atIndex - 1);
	pNodeAfter = pNodeAhead->pNext;
	pNodeAhead->pNext = node;
	node->pNext = pNodeAfter;
	_setNodeCount(list, _nodeCount(list) + 1);
	_setLastAccessedNode(list, node, atIndex);
}

void _deleteNode(AMCList_st *list, size_t atIndex)
{
	AMCNode_st *pNodeAhead, *pNodeToDelete;

	/* parameter check */
	if (!list)
	{
		return;
	}

	if (0 == _nodeCount(list))
	{
		return;
	}
	else if (atIndex > _nodeCount(list) - 1)
	{
		return;
	}
	else
	{}

	/* index 0 check */
	if (0 == atIndex)
	{
		pNodeToDelete = list->pFirstNode;
		list->pFirstNode = pNodeToDelete->pNext;
		_setNodeCount(list, _nodeCount(list) - 1);

		//printf("Delete first: <0x%08X>\n", (uint32_t)pNodeToDelete);
		if (0 == _lastAccessedNodeIndex(list))
		{
			_setLastAccessedNode(list, list->pFirstNode, 0);
		}
		else
		{
			_setLastAccessedNode(list, 
								_lastAccessedNodePointer(list), 
								_lastAccessedNodeIndex(list) - 1);
		}

		free(pNodeToDelete);
		return;
	}
	/* normal operation */
	else
	{
		pNodeAhead = _getNode(list, atIndex - 1);
		pNodeToDelete = pNodeAhead->pNext;
		pNodeAhead->pNext = pNodeToDelete->pNext;
		//printf("Delete: <0x%08X>\n", (uint32_t)pNodeToDelete);
		_setNodeCount(list, _nodeCount(list) - 1);
		if (atIndex < _lastAccessedNodeIndex(list))
		{
			_setLastAccessedNode(list, 
								_lastAccessedNodePointer(list), 
								_lastAccessedNodeIndex(list) - 1);
		}
		free(pNodeToDelete);
		return;
	}
}


/***********************************************************************
 *    Public Function Interfaces
 */
AMCList_st *AMCListCreate(size_t nodeSize, size_t count)
{
	//DB_LOG("Count = %d; Size = %d\n", count, nodeSize);
	return _listCreate(nodeSize, count);
}

void AMCListFree(AMCList_st *list)
{
	return _listFree(list);
}

size_t AMCListGetLength(AMCList_st *list)
{
	return _nodeCount(list);
}

AMCNode_st *AMCListGetNode(AMCList_st *list, size_t index)
{
	return _getNode(list, index);
}

void AMCListInsertNode(AMCList_st *list, AMCNode_st *node, size_t atIndex)
{
	return _insertNode(list, node, atIndex);
}

void AMCListPrintNodeAddresses(AMCList_st *list)
{
	size_t tmp, length;
	AMCNode_st *pNode = (AMCNode_st*)list;

	if (!pNode)
	{
		errPrintf("Empty pointer.\n");
		return;
	}

	printf("AMCList_st <0x%08X> (Len: %d):\n", (uint32_t)list, _nodeCount(list));
	length = _nodeCount(list);
	for (tmp = 0; tmp < length; tmp++)
	{
		pNode = pNode->pNext;
		//printf("[%03d] AMCNode_st <0x%08X>  -> (0x%08X)\n", tmp, (uint32_t)pNode, (uint32_t)(pNode->pNext));
		printf("[%03d] AMCNode_st <0x%08X>\n", tmp, (uint32_t)pNode);
	}
}

void AMCListDeleteNode(AMCList_st *list, size_t atIndex)
{
	return _deleteNode(list, atIndex);
}

