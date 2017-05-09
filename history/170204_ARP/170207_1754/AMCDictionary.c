/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCDictionary.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file implements mutable associative array support.
			
	History:
		2017-02-04: File created as AMCDictionary.c

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/********/
#define __HEADERS
#ifdef __HEADERS

#include "AMCDictionary.h"
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

typedef enum {
	_BLACK = 0,
	_RED = 1
} _color_t;


struct AMCDictNode {
	struct AMCDictNode  *parent;
	struct AMCDictNode  *left;
	struct AMCDictNode  *right;
	_color_t             color;
	char                 key[CFG_AMC_DICT_KEY_LEN_MAX + 1];
	void                *value;
};


struct AMCDict {
	struct AMCDictNode  *children;
	pthread_rwlock_t     rw_lock;
	size_t               count;
};


#define _DB(fmt, args...)		AMCPrintf("<%s, %ld> "fmt, __FILE__, __LINE__, ##args)
#define _LOCK_DICT_READ(dict)			do{/*_DB("-- LOCK --");*/ pthread_rwlock_rdlock(&((dict)->rw_lock));}while(0)
#define _UNLOCK_DICT_READ(dict)			do{/*_DB("--UNLOCK--");*/ pthread_rwlock_unlock(&((dict)->rw_lock));}while(0)
#define _LOCK_DICT_WRITE(dict)			do{/*_DB("-- LOCK --");*/ pthread_rwlock_wrlock(&((dict)->rw_lock));}while(0)
#define _UNLOCK_DICT_WRITE(dict)		do{/*_DB("--UNLOCK--");*/ pthread_rwlock_unlock(&((dict)->rw_lock));}while(0)

static struct AMCMemPool *g_item_mem_pool = NULL;
static size_t g_item_init_size = CFG_AMC_DICT_DEFAULT_INITIAL_ITEM_COUNT;
static size_t g_item_inc_size = CFG_AMC_DICT_DEFAULT_GROW_ITEM_COUNT;

static struct AMCMemPool *g_dict_mem_pool = NULL;
static size_t g_dict_init_size = CFG_AMC_DICT_DEFAULT_INITIAL_DICT_COUNT;
static size_t g_dict_inc_size = CFG_AMC_DICT_DEFAULT_GROW_DICT_COUNT;

#endif


/********/
#define __SIMPLE_STATIC_OPERATIONS
#ifdef __SIMPLE_STATIC_OPERATIONS

#define _IS_RED(node)		((node)->color)
#define _IS_BLACK(node)	(_BLACK == ((node)->color))
#define _SET_RED(node)		if (node){((node)->color = _RED);}do{}while(0)
#define _SET_BLACK(node)	if (node){((node)->color = _BLACK);}do{}while(0)


/* --------------------_compare_strings----------------------- */
static inline int _compare_strings(const char *strA, const char *strB)
{
	return strcmp(strA, strB);
}


/* --------------------_is_less_than----------------------- */
static inline BOOL _is_less_than(int cmpResult)
{
	return (cmpResult < 0);
}


/* --------------------_is_greater_than----------------------- */
static inline BOOL _is_greater_than(int cmpResult)
{
	return (cmpResult > 0);
}


/* --------------------_is_equal_to----------------------- */
static inline BOOL _is_equal_to(int cmpResult)
{
	return (0 == cmpResult);
}


#endif


/********/
#define __RED_BLACK_TREE_OPERATIONS
#ifdef __RED_BLACK_TREE_OPERATIONS

/* --------------------_rb_node_is_root----------------------- */
static inline BOOL _rb_node_is_root(const struct AMCDictNode *node)
{
	return (NULL == ((node)->parent));
}


/* --------------------_rb_brother_node----------------------- */
static struct AMCDictNode *_rb_brother_node(struct AMCDictNode *node)
{
	if (_rb_node_is_root(node)) {
		return NULL;
	}
	else {
		if (node == node->parent->left) {
			return node->parent->right;
		}
		else {
			return node->parent->left;
		}
	}
}


