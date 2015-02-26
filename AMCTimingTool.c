/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCTimingTool.c
	Description: 	
			This file provide implementations of AMCTimingTool.h.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-01-08: File created as "AMCTimingTool.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>

#include "AMCTimingTool.h"
#include "AMCMemPool.h"


#define _DEBUG_FLAG

#ifdef _DEBUG_FLAG
#define _DEBUG(fmt, args...)		printf("##"__FILE__", %d: "fmt"\n", __LINE__, ##args);
#else
#define _DEBUG(fmt, args...)
#endif


/********************/
#define __STRUCTURE_DEFINITIONS
#ifdef __STRUCTURE_DEFINITIONS

#define _CFG_DEFAULT_MEMPOOL_UNIT_COUNT		(32)

#define _SET_ERRNO(pErr, errCode)		if (pErr) (*(pErr))=(errCode);


typedef struct AMCTimingObject{
	struct AMCTimingObject  *pNext;
	struct AMCTimingTool    *pOwner;
	long                    currTime;
	long                    presetTime;
	AMCTiming_Config_t      flags;
	AMCTiming_Observation_Callback  callback;
	void                    *cbArg;
} AMCTimingObject_st;


typedef struct AMCTimingTool{
	AMCTimingObject_st       *objects;
	struct AMCMemPool        *memPool;
	pthread_t                thread;
	pthread_mutex_t          lock;
	long                     intervalUsec;
	int                      refCounter;	/* ++ by each thread; - by each "stop" */
} AMCTimingTool_st;


typedef struct AMCTimingSecObject{
	struct AMCTimingSecObject  *pNext;
	struct AMCTimingSecTool    *pOwner;
	long                       currSec;
	long                       presetSec;
	AMCTiming_Config_t         flags;
	AMCTimingSec_Observation_Callback  callback;
	void                       *cbArg;
} AMCTimingSecObject_st;



typedef struct AMCTimingSecTool{
	struct AMCTimingSecObject  *objects;
	struct AMCMemPool          *memPool;
	pthread_t                  thread;
	pthread_mutex_t            lock;
	int                        refCounter;	/* ++ by each thread; - by each "stop" */
} AMCTimingSecTool_st;





#endif

/********************/
#define __TIMEVAL_FUNCTION_INTERFACES
#ifdef __TIMEVAL_FUNCTION_INTERFACES

static int _pthread_alive(pthread_t thread);
static int _tmObj_Destroy(struct AMCTimingObject *tmObj);
static int _tmScObj_Destroy(struct AMCTimingSecObject *tmScObj);

#endif


/********************/
#define __INTERNAL_FUNCTIONS
#ifdef __INTERNAL_FUNCTIONS

static inline int _tmTool_Lock(struct AMCTimingTool *tmTool)
{
	return pthread_mutex_lock(&(tmTool->lock));
}


static inline int _tmTool_Trylock(struct AMCTimingTool *tmTool)
{
	return pthread_mutex_trylock(&(tmTool->lock));
}


static inline int _tmTool_Unlock(struct AMCTimingTool *tmTool)
{
	return pthread_mutex_unlock(&(tmTool->lock));
}


static inline int _tmScTool_Lock(struct AMCTimingSecTool *tmScTool)
{
	return pthread_mutex_lock(&(tmScTool->lock));
}


static inline int _tmScTool_Trylock(struct AMCTimingSecTool *tmScTool)
{
	return pthread_mutex_trylock(&(tmScTool->lock));
}


static inline int _tmScTool_Unlock(struct AMCTimingSecTool *tmScTool)
{
	return pthread_mutex_unlock(&(tmScTool->lock));
}


static inline BOOL _tmTool_IsRunning(const struct AMCTimingTool *tmTool)
{
	if (tmTool->thread)
	{
		return _pthread_alive(tmTool->thread) ? TRUE : FALSE;
	}
	else
	{
		return FALSE;
	}
}


static inline BOOL _tmScTool_IsRunning(const struct AMCTimingSecTool *tmScTool)
{
	if (tmScTool->thread)
	{
		return _pthread_alive(tmScTool->thread) ? TRUE : FALSE;
	}
	else
	{
		return FALSE;
	}
}


