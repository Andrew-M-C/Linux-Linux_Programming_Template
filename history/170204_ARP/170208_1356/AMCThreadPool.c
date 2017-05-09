/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang
	
	File name: AMCThreadPool.c
	Description: 	
			This file provide implementations to create and use of POSIX thread pool. 
	Reference:
			http://kenby.iteye.com/blog/1168087		<< Linux C thread pool implementation >>
			http://blog.csdn.net/hudashi/article/details/7709421    << pthread_cond_signal and pthread_cond_wait brief >>
			
	Author:		Andrew Chang
	History:
		2016-02-29: File created as "AMCThreadPool.c"

--------------------------------------------------------------
	Copyright information: 
			This file is open source and free to use. 
			I (author) promise nothing about this source file. If you use it in
		commercial application, please make sure you have done detailed test of
		this program to avoid possible bugs.
		    For any suggestions, problems and bug reports, please contact Andrew
		through either E-mails below:
		1. zhangmin@tp-link.com.cn  (as long as I am an employee of TP-LINK)
		2. laplacezhang@126.com
		
********************************************************************************/

#include "AMCThreadPool.h"
#include "AMCMemPool.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define _DEBUG_FLAG

/**********/
#define __OPERATION_MACROS
#ifdef __OPERATION_MACROS

#ifdef _DEBUG_FLAG 
#define _DEBUG(fmt, args...)		printf("##"__FILE__", %d: "fmt"\n", __LINE__, ##args)
#define _ERROR(fmt, args...)		printf("## [ERROR] "__FILE__", %d: "fmt"\n", __LINE__, ##args)
#else
#define _DEBUG(fmt, args...)
#define _ERROR(fmt, args...)		printf("[TPOOL] "fmt"\n", ##args)
#endif

#define _PTR_VALUE(ptr)		((void*)(ptr))


#endif

/**********/
#define __DATA_TYPES
#ifdef __DATA_TYPES

struct AMCThreadPool;		// forward declaration

typedef enum {
	_TPoolMsg_NoMsg = 0,
	_TPoolMsg_NewTask,
	_TPoolMsg_Destroy,
} AMCThreadPoolMsg_t;


struct AMCThread {
	struct AMCThread      *pNext;		// used to construct a thread chain
	struct AMCThreadPool  *pOwnerPool;
	pthread_t              thread;
	BOOL                   isRunning;	// no actual use.
	BOOL                   isInitFail;	// no actual use.
};


struct AMCThreadTask {
	struct AMCThreadTask  *pNext;
	AMCThreadPool_Routine  pRoutine;
	void                  *pArg;
};


struct AMCThreadPool {
	AMCThreadPoolMsg_t     msg;
	size_t                 threadTotal;		// total valid thread amount
	size_t                 freeCount;		// free-to-use thread amount
	size_t                 initSize;		// initialized thread size
	pthread_mutex_t        mutex;
	pthread_cond_t         cond;
	struct AMCMemPool     *taskQueueMemPool;
	struct AMCThreadTask  *taskQueue;
	struct AMCThread      *staticThreads;
	struct AMCThread      *dynamicThreads;
};

#endif


/**********/
#define __INTERNAL_FUNCTIONS
#ifdef __INTERNAL_FUNCTIONS

/****/
/* lock functions */
static inline int _LOCK_POOL(struct AMCThreadPool *pool)
{
	return pthread_mutex_trylock(&(pool->mutex));
}


static inline int _UNLOCK_POOL(struct AMCThreadPool *pool)
{
	return pthread_mutex_unlock(&(pool->mutex));
}


static inline int _WAIT_POOL(struct AMCThreadPool *pool)
{
	return pthread_cond_wait(&(pool->cond), &(pool->mutex));
}


static inline int _SIGNAL_POOL(struct AMCThreadPool *pool)
{
	return pthread_cond_signal(&(pool->cond));
}


static inline int _BROADCAST_POOL(struct AMCThreadPool *pool)
{
	return pthread_cond_broadcast(&(pool->cond));
}


/****/
/* thread pool basic informations */
static inline inline int _pool_free_thread_count(const struct AMCThreadPool *pool)
{
	return pool->freeCount;
}


