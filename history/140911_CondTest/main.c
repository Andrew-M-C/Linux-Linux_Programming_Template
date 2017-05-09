/**/

#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define CFG_LIB_THREAD
#define CFG_LIB_RAND
#define CFG_LIB_TIME
#define CFG_LIB_SLEEP
#include "AMCCommonLib.h"

//pthread_mutex_t _mutexMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  _cond  = PTHREAD_COND_INITIALIZER;

#define MAX_RAND_USEC		2000000
#define MIN_RAND_USEC		0

#define CFG_PRODUCER_COUNT	3


#define REPEAT_TIME_MASK	0x0000FFFF
#define REPEAT_TIME_OFF	0
#define ID_MASK			0xFFFF0000
#define ID_OFF				16


#define COMM_BUFF_SIZE		(128)

unsigned long _buff[COMM_BUFF_SIZE];
BOOL _isAppEnds = FALSE;
size_t _buffReadIndex = 0;
size_t _buffCount = 0;

#define _NEXT_BUFF_INDEX()		((_buffReadIndex + _buffCount) & (COMM_BUFF_SIZE - 1))
#define _THIS_BUFF_INDEX()		(_buffReadIndex)

#define _BUFF_READ_IDX_PP()		++_buffReadIndex; _buffReadIndex &= (COMM_BUFF_SIZE - 1)

static void *_threadProducer(void *arg)
{
	long repeatCount = (((long)arg) & REPEAT_TIME_MASK) >> REPEAT_TIME_OFF;
	long pthreadID = (((long)arg) & ID_MASK) >> ID_OFF;
	int num;

	//pthread_detach(pthread_self());
	AMCLog("Repeat count: %ld", repeatCount);

	while(repeatCount > 0)
	{
		usleep(randInt(MIN_RAND_USEC, MAX_RAND_USEC));

		pthread_mutex_lock(&_mutex);
		{
			num = randInt(100, 200);
			AMCLog("%02ld Fire %03d --->", pthreadID, num);

			_buff[_NEXT_BUFF_INDEX()] = num;
			_buffCount ++;
		}
		pthread_mutex_unlock(&_mutex);

		pthread_cond_broadcast(&_cond);
		
		repeatCount --;
	}

	return NULL;
}


static void *_threadConsumer(void *arg)
{
	//pthread_detach(pthread_self());

	do
	{	
		pthread_mutex_lock(&_mutex);
		{
			pthread_cond_wait(&_cond, &_mutex);

			while (0 != _buffCount)
			{
				AMCLog("            ---> Get: %d, remaining %d", _buff[_THIS_BUFF_INDEX()], _buffCount - 1);

				_BUFF_READ_IDX_PP();
				_buffCount --;
			}
		}
		pthread_mutex_unlock(&_mutex);

		usleep(randInt(MIN_RAND_USEC, 1000000));		/* simulate processing */
	}
	while (FALSE == _isAppEnds);
	

	AMCPrintf ("Recv thread ends.");
	return NULL;
}




/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	pthread_t producer[CFG_PRODUCER_COUNT] = {};
	pthread_t consumer = 0;
	unsigned long tmp;

	srand(time(NULL));

	/**** init producer ****/
	for (tmp = 0; tmp < CFG_PRODUCER_COUNT; tmp ++)
	{
		pthread_create(&(producer[tmp]), NULL, _threadProducer, (void*)(5 + (tmp << ID_OFF)));
	}


	/**** init consumer ****/
	pthread_create(&consumer, NULL, _threadConsumer, NULL);


	/********/
	AMCPrintf ("Now wait for threads.");
	for (tmp = 0; tmp < CFG_PRODUCER_COUNT; tmp ++)
	{
		pthread_join(producer[tmp], NULL);
	}

	AMCPrintf ("Producers ends.");

	pthread_mutex_lock(&_mutex);
	_isAppEnds = TRUE;
	pthread_mutex_unlock(&_mutex);
	//pthread_mutex_lock(&_mutex);
	pthread_cond_signal(&_cond);
	//pthread_mutex_unlock(&_mutex);
	
	pthread_join(consumer, NULL);
	AMCPrintf ("Consumer ends.");
	usleep(100000);

	pthread_mutex_destroy(&_mutex);
	pthread_cond_destroy(&_cond);
	return 0;
}

int main(int argc, char* argv[])
{
	int mainRet;
	
	printf ("\n"CFG_SOFTWARE_DISCRIPT_STR"\n");
	printf ("Version "CFG_SOFTWARE_VER_STR"\n");
	printf ("Author: "CFG_AUTHOR_STR"\n");
	printf ("main() build time: "__TIME__", "__DATE__"\n");
	printf ("----START----\n");

#ifdef	_CFG_ENC_INPUT_STR
	mainRet = _testEncodeMain(argc, argv);
#elif defined(_CFG_DEC_INPUT_STR)
	mainRet = _testDecodeMain(argc, argv);
#else
	mainRet = trueMain(argc, argv);
#endif
	//TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