#endif


/********************/
#define __TIMEVAL_INVOLKE_TOOLS
#ifdef __TIMEVAL_INVOLKE_TOOLS

static size_t _maxSizeT(const size_t array[], size_t count)
{
	unsigned long tmp;
	size_t ret;

	for (ret = array[0], tmp = 0;
		tmp < count; 
		tmp ++)
	{
		if (array[tmp] > ret)
		{
			ret = array[tmp];
		}
	}

	return ret;
}

static size_t _memPoolUnitSize(void)
{
	const size_t MALLOC_STRUCTS[] = {
		sizeof(AMCTimingObject_st),
		sizeof(AMCTimingTool_st),
		sizeof(AMCTimingSecObject_st),
		sizeof(AMCTimingSecTool_st),
		sizeof(void *),
		0
	};
	static size_t ret = 0;
	if (0 == ret)
	{
		ret = _maxSizeT(MALLOC_STRUCTS, sizeof(MALLOC_STRUCTS) / sizeof(*MALLOC_STRUCTS));
	}
	return ret;
}


static BOOL _pthread_alive(pthread_t thread)
{
	int callStat = pthread_kill(thread, 0);
	if (ESRCH == callStat)
	{
		return FALSE;
	}
	else if (EINVAL == callStat)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

static inline int _pthread_disableCancel()
{
	return pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
}

static inline int _pthread_enableCancel()
{
	return pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}


static void *_threadTimingRoutine(void *arg)
{
	struct AMCTimingTool *tmTool = (AMCTimingTool_st *)arg;
	struct AMCTimingObject *pObj = NULL;
	struct AMCTimingObject *prevObj = NULL;
	long intvl = tmTool->intervalUsec;

	while (1)
	{
		/**/
		pthread_testcancel();
		usleep(intvl);
		pthread_testcancel();

		/**/
		_pthread_disableCancel();
		_tmTool_Lock(tmTool);
		if (1)
		{
			/* execute all timeout callbacks */
			pObj = tmTool->objects;
			prevObj = NULL;			
			while(pObj)
			{
				pObj->currTime -= tmTool->intervalUsec;

				/* timeout? */
				if ((pObj->currTime) <= 0)
				{
					/* execute timeout routine */
					if (pObj->callback)
					{
						(pObj->callback)(pObj, pObj->cbArg);
					}

					pObj = pObj->pNext;
				}
				/* normal status */
				else
				{
					prevObj = pObj;
					pObj = pObj->pNext;
				}
			}

			/* check all timeout and remove useless ones */
			pObj = tmTool->objects;
			prevObj = NULL;			
			while(pObj)
			{
				/* timeout? */
				if ((pObj->currTime) <= 0)
				{
					/* check whether should repeat */
					if ((pObj->flags) & TmCfg_Repeated)
					{
						pObj->currTime = pObj->presetTime;
						pObj = pObj->pNext;
					}
					else
					{
						if (prevObj)
						{
							prevObj->pNext = pObj->pNext;
							_tmObj_Destroy(pObj);			/* release TM Object */
							pObj = prevObj->pNext;
						}
						else
						{
							tmTool->objects = pObj->pNext;
							_tmObj_Destroy(pObj);			/* release TM Object */
							pObj = tmTool->objects;
						}
					}
				}
				else
				{
					prevObj = pObj;
					pObj = pObj->pNext;
				}
			}
		}
		_tmTool_Unlock(tmTool);
		_pthread_enableCancel();
		pthread_testcancel();
	}

	
	pthread_exit(NULL);
	return NULL;
}


static void *_threadTimingSecRoutine(void *arg)
{
	struct AMCTimingSecTool *tmScTool = (AMCTimingSecTool_st *)arg;
	struct AMCTimingSecObject *pScObj = NULL;
	struct AMCTimingSecObject *prevScObj = NULL;
	struct timeval currTime;
	int callStat;

	while (1)
	{
		/**/
		callStat = gettimeofday(&currTime, NULL);
		if (0 == callStat)
		{
			pthread_testcancel();
			usleep(AMC_TM_1_SECOND - currTime.tv_usec);
			pthread_testcancel();
		}
		else
		{
			pthread_testcancel();
			sleep(1);
			pthread_testcancel();
		}
		

		/**/
		_pthread_disableCancel();
		_tmScTool_Lock(tmScTool);
		if (1)
		{
			/* execute all timeout callbacks */
			pScObj = tmScTool->objects;
			prevScObj = NULL;			
			while(pScObj)
			{
				pScObj->currSec -= 1;

				/* timeout? */
				if ((pScObj->currSec) <= 0)
				{
					/* execute timeout routine */
					if (pScObj->callback)
					{
						(pScObj->callback)(pScObj, pScObj->cbArg);
					}

					pScObj = pScObj->pNext;
				}
				/* normal status */
				else
				{
					prevScObj = pScObj;
					pScObj = pScObj->pNext;
				}
			}

			/* check all timeout and remove useless ones */
			pScObj = tmScTool->objects;
			prevScObj = NULL;			
			while(pScObj)
			{
				/* timeout? */
				if ((pScObj->currSec) <= 0)
				{
					/* check whether should repeat */
					if ((pScObj->flags) & TmCfg_Repeated)
					{
						pScObj->currSec= pScObj->presetSec;
						pScObj = pScObj->pNext;
					}
					else
					{
						if (prevScObj)
						{
							prevScObj->pNext = pScObj->pNext;
							_tmScObj_Destroy(pScObj);			/* release SEC Object */
							pScObj = prevScObj->pNext;
						}
						else
						{
							tmScTool->objects = pScObj->pNext;
							_tmScObj_Destroy(pScObj);			/* release SEC Object */
							pScObj = tmScTool->objects;
						}
					}
				}
				else
				{
					prevScObj = pScObj;
					pScObj = pScObj->pNext;
				}
			}
		}
		_tmScTool_Unlock(tmScTool);
		_pthread_enableCancel();
		pthread_testcancel();
	}

	
	pthread_exit(NULL);
	return NULL;
}


#endif


/********************/
#define __TM_OBJ_OPERATIONS
#ifdef __TM_OBJ_OPERATIONS

static int _tmObj_Destroy(struct AMCTimingObject *tmObj)
{
	int callStat = AMCMemPool_Free(tmObj);
	return (0 == callStat) ? 0 : TmToolErrno_MemPoolError;
}


static int _tmScObj_Destroy(struct AMCTimingSecObject *tmScObj)
{
	int callStat = AMCMemPool_Free(tmScObj);
	return (0 == callStat) ? 0 : TmToolErrno_MemPoolError;
}


static int _tmObj_RemoveAndDestroy(struct AMCTimingObject *tmObj)
{
	if (NULL == tmObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	/* first object? */
	if (tmObj->pOwner->objects == tmObj)
	{
		tmObj->pOwner->objects = tmObj->pNext;
		return _tmObj_Destroy(tmObj);
	}
	/* not first object */
	else
	{
		struct AMCTimingObject *prevObj;

		prevObj = tmObj->pOwner->objects;
		while (NULL != prevObj->pNext)
		{
			if (tmObj == prevObj->pNext)
			{
				prevObj->pNext = tmObj->pNext;
				return _tmObj_Destroy(tmObj);
			}
		}

		/* regulary code should NOT execute here */
		return TmToolError_TmObjInvalid;
	}
}


static int _tmScObj_RemoveAndDestroy(struct AMCTimingSecObject *tmScObj)
{
	if (NULL == tmScObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	/* first object? */
	if (tmScObj->pOwner->objects == tmScObj)
	{
		tmScObj->pOwner->objects = tmScObj->pNext;
		return _tmScObj_Destroy(tmScObj);
	}
	/* not first object */
	else
	{
		struct AMCTimingSecObject *prevScObj;

		prevScObj = tmScObj->pOwner->objects;
		while (NULL != prevScObj->pNext)
		{
			if (tmScObj == prevScObj->pNext)
			{
				prevScObj->pNext = tmScObj->pNext;
				return _tmScObj_Destroy(tmScObj);
			}
		}

		/* regulary code should NOT execute here */
		return TmToolError_TmObjInvalid;
	}
}


#endif



/********************/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

#define _LOCK_TM_TOOL_AND_SAVE_FLAG(pTmTool, boolFlag)		boolFlag = ((0 == _tmTool_Trylock(pTmTool)) ? TRUE : FALSE)
#define _UNLOCK_TM_TOOL_BY_FLAG(pTmTool, boolFlag)			if (boolFlag) _tmTool_Unlock(pTmTool)

struct AMCTimingTool *AMCTiming_New(long intervalUsec, long expectedObjectCount, int *errOut)
{
	AMCTimingTool_st *ret = NULL;

	/* check parameters */
	if (intervalUsec < 1000)
	{
		_DEBUG("interval not supported.");
		_SET_ERRNO(errOut, TmToolErrno_ParamError);
		return NULL;
	}

	if (expectedObjectCount <= 0)
	{
		expectedObjectCount = _CFG_DEFAULT_MEMPOOL_UNIT_COUNT;
	}

	/* allocate basic tool struct */
	ret = malloc(sizeof(*ret));
	if (NULL == ret)
	{
		_DEBUG("Cannot malloc timing tool.");
		_SET_ERRNO(errOut, TmToolErrno_SystemCallError);
		return NULL;
	}

	/* create memory pool */
	ret->memPool = AMCMemPool_Create(_memPoolUnitSize(), expectedObjectCount, (expectedObjectCount + 1) / 2, TRUE);
	if (NULL == ret)
	{
		_DEBUG("Cannot alloc memory tool.");
		_SET_ERRNO(errOut, TmToolErrno_MemPoolError);
		free(ret);
		return NULL;
	}

	/* initialize struct members */
	ret->objects = NULL;
	ret->thread = 0;
	ret->intervalUsec = intervalUsec;
	ret->refCounter = 0;

	if (0 != pthread_mutex_init(&(ret->lock), NULL))
	{
		_SET_ERRNO(errOut, TmToolErrno_SystemCallError);
		AMCMemPool_Destory(ret->memPool);
		free(ret);
		return NULL;
	}

	/* return */
	return ret;
}


int AMCTiming_Destroy(struct AMCTimingTool *tmTool)
{
	if (NULL == tmTool)
	{
		return TmToolErrno_ParamError;
	}

	if (0 == _tmTool_Trylock(tmTool))
	{
		AMCTiming_Stop(tmTool);
		AMCMemPool_Destory(tmTool->memPool);	// objects are assigned in memory pool and destroyed here.
		_tmTool_Unlock(tmTool);
		pthread_mutex_destroy(&(tmTool->lock));
		
		return 0;
	}
	else
	{
		return TmToolError_InRountine;
	}
}


int AMCTiming_Start(struct AMCTimingTool *tmTool)
{
	int callStat;
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmTool)
	{
		return TmToolErrno_ParamError;
	}

	if (_tmTool_IsRunning(tmTool))
	{
		return TmToolError_TmToolRunning;
	}

	/* protected block */
	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmTool, shouldLock);
	if (1)	// simply use as a block
	{
		callStat = pthread_create(&(tmTool->thread), NULL, _threadTimingRoutine, (void*)tmTool);
		if (0 == callStat)
		{
			++ (tmTool->refCounter);
		}
		else
		{
			ret = TmToolErrno_SystemCallError;
		}
	}
	_UNLOCK_TM_TOOL_BY_FLAG(tmTool, shouldLock);
	
	return ret;
}


int AMCTiming_Stop(struct AMCTimingTool *tmTool)
{
	int callStat;
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmTool)
	{
		return TmToolErrno_ParamError;
	}

	if (FALSE == _tmTool_IsRunning(tmTool))
	{
		return TmToolError_TmToolStopped;
	}

	/* protected block */
	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmTool, shouldLock);
	if (1)	// simply use as a block
	{
		callStat = pthread_cancel(tmTool->thread);
		if (0 == callStat)
		{
			callStat = pthread_join(tmTool->thread, NULL);
		}

		if (0 == callStat)
		{
			tmTool->thread = 0;
			-- (tmTool->refCounter);
		}
		else
		{
			ret = TmToolError_TmToolStopped;
		}
	}
	_UNLOCK_TM_TOOL_BY_FLAG(tmTool, shouldLock);

	return ret;
}


