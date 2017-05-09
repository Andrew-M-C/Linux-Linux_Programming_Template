/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCBinSearchTree.h
	Description: 	
			This file provide simple sinary search tree interface.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-15: File created

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/
#ifndef	_AMC_BIN_SEARCH_TREE_H
#define	_AMC_BIN_SEARCH_TREE_H

#include "AMCMemPool.h"

#ifndef NULL
#define NULL	((void*)0)
#endif

enum {
	AMCBsTreeError_Success = 0,
	AMCBsTreeError_SysCallError,
	AMCBsTreeError_ParaError,
	AMCBsTreeError_MemPoolError,
	AMCBsTreeError_BsTreeInvalid,
	AMCBsTreeError_IdNotFound,
	AMCBsTreeError_IdExists,
	AMCBsTreeError_BufferTooSmall,
	AMCBsTreeError_UnknownError
};

struct AMCStaticBinSearchTree;		/* function prefix: AMCStaticBsTree_ */
struct AMCDynamicBinSearchTree;		/* function prefix: AMCDynamicBsTree_ */

/* static ones */
struct AMCStaticBinSearchTree *AMCStaticBsTree_New(unsigned long contentSize, unsigned long expectedSize, int *errorOut);
int AMCStaticBsTree_Destroy(struct AMCStaticBinSearchTree *tree);

int AMCStaticBsTree_Insert(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *content);
int AMCStaticBsTree_ForceInsert(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *content);
int AMCStaticBsTree_Delete(struct AMCStaticBinSearchTree *tree, unsigned long id);

unsigned long AMCStaticBsTree_NodeSize(struct AMCStaticBinSearchTree *tree);
void *AMCStaticBsTree_GetContent(struct AMCStaticBinSearchTree *tree, unsigned long id, int *errorOut);		/* not thread safe */

int AMCStaticBsTree_ReadContent(struct AMCStaticBinSearchTree *tree, unsigned long id, void *contentOut, unsigned long bufferSize);		/* thread safe */
int AMCStaticBsTree_WriteContent(struct AMCStaticBinSearchTree *tree, unsigned long id, const void *contentIn, unsigned long bufferSize);	/* thread safe */

void AMCStaticBsTree_DebugStdout(struct AMCStaticBinSearchTree *tree);


/* dynamic ones */
#if 0
struct AMCDynamicBinSearchTree *AMCDynamicBsTree_New(int *errorOut);
int AMCDynamicBsTree_Destroy(struct AMCDynamicBinSearchTree *tree);

int AMCDynamicBsTree_Insert(struct AMCDynamicBinSearchTree *tree, unsigned long id, const void *content, unsigned long size);
int AMCDynamicBsTree_ForceInsert(struct AMCDynamicBinSearchTree *tree, unsigned long id, const void *content, unsigned long size);
int AMCDynamicBsTree_Delete(struct AMCDynamicBinSearchTree *tree, unsigned long id);

unsigned long AMCDynamicBsTree_NodeSize(struct AMCDynamicBinSearchTree *tree, unsigned long id);
void *AMCDynamicBsTree_GetContent(struct AMCDynamicBinSearchTree *tree, unsigned long id, int *errorOut);
#endif


#endif