/* --------------------_rb_alloc_node----------------------- */
static struct AMCDictNode *_rb_alloc_node(const char *key, void *value, _color_t color)
{
	struct AMCDictNode *ret = AMCMemPool_Alloc(g_item_mem_pool);
	if (ret)
	{
		ret->parent = NULL;
		ret->left  = NULL;
		ret->right = NULL;
		ret->color = color;
		ret->value = value;
	
		strncpy(ret->key, key, CFG_AMC_DICT_KEY_LEN_MAX);
	}
	return ret;
}


/* --------------------_rb_free_node----------------------- */
static inline int _rb_free_node(struct AMCDictNode *node, BOOL freeObject)
{
	if (freeObject) {
		free(node->value);
	}
	memset(node, 0, sizeof(*node));
	return AMCMemPool_Free(node);
}


/* --------------------_rb_find_min_leaf----------------------- */
static struct AMCDictNode *_rb_find_min_leaf(struct AMCDictNode *node)
{
	struct AMCDictNode *ret = node;

	while(ret->left)
	{
		ret = ret->left;
	}
	
	return ret;
}


/* --------------------_rb_find_node----------------------- */
static struct AMCDictNode *_rb_find_node(struct AMCDict *dict, const char *key)
{
	struct AMCDictNode *ret = dict->children;
	struct AMCDictNode *parent = NULL;
	BOOL isFound = FALSE;
	int cmpResult = 0;
	
	if (NULL == ret) {
		return NULL;
	}

	/* let's start searching! */
	do
	{
		cmpResult = _compare_strings(key, ret->key);

		if (_is_less_than(cmpResult)) {
			parent = ret;
			ret = ret->left;
		}
		else if (_is_greater_than(cmpResult)) {
			parent = ret;
			ret = ret->right;
		}
		else {
			isFound = TRUE;
		}
	}
	while((FALSE == isFound) && ret);

	return ret;
}


/* --------------------__rb_rotate_left----------------------- */
static void __rb_rotate_left(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *prevRight = node->right;
	struct AMCDictNode *prevRightLeft = node->right->left;
	//_DEBUG("Rotate left %ld", node->id);
	//_DEBUG("prevRight %ld, prevRightLeft %ld", prevRight ? prevRight->id : -1, prevRightLeft ? prevRightLeft->id : -1);

	if (_rb_node_is_root(node))
	{
		dict->children = prevRight;
		
		node->parent = prevRight;
		node->right = prevRightLeft;
		if (prevRightLeft)
		{
			prevRightLeft->parent = node;
		}
		
		prevRight->left = node;
		prevRight->parent = NULL;
	}
	else
	{
		struct AMCDictNode *parent = node->parent;

		//_rbTree_DumpNode(parent, 5);
		if (node == node->parent->left)
		{
			parent->left = prevRight;
		}
		else
		{
			parent->right = prevRight;
		}

		node->parent = prevRight;
		node->right = prevRightLeft;
		if (prevRightLeft)
		{
			prevRightLeft->parent = node;
		}

		prevRight->left = node;
		prevRight->parent = parent;

		//_rbTree_DumpNode(parent, 5);
	}
	
	return;
}


/* --------------------__rb_rotate_right----------------------- */
static void __rb_rotate_right(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *prevLeft = node->left;
	struct AMCDictNode *prevLeftRight = node->left->right;
	_DEBUG("Rotate right %ld", node->id);

	if (_rbTree_NodeIsRoot(node))
	{
		dict->children = prevLeft;
		
		node->parent = prevLeft;
		node->left = prevLeftRight;
		if (prevLeftRight)
		{
			prevLeftRight->parent = node;
		}
		
		prevLeft->right = node;
		prevLeft->parent = NULL;
	}
	else
	{
		struct AMCDictNode *parent = node->parent;
		if (node == node->parent->left)
		{
			parent->left = prevLeft;
		}
		else
		{
			parent->right = prevLeft;
		}

		node->parent = prevLeft;
		node->left = prevLeftRight;
		if (prevLeftRight)
		{
			prevLeftRight->parent = node;
		}

		prevLeft->right = node;
		prevLeft->parent = parent;
	}
	
	return;
}