BOOL AMCTiming_IsRunning(const struct AMCTimingTool *tmTool, int *errnoOut)
{
	if (NULL == tmTool)
	{
		_SET_ERRNO(errnoOut, TmToolErrno_ParamError);
		return FALSE;
	}

	return _tmTool_IsRunning(tmTool);
}


struct AMCTimingObject *AMCTiming_AddObject(
	struct AMCTimingTool *tmTool, 
	AMCTiming_Observation_Callback observation, 
	long timeoutUsec, 
	AMCTiming_Config_t flags, 
	void *arg, 
	int *errnoOut)
{
	struct AMCTimingObject *tmObj = NULL;
	BOOL shouldLock;

	/* check parameter */
	if (NULL == tmTool)
	{
		_SET_ERRNO(errnoOut, TmToolErrno_ParamError);
		return NULL;
	}

	if (timeoutUsec < (tmTool->intervalUsec))
	{
		timeoutUsec = tmTool->intervalUsec;
		_SET_ERRNO(errnoOut, TmToolWarn_TimeoutTooSoon);
	}

	/* allocate object */
	tmObj = AMCMemPool_Alloc(tmTool->memPool);
	if (NULL == tmObj)
	{
		_SET_ERRNO(errnoOut, TmToolErrno_MemPoolError);
		return NULL;
	}

	/* set initial values */
	tmObj->pNext = NULL;
	tmObj->pOwner = tmTool;
	tmObj->currTime = timeoutUsec;
	tmObj->presetTime = timeoutUsec;
	tmObj->flags = flags;
	tmObj->callback = observation;
	tmObj->cbArg = arg;

	/* add to timing tool */
	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmTool, shouldLock);
	if (1)
	{
		struct AMCTimingObject *prevObj = tmTool->objects;

		if (FALSE == shouldLock)
		{
			tmObj->currTime += tmTool->intervalUsec;
		}

		if (NULL == prevObj)
		{
			tmTool->objects = tmObj;
		}
		else
		{		
			while (NULL != prevObj->pNext)
			{
				prevObj = prevObj->pNext;
			}
			prevObj->pNext = tmObj;
		}
	}
	_UNLOCK_TM_TOOL_BY_FLAG(tmTool, shouldLock);

	/* return */
	return tmObj;
}


