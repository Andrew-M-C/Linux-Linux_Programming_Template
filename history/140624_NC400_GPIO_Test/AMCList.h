/*******************************************************************************
	Copyright (C), 2011-2013, Andrew Min Chang

	File name: 	AMCList.h
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide interfaces for list programming. User may create a 
		new structure with the node structure provided at the FIRST member, and
		pass them to interfaces described in this file.
			
	History:
		2012-09-23: Create File

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef	_AMC_LIST_H
#define	_AMC_LIST_H

#include "AMCDataTypes.h"

typedef struct AMCNode{
	struct AMCNode* pNext;
}AMCNode_st;

typedef struct {
	AMCNode_st	*pFirstNode;
	AMCNode_st	*listInfo01;		/* Please do NOT modify this */
	uint32_t	listInfo02;		/* Please do NOT modify this */
	uint32_t	listInfo03;		/* Please do NOT modify this */
	uint32_t	listInfo04;		/* Please do NOT modify this */
}AMCList_st;


AMCList_st *AMCListCreate(size_t nodeSize, size_t count);
void AMCListFree(AMCList_st *list);

size_t AMCListGetLength(AMCList_st *list);
AMCNode_st *AMCListGetNode(AMCList_st *list, size_t index);
void AMCListInsertNode(AMCList_st *list, AMCNode_st *node, size_t atIndex);
void AMCListDeleteNode(AMCList_st *list, size_t atIndex);
void AMCListPrintNodeAddresses(AMCList_st *list);	/* debug use */

#endif	/* end of file */
