/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCTree.h
	Description: 	
			This file provide simple tree interface.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-15: File "AMCBinSearchTree.h" created
		2015-01-16: File "AMCRedBlackTree.h" created
		2015-01-19: File "AMCTree.h" created and combines previous tow together.

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_TREE_H
#define _AMC_TREE_H

#include "AMCMemPool.h"

#define CFG_DEFAULT_EXPECTED_NODE_COUNT		16

#ifndef NULL
#define NULL	((void*)0)
#endif

enum {
	AMCTreeError_Success = 0,
	AMCTreeError_SysCallError,
	AMCTreeError_ParaError,
	AMCTreeError_MemPoolError,
	AMCTreeError_TreeInvalid,
	AMCTreeError_IdNotFound,
	AMCTreeError_IdExists,
	AMCTreeError_BufferTooSmall,
	AMCTreeError_UnknownError
};

/**********/
/* static basic binary search tree */
struct AMCStaticBinSearchTree;		/* function prefix: AMCStaticBsTree_ */

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


/**********/
/* static red-black search tree */
struct AMCStaticRedBlackTree;		/* function prefix: AMCStaticRbTree_ */


struct AMCStaticRedBlackTree *AMCStaticRbTree_New(unsigned long contentSize, unsigned long expectedSize, int *errorOut);
int AMCStaticRbTree_Destroy(struct AMCStaticRedBlackTree *tree);

int AMCStaticRbTree_Insert(struct AMCStaticRedBlackTree *tree, unsigned long id, const void *content);
int AMCStaticRbTree_ForceInsert(struct AMCStaticRedBlackTree *tree, unsigned long id, const void *content);
int AMCStaticRbTree_Delete(struct AMCStaticRedBlackTree *tree, unsigned long id);

unsigned long AMCStaticRbTree_NodeSize(struct AMCStaticRedBlackTree *tree);
void *AMCStaticRbTree_GetContent(struct AMCStaticRedBlackTree *tree, unsigned long id, int *errorOut);		/* not thread safe */

int AMCStaticRbTree_ReadContent(struct AMCStaticRedBlackTree *tree, unsigned long id, void *contentOut, unsigned long bufferSize);		/* thread safe */
int AMCStaticRbTree_WriteContent(struct AMCStaticRedBlackTree *tree, unsigned long id, const void *contentIn, unsigned long bufferSize);	/* thread safe */

void AMCStaticRbTree_DebugStdout(struct AMCStaticRedBlackTree *tree);



#endif