int AMCTiming_DelObject(struct AMCTimingObject *tmObj)
{
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmObj->pOwner, shouldLock);
	if (shouldLock)
	{
		/* not in the routine */
		ret = _tmObj_RemoveAndDestroy(tmObj);
	}
	else
	{
		/* in the routine */
		tmObj->flags &= ~TmCfg_Repeated;
	}
	_UNLOCK_TM_TOOL_BY_FLAG(tmObj->pOwner, shouldLock);

	return ret;
}


int AMCTiming_ResetTimeout(struct AMCTimingObject *tmObj, long timeoutUsec)
{
	BOOL shouldLock;

	if (NULL == tmObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	if (timeoutUsec < (tmObj->pOwner->intervalUsec))
	{
		timeoutUsec = tmObj->pOwner->intervalUsec;
	}

	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmObj->pOwner, shouldLock);
	tmObj->presetTime = timeoutUsec;
	_UNLOCK_TM_TOOL_BY_FLAG(tmObj->pOwner, shouldLock);

	return 0;
}


int AMCTiming_TriggerObject(struct AMCTimingObject *tmObj)
{
	BOOL shouldLock;

	if (NULL == tmObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	_LOCK_TM_TOOL_AND_SAVE_FLAG(tmObj->pOwner, shouldLock);
	tmObj->currTime = 0;
	_UNLOCK_TM_TOOL_BY_FLAG(tmObj->pOwner, shouldLock);

	return 0;
}


int AMCTiming_DebugStdout(const struct AMCTimingTool *tmTool)
{
	// TODO:
	return 0;
}


/**/
#define _LOCK_SC_TOOL_AND_SAVE_FLAG(pTmScTool, boolFlag)		boolFlag = ((0 == _tmScTool_Trylock(pTmScTool)) ? TRUE : FALSE)
#define _UNLOCK_SC_TOOL_BY_FLAG(pTmScTool, boolFlag)			if (boolFlag) _tmScTool_Unlock(pTmScTool)

struct AMCTimingSecTool *AMCTimingSecond_New(long expectedObjectCount, int *errOut)
{
	AMCTimingSecTool_st *ret = NULL;

	if (expectedObjectCount <= 0)
	{
		expectedObjectCount = _CFG_DEFAULT_MEMPOOL_UNIT_COUNT;
	}

	/* allocate basic tool struct */
	ret = malloc(sizeof(*ret));
	if (NULL == ret)
	{
		_DEBUG("Cannot malloc timing second tool.");
		_SET_ERRNO(errOut, TmToolErrno_SystemCallError);
		return NULL;
	}

	/* create memory pool */
	ret->memPool = AMCMemPool_Create(_memPoolUnitSize(), expectedObjectCount, (expectedObjectCount + 1) / 2, TRUE);
	if (NULL == ret)
	{
		_DEBUG("Cannot alloc memory tool.");
		_SET_ERRNO(errOut, TmToolErrno_MemPoolError);
		free(ret);
		return NULL;
	}

	/* initialize struct members */
	ret->objects = NULL;
	ret->thread = 0;
	ret->refCounter = 0;

	if (0 != pthread_mutex_init(&(ret->lock), NULL))
	{
		_SET_ERRNO(errOut, TmToolErrno_SystemCallError);
		AMCMemPool_Destory(ret->memPool);
		free(ret);
		return NULL;
	}

	/* return */
	return ret;
}


int AMCTimingSecond_Destroy(struct AMCTimingSecTool *tmScTool)
{
	if (NULL == tmScTool)
	{
		return TmToolErrno_ParamError;
	}

	if (0 == _tmScTool_Trylock(tmScTool))
	{
		AMCTimingSecond_Stop(tmScTool);
		AMCMemPool_Destory(tmScTool->memPool);	// objects are assigned in memory pool and destroyed here.
		_tmScTool_Unlock(tmScTool);
		pthread_mutex_destroy(&(tmScTool->lock));

		return 0;
	}
	else
	{
		return TmToolError_InRountine; 			/* cannot lock in rountine */
	}
}


int AMCTimingSecond_Start(struct AMCTimingSecTool *tmScTool)
{
	int callStat;
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmScTool)
	{
		return TmToolErrno_ParamError;
	}

	if (_tmScTool_IsRunning(tmScTool))
	{
		return TmToolError_TmToolRunning;
	}

	/* protected block */
	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScTool, shouldLock);
	if (1)	// simply use as a block
	{
		callStat = pthread_create(&(tmScTool->thread), NULL, _threadTimingSecRoutine, (void*)tmScTool);
		if (0 == callStat)
		{
			++ (tmScTool->refCounter);
		}
		else
		{
			ret = TmToolErrno_SystemCallError;
		}
	}
	_UNLOCK_SC_TOOL_BY_FLAG(tmScTool, shouldLock);
	
	return ret;
}