/* --------------------__rb_check_insert----------------------- */
static void __rb_check_insert(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *uncleNode = node->parent ? _rb_brother_node(node->parent) : NULL;

	if (_rb_node_is_root(node))								// root node. This is impossible because it is done previously
	{_DEBUG("MARK (%s) case 1", node->key);
		_SET_BLACK(node);
	}
	else if (_IS_BLACK(node->parent))	// parent black. Nothing addition needed
	{_DEBUG("MARK (%s) case 2", node->key);
		// nop
	}
	else if (uncleNode &&
			_IS_RED(uncleNode))	// parent and uncle node are both red
	{_DEBUG("MARK (%s) case 3, uncle: %s, parent: %s", node->key, uncleNode->key, node->parent->key);
		_SET_BLACK(node->parent);
		_SET_BLACK(uncleNode);
		_SET_RED(node->parent->parent);
		__rb_check_insert(dict, node->parent->parent);
	}
	else				// parent red, meanwhile uncle is black or NULL 
	{
		// Step 1
		if ((node == node->parent->right) &&
			(node->parent == node->parent->parent->left))	// node is its parent's left child, AND parent is grandparent's right child. left-rotation needed.
		{_DEBUG("MARK (%s) case 4", node->key);
			__rb_rotate_left(dict, node->parent);
			node = node->left;
			_DEBUG("MARK test:");
			_rbTree_Dump(dict);
		}
		else if ((node == node->parent->left) &&
				(node->parent == node->parent->parent->right))
		{_DEBUG("MARK (%s) case 4", node->key);
			__rb_rotate_right(dict, node->parent);
			node = node->right;
			_DEBUG("MARK test:");
			_rbTree_Dump(dict);
		}
		else
		{}

		// Step 2
		_SET_BLACK(node->parent);
		_SET_RED(node->parent->parent);

		if ((node == node->parent->left) &&
			(node->parent == node->parent->parent->left))
		{_DEBUG("MARK (%s) case 5", node->key);
			__rb_rotate_right(dict, node->parent->parent);
		}
		else
		{_DEBUG("MARK (%s) case 5", node->key);
			__rb_rotate_left(dict, node->parent->parent);
			_DEBUG("MARK test:");
			_rbTree_Dump(dict);
		}
	}
	return;
}


/* --------------------__rb_check_insert----------------------- */
static AMCDictErrno_st _rb_insert(struct AMCDict *dict, const char *key, void *value, BOOL replaceWhenConflict, BOOL shouldFree, BOOL *isDuplicated)
{
	struct AMCDictNode *node = dict->children;
	struct AMCDictNode *newNode = NULL;
	struct AMCDictNode *parent = dict->children;
	AMCDictErrno_st error = {0, 0};
	int cmpResult = 0;

	*isDuplicated = FALSE;
	
	if (NULL == dict->children)	// root
	{
		newNode = _rb_alloc_node(key, value, _BLACK);
		if (NULL == newNode) {
			error = _make_error(AMC_DICT_MEMPOOL_ERROR);
		}
		else {
			dict->children = ret;
			dict->count ++;
		}
	
		return error;
	}

	/* insert a node by binary search tree method */
	do
	{
		cmpResult _compare_strings(key, node->key);
		
		if (_is_less_than(cmpResult))
		{
			if (NULL == node->left)		/* found a slot */
			{_DEBUG("Found parent %s", node->key);
				newNode = _rb_alloc_node(key, value, _red);
				if (NULL == newNode) {
					error = _make_error(AMC_DICT_MEMPOOL_ERROR);
					goto ENDS;
				}

				node->left = newNode;
				newNode->parent = node;
				dict->count ++;
			}
			else
			{
				parent = node;
				node = node->left;
			}
		}
		else if (_is_greater_than(cmpResult))
		{
			if (NULL == node->right)		/* found a slot */
			{_DEBUG("Found parent %s", node->key);
				newNode = _rb_alloc_node(key, value, _red);
				if (NULL == newNode) {
					error = _make_error(AMC_DICT_MEMPOOL_ERROR);
					goto ENDS;
				}

				node->right = newNode;
				newNode->parent = node;
				newNode->count ++;
			}
			else
			{
				parent = node;
				node = node->right;
			}
		}
		else
		{
			*isDuplicated = TRUE;
			newNode = node;

			if (replaceWhenConflict)
			{
				if ((node->value != value)
					&& shouldFree)
				{
					free(node->value);
					node->value = NULL;
				}

				node->value = value;
			}
			else
			{
				error = _make_error(AMC_DICT_OBJECT_EXISTS);
			}
		}
		
	}
	while(NULL == newNode);

	/* special red-black tree operation */
	if ((FALSE == *isDuplicated) && newNode) {
		__rb_check_insert(dict, newNode);
	}

ENDS:
	/* ends */
	return error;
}