/****/
/* thread deallocate function */
/* ONLY invoked after all threads exit. This function assumes that all threads have been end normally */
static int _pool_deallocate(struct AMCThreadPool *pool)
{
	int status = 0;
	struct AMCThread *thread, *threadNext;

	/* deallocate dynamic threads */
	if (pool->dynamicThreads)
	{
		thread = pool->dynamicThreads;
		pool->dynamicThreads = NULL;
		while(thread)
		{
			threadNext = thread->pNext;
			free(thread);
			thread = threadNext;
		}
	}

	/* deallocate static threads */
	if (pool->staticThreads)
	{
		free(pool->staticThreads);
		pool->staticThreads = NULL;
	}

	/* deallocate task queue */
	if (pool->taskQueueMemPool)
	{
		AMCMemPool_Destory(pool->taskQueueMemPool);
		pool->taskQueueMemPool = NULL;
		pool->taskQueue = NULL;
	}

	/* deallopcate thread pool itself */
	if (0 == status)
	{
		pthread_cond_destroy (&(pool->cond));
		pthread_mutex_destroy(&(pool->mutex));

		pool->threadTotal = 0;
		pool->freeCount = 0;
		pool->initSize = 0;

		free(pool);
		pool = NULL;
	}
	
	/* ends */
	return status;
}


static inline BOOL _pool_is_empty(const struct AMCThreadPool *pool)
{
	return (NULL == pool->taskQueue);
}


static inline BOOL _pool_is_to_destroy(const struct AMCThreadPool *pool)
{
	return (_TPoolMsg_Destroy == pool->msg);
}