int AMCTimingSecond_Stop(struct AMCTimingSecTool *tmScTool)
{
	int callStat;
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmScTool)
	{
		return TmToolErrno_ParamError;
	}

	if (FALSE == _tmScTool_IsRunning(tmScTool))
	{
		return TmToolError_TmToolStopped;
	}

	/* protected block */
	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScTool, shouldLock);
	if (1)	// simply use as a block
	{
		callStat = pthread_cancel(tmScTool->thread);
		if (0 == callStat)
		{
			callStat = pthread_join(tmScTool->thread, NULL);
		}

		if (0 == callStat)
		{
			tmScTool->thread = 0;
			-- (tmScTool->refCounter);
		}
		else
		{
			ret = TmToolError_TmToolStopped;
		}
	}
	_UNLOCK_SC_TOOL_BY_FLAG(tmScTool, shouldLock);

	return ret;
}


BOOL AMCTimingSecond_IsRunning(const struct AMCTimingSecTool *tmScTool, int *errOut)
{
	if (NULL == tmScTool)
	{
		_SET_ERRNO(errOut, TmToolErrno_ParamError);
		return FALSE;
	}

	return _tmScTool_IsRunning(tmScTool);
}


struct AMCTimingSecObject *AMCTimingSecond_AddObject(
	struct AMCTimingSecTool *tmScTool, 
	AMCTimingSec_Observation_Callback observation, 
	long timeoutSec, 
	AMCTiming_Config_t flags, 
	void *arg, 
	int *errnoOut)
{
	struct AMCTimingSecObject *tmScObj = NULL;
	BOOL shouldLock;

	/* check parameter */
	if (NULL == tmScTool)
	{
		_SET_ERRNO(errnoOut, TmToolErrno_ParamError);
		return NULL;
	}

	if (timeoutSec < 1)
	{
		timeoutSec = 1;
		_SET_ERRNO(errnoOut, TmToolWarn_TimeoutTooSoon);
	}

	/* allocate object */
	tmScObj = AMCMemPool_Alloc(tmScTool->memPool);
	if (NULL == tmScObj)
	{
		_SET_ERRNO(errnoOut, TmToolErrno_MemPoolError);
		return NULL;
	}

	/* set initial values */
	tmScObj->pNext = NULL;
	tmScObj->pOwner = tmScTool;
	tmScObj->currSec = timeoutSec;
	tmScObj->presetSec = timeoutSec;
	tmScObj->flags = flags;
	tmScObj->callback = observation;
	tmScObj->cbArg = arg;

	/* add to timing tool */
	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScTool, shouldLock);
	if (1)
	{
		struct AMCTimingSecObject *prevScObj = tmScTool->objects;

		if (FALSE == shouldLock)
		{
			tmScObj->currSec += 1;
		}

		if (NULL == prevScObj)
		{
			tmScTool->objects = tmScObj;
		}
		else
		{		
			while (NULL != prevScObj->pNext)
			{
				prevScObj = prevScObj->pNext;
			}
			prevScObj->pNext = tmScObj;
		}
	}
	_UNLOCK_SC_TOOL_BY_FLAG(tmScTool, shouldLock);

	/* return */
	return tmScObj;
}