/* --------------------_rb_delete_leaf----------------------- */
static AMCDictErrno_st _rb_delete_leaf(struct AMCDict *dict, struct AMCDictNode *node, BOOL freeObject)
{
	AMCDictErrno_st error = {0, 0};

	if (_rb_node_is_root(node))
	{
		dict->children = NULL;
		dict->count = 0;
		_rb_free_node(node, freeObject);
	}
	else
	{_DEBUG("DEL: %s <- %s", node->key, node->parent->key)
		if (node == node->parent->left)
		{
			node->parent->left= NULL;
		}
		else
		{
			node->parent->right = NULL;
		}

		dict->count --;
		_rb_free_node(node, freeObject);
	}

	return;
}


/* --------------------_rb_read_node_status----------------------- */
static inline void _rb_read_node_status(const struct AMCDictNode *node, BOOL *hasChild, BOOL *hasTwoChild, BOOL *hasLeftChild, BOOL *hasRightChild)
{
	if (node->left)
	{
		if (node->right)
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
		if (node->right)
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


/* --------------------__rb_delete_node_and_reconnect_with----------------------- */
static void __rb_delete_node_and_reconnect_with(struct AMCDict *dict, struct AMCDictNode *nodeToDel, struct AMCDictNode *nodeToReplace)
{
	if (_rb_node_is_root(nodeToDel))
	{
		dict->count --;
		dict->entry = nodeToReplace;
		nodeToReplace->parent = NULL;
	}
	else
	{
		dict->count --;
		nodeToReplace->parent = nodeToDel->parent;

		if (nodeToDel == nodeToDel->parent->left) {
			nodeToDel->parent->left = nodeToReplace;
		}
		else {
			nodeToDel->parent->right = nodeToReplace;
		}
	}

	return;
}


/* --------------------__rb_check_delete_node_case_6----------------------- */
static void __rb_check_delete_node_case_6(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *sibling = _rb_brother_node(node->parent);

	sibling->color = node->parent->color;
	_SET_BLACK(node->parent);

	if (node == node->parent->left) {
		_SET_BLACK(sibling->right);
		__rb_rotate_left(dict, node->parent);
	}
	else {
		_SET_BLACK(sibling->left);
		__rb_rotate_right(dict, node->parent);
	}

	return;
}


/* --------------------__rb_check_delete_node_case_5----------------------- */
static void __rb_check_delete_node_case_5(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *sibling = _rb_brother_node(node->parent);
	BOOL sblLeftIsBlack = (sibling->left) ? _IS_BLACK(sibling->left) : TRUE;
	BOOL sblRightIsBlack = (sibling->right) ? _IS_BLACK(sibling->right) : TRUE;

	if (_IS_RED(sibling))
	{}
	else if ((node == node->parent->left) &&
		(FALSE == sblLeftIsBlack) &&
		sblRightIsBlack)
	{
		_SET_RED(sibling);
		_SET_BLACK(sibling->left);
		__rb_rotate_right(dict, sibling);
	}
	else if ((node == node->parent->right) &&
			sblLeftIsBlack &&
			(FALSE == sblRightIsBlack))
	{
		_SET_RED(sibling);
		_SET_BLACK(sibling->right);
		__rb_rotate_left(dict, sibling);
	}
	else
	{}

	__rb_check_delete_node_case_6(dict, node);
}


/* --------------------__rb_check_delete_node_case_4----------------------- */
static void __rb_check_delete_node_case_4(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *sibling = _rb_brother_node(node->parent);
	BOOL sblLeftIsBlack = (sibling->left) ? _IS_BLACK(sibling->left) : TRUE;
	BOOL sblRightIsBlack = (sibling->right) ? _IS_BLACK(sibling->right) : TRUE;

	if (_IS_RED(node->parent) &&
		_IS_BLACK(sibling) &&
		sblLeftIsBlack && sblRightIsBlack)
	{_DEBUG("DEL: case 4");
		_SET_RED(sibling);
		_SET_BLACK(node->parent);
	}
	else
	{
		__rb_check_delete_node_case_5(dict, node);
	}
}


/* --------------------__rb_check_delete_node_case_3----------------------- */
static void __rb_check_delete_node_case_3(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *sibling = _rb_brother_node(node->parent);
	BOOL sblLeftIsBlack = (sibling->left) ? _IS_BLACK(sibling->left) : TRUE;
	BOOL sblRightIsBlack = (sibling->right) ? _IS_BLACK(sibling->right) : TRUE;

	if (_IS_BLACK(node->parent) &&
		_IS_BLACK(sibling) &&
		sblLeftIsBlack && sblRightIsBlack)
	{_DEBUG("DEL: case 3");
		_SET_RED(sibling);
		__rb_check_delete_node_case_1(dict, node->parent);
	}
	else
	{
		__rb_check_delete_node_case_4(dict, node);
	}
}


/* --------------------__rb_check_delete_node_case_2----------------------- */
static void __rb_check_delete_node_case_2(struct AMCDict *dict, struct AMCDictNode *node)
{
	struct AMCDictNode *sibling = _rb_brother_node(node);
		
	if (_IS_RED(sibling))
	{_DEBUG("DEL: case 2");
		_SET_RED(node->parent);
		_SET_BLACK(sibling);

		if (node == node->parent->left)
			__rbTree_RotateLeft(dict, node->parent);
		else
			__rbTree_RotateRight(dict, node->parent);
	}
	else
	{}

	return __rb_check_delete_node_case_3(dict, node);
}


/* --------------------__rb_check_delete_node_case_1----------------------- */
static void __rb_check_delete_node_case_1(struct AMCDict *dict, struct AMCDictNode *node)
{
	/* let's operate with diferent cases */
	if (_rb_node_is_root(node))
	{_DEBUG("DEL: case 1");
		return;
	}
	else			// Right child version
	{
		__rb_check_delete_node_case_1(dict, node);
	}
}


/* --------------------_rb_delete_node----------------------- */
static AMCDictErrno_st _rb_delete_node(struct AMCDict *dict, struct AMCDictNode *node, BOOL freeObject)
{
	AMCDictErrno_st error = {0, 0};
	BOOL nodeHasChild, nodeHasTwoChildren, nodeHasLeftChild, nodeHasRightChild;
	_rb_read_node_status(node, &nodeHasChild, &nodeHasTwoChildren, &nodeHasLeftChild, &nodeHasRightChild);

	if (FALSE == nodeHasChild)
	{_DEBUG("DEL: node %s is leaf", node->key);
		_rb_delete_leaf(dict, node, freeObject);
		return error;
	}
	else if (nodeHasTwoChildren)
	{_DEBUG("DEL: node %s has 2 children", node->key);
		struct AMCDictNode *smallestRightChild = _rb_find_min_leaf(node->right);

		/* replace with smallest node */
		strcpy(node->key, smallestRightChild->key);
		node->value = smallestRightChild->value;

		/* start operation with the smallest one */
		return _rb_delete_node(dict, smallestRightChild, freeObject);
	}
	else
	{
		struct AMCDictNode *child = (node->left) ? node->left : node->right;

		if (_IS_RED(node))		// if node is red, both parent and child nodes are black. We could simply replace it with its child
		{_DEBUG("DEL: node %s red", node->key);
			__rb_delete_node_and_reconnect_with(dict, node, child);
			_rb_free_node(node, freeObject);
			return error;
		}
		else if (_IS_RED(child))		// if node is black but its child is red. we could repace it with its child and refill the child as black
		{_DEBUG("DEL: node %ld black but child red", node->id);
			__rb_delete_node_and_reconnect_with(dict, node, child);
			_SET_BLACK(child);
			_rb_free_node(node, freeObject);
			return error;
		}
		else				// both node and its child are all black. This is the most complex one.
		{
			/* first of all, we should replace node with child */
			__rb_delete_node_and_reconnect_with(dict, node, child);
			_rb_free_node(node, freeObject);	/* "node" val is useless */

			__rb_check_delete_node_case_1(dict, child);
			return error;
		}
	}
}


/* --------------------_rb_delete_node----------------------- */
static AMCDictErrno_st _rbTree_Delete(struct AMCDict *dict, const char *key, BOOL freeObject)
{
	AMCDictErrno_st error = {0, 0};
	struct AMCDictNode *node = _rb_find_node(dict, key);

	if (NULL == node) {
		return AMCTreeError_IdNotFound;
	}

	return _rb_delete_node(dict, node, freeObject);
}


#endif


/********************/
#define __PRIVATE_DUMP_FUNCTIONS
#ifdef __PRIVATE_DUMP_FUNCTIONS

/* --------------------_rb_dump_node----------------------- */
static void _rb_dump_node(const struct AMCDictNode *node, unsigned long tabCount)
{
	if ((NULL == node->left) && (NULL == node->right))
	{
		return;
	}

	unsigned long tmp;
	for (tmp = 0; tmp < tabCount; tmp++)
	{
		printf("\t");
	}

	/* node */
	if (_IS_BLACK(node)) {
		printf("Node \033[0;43mBlack\033[0m (%s), ", node->key);
	}
	else {
		printf("Node \033[0;41mRed\033[0m (%s), ", node->key);
	}

	/* left */
	if (node->left) {
		if (_IS_BLACK(node->left)) {
			printf("<(\033[0;43m%s\033[0m), ", node->left->key);
		}
		else {
			printf("<(\033[0;41m%s\033[0m), ", node->left->key);
		}
	}
	else {
		printf("<(_), ");
	}

	/* right */
	if (node->right)
	{
		if (_IS_BLACK(node->right)) {
			printf("(\033[0;43m%s\033[0m)>\n", node->right->key);
		}
		else {
			printf("(\033[0;41m%s\033[0m)>\n", node->right->key);
		}
	}
	else {
		printf("(_)>\n");
	}

	/* check child */
	if (node->left) {
		if (node != node->left->parent) {
			printf("WARNING (\033[0;41m%s <- %s\033[0m)>\n", node->left->key, node->left->parent->key);exit(1);
		}
	}
	if (node->right) {
		if (node != node->right->parent) {
			printf("WARNING (\033[0;41m%s <- %s\033[0m)>\n", node->right->key, node->right->parent->key);exit(1);
		}
	}

	/* call self */
	if (node->left) {
		_rb_dump_node(node->left, tabCount + 1);
	}

	if (node->right) {
		_rb_dump_node(node->right, tabCount + 1);
	}

	return;
}


/* --------------------_rb_dump_dictionary----------------------- */
static void _rb_dump_dictionary(const struct AMCDict *dict)
{
	if (NULL == dict->children)
	{
		printf("Dictionary %p, size: 0\n", dict);
		return;
	}
	else if (1 == dict->nodeCount)
	{
		printf("Dictionary %p, size: %ld\n\t", dict, dict->count);
		if (_IS_BLACK(dict->children)) 	{
			printf("Node \033[0;43mBlack\033[0m (%s)\n", dict->children->key);
		}
		else {
			printf("Node \033[0;41mRed\033[0m (%s)\n", dict->children->key);
		}
	}
	else
	{
		printf("Dictionary %p, size: %ld\n", dict, dict->count);
		_rb_dump_node(dict->children, 1);
		return;
	}
}


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




#endif


/********/
#define __ARRAY_MODIFICATIONS
#ifdef __ARRAY_MODIFICATIONS




#endif


/********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES




#endif

/********/
/* end of file */