/****/
/* thread operation functions */
static BOOL _thread_is_static_one(const struct AMCThread *thread)	// determine whether a thread is created when its thread pool initialized
{
	struct AMCThreadPool *pool = thread->pOwnerPool;
	struct AMCThread *staticThreads = pool->staticThreads;
	size_t staticThreadSize = sizeof(*staticThreads) * (pool->initSize);

	if ((_PTR_VALUE(thread) >= _PTR_VALUE(staticThreads)) &&
		(_PTR_VALUE(thread) < _PTR_VALUE(staticThreads) + staticThreadSize))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


static inline BOOL _thread_is_dynamic_one(const struct AMCThread *thread)
{
	return !_thread_is_static_one(thread);
}


static struct AMCThreadTask *_task_in_the_end_of_queue(const struct AMCThreadPool *pool)
{
	struct AMCThreadTask *task = pool->taskQueue;
	if (task)
	{
		while(task->pNext)
		{
			task = task->pNext;
		}	
	}
	return task;
}


static inline struct AMCThreadTask *_task_in_the_beginning_of_queue(const struct AMCThreadPool *pool)
{
	return pool->taskQueue;
}


#endif


/**********/
#define __THREAD_POOL_TASK
#ifdef __THREAD_POOL_TASK

static void *_threadPoolTaskStatic(void *arg)
{
	pthread_detach(pthread_self());

	struct AMCThread *self = (struct AMCThread *)arg;
	struct AMCThreadPool *pool = self->pOwnerPool;

	AMCThreadPool_Routine routine;
	void *routineArg;

	_DEBUG("Thread %p allocated", self);

	for (;;)
	{
		_LOCK_POOL(pool);			/* ---LOCK--- */

		/****/
		/* check messages */
		if ((FALSE == _pool_is_to_destroy(pool)) &&
			_pool_is_empty(pool))
		{
			_DEBUG("Thread %p sleeping", self);
			_WAIT_POOL(pool);
		}

		_DEBUG("Thread %p awake", self);


		if (_pool_is_to_destroy(pool))
		{
			_DEBUG("Thread %p escape", self);
			_UNLOCK_POOL(pool);
			break;					/* !!! */
		}
		else
		{
			struct AMCThreadTask *task = _task_in_the_beginning_of_queue(pool);
			if (task)
			{
				pool->taskQueue = task->pNext;
				routine = task->pRoutine;
				routineArg = task->pArg;
				AMCMemPool_Free(task);
				task = NULL;
			}
			else
			{
				_ERROR("Thread %p: No routine specified", self);
				_UNLOCK_POOL(pool);
				continue;			/* !!! */
			}

			if (NULL == _task_in_the_beginning_of_queue(pool))
			{
				pool->msg = _TPoolMsg_NoMsg;
				pool->freeCount -= 1;
			}
		}

		_UNLOCK_POOL(pool);			/* --UNLOCK-- */

		/****/
		/* involke routine */
		routine(routineArg);

		/****/
		/* ends */
		if (_pool_is_to_destroy(pool))
		{
			break;		/* !!! */
		}
		else
		{
			_LOCK_POOL(pool);

			_DEBUG("Thread %p end task", self);
			pool->freeCount += 1;

			_UNLOCK_POOL(pool);
		}
	}

	_DEBUG("Thread %p exit", self);
	pthread_exit(NULL);
	return NULL;
}


static void *_threadPoolTaskDynamic(void *arg)
{
	struct AMCThread *self = (struct AMCThread *)arg;
	pthread_detach(pthread_self());

	_DEBUG("Thread %p allocated dynamic", self);

	// TODO:

	_DEBUG("Thread item <%p> exit", self);
	pthread_exit(NULL);
	return NULL;
}

#endif


/**********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES

struct AMCThreadPool *AMCThreadPool_Create(size_t threadCount)
{
	struct AMCThreadPool *pool = NULL;
	BOOL isOK = TRUE;

	if (0 == threadCount)
	{
		_ERROR("Zero thread pool size not allowed.");
		errno = EINVAL;
		return NULL;
	}

	pool = malloc(sizeof(*pool));
	if (NULL == pool) {
		isOK = FALSE;
	}

	/* init basic members */
	if (isOK)
	{
		pool->msg = _TPoolMsg_NoMsg;
		pool->threadTotal = 0;
		pool->freeCount = 0;
		pool->initSize = threadCount;
		pthread_mutex_init(&(pool->mutex), NULL);
		pthread_cond_init (&(pool->cond),  NULL);
		pool->taskQueueMemPool = NULL;
		pool->taskQueue = NULL;
		pool->staticThreads = NULL;
		pool->dynamicThreads = NULL;

		/* create queue memory pool */
		pool->taskQueueMemPool = AMCMemPool_Create(sizeof(struct AMCThreadTask), threadCount, threadCount, FALSE);
		if (NULL == pool->taskQueueMemPool) {
			isOK = FALSE;
		}
	}

	/* create threads */
	if (isOK)
	{
		pool->staticThreads = malloc(sizeof(struct AMCThread) * threadCount);
		if (NULL == pool->staticThreads) {
			isOK = FALSE;
		}
	}

	/* init thread */
	if (isOK)
	{
		int callStat = 0;
		size_t tmp;
		struct AMCThread *threadArray = pool->staticThreads;

		for (tmp = 0; tmp < threadCount; tmp++)
		{
			threadArray[tmp].pNext      = &(threadArray[tmp + 1]);
			threadArray[tmp].pOwnerPool = pool;
			threadArray[tmp].isRunning  = FALSE;
			threadArray[tmp].isInitFail = FALSE;
			
			callStat = pthread_create(&(threadArray[tmp].thread), NULL, _threadPoolTaskStatic, &(threadArray[tmp]));
			if (0 == callStat) {
				_DEBUG("Thread No.%d created", tmp);
				pool->freeCount += 1;
				pool->threadTotal += 1;
			}
			else {
				threadArray[tmp].isInitFail = FALSE;
			}
		}

		threadArray[threadCount - 1].pNext = NULL;
	}

	/* return */
	if (FALSE == isOK)
	{
		_ERROR("Failed to init thread pool");
		if (pool) {
			_pool_deallocate(pool);
			free(pool);
			pool = NULL;
		}
	}
	else
	{
		usleep(500);
	}
	return pool;
}