int AMCTimingSecond_DelObject(struct AMCTimingSecObject *tmScObj)
{
	int ret = 0;
	BOOL shouldLock;

	if (NULL == tmScObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmScObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScObj->pOwner, shouldLock);
	ret = _tmScObj_RemoveAndDestroy(tmScObj);
	_UNLOCK_SC_TOOL_BY_FLAG(tmScObj->pOwner, shouldLock);

	return ret;
}


int AMCTimingSecond_ResetTimeout(struct AMCTimingSecObject *tmScObj, long timeoutSec)
{
	BOOL shouldLock;

	if (NULL == tmScObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmScObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	if (timeoutSec < 1)
	{
		timeoutSec = 1;
	}

	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScObj->pOwner, shouldLock);
	tmScObj->presetSec = timeoutSec;
	_UNLOCK_SC_TOOL_BY_FLAG(tmScObj->pOwner, shouldLock);

	return 0;
}


int AMCTimingSecond_TriggerObject(struct AMCTimingSecObject *tmScObj)
{
	BOOL shouldLock;

	if (NULL == tmScObj)
	{
		return TmToolErrno_ParamError;
	}

	if (NULL == tmScObj->pOwner)
	{
		return TmToolError_TmObjInvalid;
	}

	_LOCK_SC_TOOL_AND_SAVE_FLAG(tmScObj->pOwner, shouldLock);
	tmScObj->currSec = 0;
	_UNLOCK_SC_TOOL_BY_FLAG(tmScObj->pOwner, shouldLock);

	return 0;
}


int AMCTimingSecond_DebugStdout(const struct AMCTimingSecTool *tmTool)
{
	// TODO:
	if (NULL == tmTool)
	{
		printf("Second timing tool NULL\n");
	}
	printf("\tMem Pool status: ");
	AMCMemPool_DebugStdout(tmTool->memPool);
	return 0;
}



#endif