int AMCThreadPool_AddTask(struct AMCThreadPool *pool, AMCThreadPool_Routine routine, void *arg)
{
	if (pool && routine)
	{}
	else {
		errno = EINVAL;
		return -1;
	}

	int ret = 0;
_DEBUG("MARK");
	_LOCK_POOL(pool);		/* ---LOCK--- */

	/****/
	/* fetch a thread to run this task */
	if (0 == pool->freeCount)		/* create a dynamic thread to do this */
	{
		struct AMCThread *newThread = malloc(sizeof(*newThread));
_DEBUG("MARK");
		if (NULL == newThread)
		{
			_ERROR("Cannot alloc dynamic thread: %s", strerror(errno));
			ret = -1;
		}
		else
		{
			newThread->pNext = pool->dynamicThreads;
			pool->dynamicThreads = newThread;
			newThread->pOwnerPool = pool;
			newThread->isRunning = TRUE;
			newThread->isInitFail = FALSE;
			pool->msg = _TPoolMsg_NewTask;
_DEBUG("MARK");
			ret = pthread_create(&(newThread->thread), NULL, _threadPoolTaskDynamic, newThread);
			if (0 == ret) {
_DEBUG("MARK");
				// OK
			}
			else {
				_ERROR("Cannot create dynamic thread: %s", strerror(ret));
				pool->dynamicThreads = newThread->pNext;
				free(newThread);
				newThread = NULL;
			}
		}
	}
	else						/* use static thread to do this */
	{
		// nothing to do, just save task msg and send signal
	}
_DEBUG("MARK");
	/****/
	/* save task information */
	if (0 == ret)
	{
_DEBUG("MARK");	
		struct AMCThreadTask *newTask = AMCMemPool_Alloc(pool->taskQueueMemPool);
		if (NULL == newTask)
		{
			_ERROR("Cannot get new task");
			ret = -1;
		}
		else
		{
			struct AMCThreadTask *lastTask = _task_in_the_end_of_queue(pool);
_DEBUG("MARK");
			newTask->pNext = NULL;
			newTask->pRoutine = routine;
			newTask->pArg = arg;
			
			if (lastTask) {
				lastTask->pNext = newTask;
			}
			else {
				pool->taskQueue = newTask;
			}
		}
	}
_DEBUG("MARK");
	/****/
	/* send signal to waiting threads */
	if (0 == ret)
	{
		_DEBUG("SIGNAL");
		_SIGNAL_POOL(pool);
	}
_DEBUG("MARK");
	_UNLOCK_POOL(pool);		/* --UNLOCK-- */
_DEBUG("MARK");
	return ret;
}


int AMCThreadPool_Destroy(struct AMCThreadPool *pool)
{
	if (NULL == pool) {
		errno = EINVAL;
		return -1;
	}

	_LOCK_POOL(pool);		/* ---LOCK--- */

	if (_TPoolMsg_Destroy != pool->msg) {
		pool->msg = _TPoolMsg_Destroy;
		_BROADCAST_POOL(pool);		/* --BROADCAST-- */		/* actual deallocation will be done in threads */
	}
	_UNLOCK_POOL(pool);		/* --UNLOCK-- */

	return 0;
}


size_t AMCThreadPool_runningThreadCount(struct AMCThreadPool *pool)
{
	if (NULL == pool) {
		return 0;
	}

	size_t ret = 0;
	struct AMCThread *thread = NULL;

	_LOCK_POOL(pool);		/* ---LOCK--- */

	if (_TPoolMsg_Destroy != pool->msg)
	{
		for (thread = pool->staticThreads; thread; thread = thread->pNext)
		{
			if (thread->isRunning) {
				ret ++;
			}
		}

		for (thread = pool->dynamicThreads; thread; thread = thread->pNext)
		{
			if (thread->isRunning) {
				ret ++;
			}
		}
	}
	_UNLOCK_POOL(pool);		/* --UNLOCK-- */

	return ret;
}


size_t AMCThreadPool_freeThreadCount(struct AMCThreadPool *pool)
{
	if (pool) {
		return _pool_free_thread_count(pool);
	}
	else {
		return 0;
	}
}


size_t AMCThreadPool_totalThreadCount(struct AMCThreadPool *pool)
{
	return pool->threadTotal;
}



#endif


