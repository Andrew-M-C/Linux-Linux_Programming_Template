/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCRandom.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file provide implementation of corresponding header file
			
	History:
		2016-07-06: File created

--------------------------------------------------------------
	License information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCRandom.h"

#include <stdlib.h>
#include <time.h>

//#define _DEBUG_FLAG

#ifdef _DEBUG_FLAG
#define CFG_LIB_STDOUT
#define CFG_LIB_RAND
#include "AMCCommonLib.h"
#define _DEBUG(fmt, args...)	AMCPrintf("## "__FILE__", %d: "fmt, __LINE__, ##args)
#else
#define _DEBUG(fmt, args...)
#endif


/****/
/* ahead declarations */

static long _random_long_init(long min, long max);
static int _random_int_init(int min, int max);
static long long _random_longlong_init(long long min, long long max);
static signed char _random_char_init(signed char min, signed char max);
static int8_t _random_8_init(int8_t min, int8_t max);
static int16_t _random_16_init(int16_t min, int16_t max);
static int32_t _random_32_init(int32_t min, int32_t max);
static int64_t _random_64_init(int64_t min, int64_t max);

static void _random_init(void);

static int8_t  _random_8_from_int(int8_t min, int8_t max);
static int16_t _random_16_from_int(int16_t min, int16_t max);
static int32_t _random_32_from_int(int32_t min, int32_t max);
static int32_t _random_32_from_16(int32_t min, int32_t max);
static int32_t _random_32_from_long(int32_t min, int32_t max);
static int64_t _random_64_from_longlong(int64_t min, int64_t max);
static long _random_long_from_long(long min, long max);
static long long _random_longlong_from_long(long long min, long long max);
static int _random_int_from_int(int min, int max);


static const int8_t _g_int8_min = (((uint8_t)0xFF) + 2);
static const int8_t _g_int8_max = 0x7F;
static const int16_t _g_int16_min = (((uint16_t)0xFFFF) + 2);
static const int16_t _g_int16_max = 0x7FFF;
static const int32_t _g_int32_min = (((uint32_t)0xFFFFFFFF) + 2);
static const int32_t _g_int32_max = 0x7FFFFFFF;
static const int64_t _g_int64_min = (((uint64_t)0xFFFFFFFFFFFFFFFF) + 2);
static const int64_t _g_int64_max = 0x7FFFFFFFFFFFFFFF;

static signed char _g_char_min;
static signed char _g_char_max;
static int _g_int_min;
static int _g_int_max;
static long _g_long_min;
static long _g_long_max;
static long long _g_longlong_min;
static long long _g_longlong_max;


/****/
/* function pointers */
static long (*_random_long_func)(long min, long max)						= _random_long_init;
static int (*_random_int_func)(int min, int max)							= _random_int_init;
static long long (*_random_longlong_func)(long long min, long long max)		= _random_longlong_init;
static signed char (*_random_char_func)(signed char min, signed char max)	= _random_char_init;

static int64_t (*_random_64_func)(int64_t min, int64_t max)					= _random_64_init;
static int32_t (*_random_32_func)(int32_t min, int32_t max)					= _random_32_init;
static int16_t (*_random_16_func)(int16_t min, int16_t max)					= _random_16_init;
static int8_t (*_random_8_func)(int8_t min, int8_t max)						= _random_8_init;


static long _random_long_init(long min, long max)
{
	_random_init();
	return (_random_long_func)(min, max);
}


static int _random_int_init(int min, int max)
{
	_random_init();
	return (_random_int_func)(min, max);
}


static long long _random_longlong_init(long long min, long long max)
{
	_random_init();
	return (_random_longlong_func)(min, max);
}


static signed char _random_char_init(signed char min, signed char max)
{
	_random_init();
	return (_random_char_func)(min, max);
}


static int8_t _random_8_init(int8_t min, int8_t max)
{
	_random_init();
	return (_random_8_func)(min, max);
}


static int16_t _random_16_init(int16_t min, int16_t max)
{
	_random_init();
	return (_random_16_func)(min, max);
}


static int32_t _random_32_init(int32_t min, int32_t max)
{
	_random_init();
	return (_random_32_func)(min, max);
}


static int64_t _random_64_init(int64_t min, int64_t max)
{
	_random_init();
	return (_random_64_func)(min, max);
}


static void _random_init()
{
	_DEBUG("%s", __func__);
	
	srand(time(NULL));
	srandom(time(NULL));

	/* check int8_t */
	if (1)
	{
		_random_8_func = _random_8_from_int;
	}

	/* check int16_t */
	if (1)
	{
		_random_16_func = _random_16_from_int;
	}

	/* check int32_t */
	if (2 == sizeof(int))
	{
		_random_32_func = _random_32_from_16;
	}
	else	// (4 == sizeof(int))
	{
		_random_32_func = _random_32_from_int;
	}

	/* check int64_t */
	if (1)
	{
		_random_64_func = _random_64_from_longlong;
	}

	/* check char */
	if (1)
	{
		_g_char_min = _g_int8_min;
		_g_char_max = _g_int8_max;
		_random_char_func = _random_8_from_int;
	}

	/* check int */
	if (2 == sizeof(int))
	{
		_g_int_min = _g_int16_min;
		_g_int_max = _g_int16_max;
		_random_int_func = _random_int_from_int;
	}
	else
	{
		_g_int_min = _g_int32_min;
		_g_int_max = _g_int32_max;
		_random_int_func = _random_int_from_int;
	}

	/* check long */
	if (4 == sizeof(long))
	{
		_g_long_min = _g_int32_min;
		_g_long_max = _g_int32_max;
		_random_long_func = _random_long_from_long;
	}
	else
	{
		_g_long_min = _g_int64_min;
		_g_long_max = _g_int64_max;
		_random_long_func = _random_long_from_long;
	}

	/* check long long */
	if (8 == sizeof(long))
	{
		_g_longlong_min = _g_int64_min;
		_g_longlong_max = _g_int64_max;
		_random_longlong_func = _random_long_from_long;
	}
	else
	{
		_g_longlong_min = _g_int64_min;
		_g_longlong_max = _g_int64_max;
		_random_longlong_func = _random_longlong_from_long;
	}

	/* ENDS */
	return;
}


static int8_t _random_8_from_int(int8_t min, int8_t max)
{
	int ret = _random_int_func((int)min, (int)max);
	return (int8_t)ret;
}


static int16_t _random_16_from_int(int16_t min, int16_t max)
{
	int ret = _random_int_func((int)min, (int)max);
	return (int16_t)ret;
}


static int32_t _random_32_from_int(int32_t min, int32_t max)
{
	int ret = _random_int_func((int)min, (int)max);
	return (int32_t)ret;
}


static int32_t _random_32_from_16(int32_t min, int32_t max)
{
	_DEBUG("%s", __func__);

	uint32_t retU;
	int32_t *ret = (uint32_t*)(&retU);
	uint16_t hi, lo;


	hi = _random_16_func(_g_int16_min, _g_int16_max);
	lo = _random_16_func(_g_int16_min, _g_int16_max);

	retU = lo + (((uint32_t)hi) << 16);
	*ret = min + (int32_t)((float)(max - min + 1) * (float)((*ret) / (_g_int32_max + 1.0)));

	return *ret;
}


static int32_t _random_32_from_long(int32_t min, int32_t max)
{
	int ret = _random_long_func((int)min, (int)max);
	return (int32_t)ret;
}


static int64_t _random_64_from_longlong(int64_t min, int64_t max)
{
	long long ret = _random_longlong_func((long long)min, (long long)max);
	return (int64_t)ret;
}


static long _random_long_from_long(long min, long max)
{
	long result;

	if (min == max)
	{
		result = min;
	}
	else
	{
		if (min > max)
		{
			/* swap */
			result = min;
			min    = max;
			max    = result;
		}

		/* randomize */
		if ((max == _g_long_max)
			&& (min == _g_long_min))
		{
			result = random();
		}
		else
		{
			result = min + (long)((double)(max - min + 1) * (double)(random() / (_g_long_max + 1.0)));
		}
	}
	_DEBUG("%s: (%ld ~ %ld) = %ld", __func__, min, max, result);
	return result;
}


static long long _random_longlong_from_long(long long min, long long max)
{
	unsigned long long retU;
	long long *ret = (long long*)(&retU);
	unsigned long hi, lo;

	hi = _random_long_func(_g_long_min, _g_long_max);
	lo = _random_long_func(_g_long_min, _g_long_max);

	retU = lo + (((unsigned long long)hi) << 32);
	*ret = min + (long long)((double)(max - min + 1) * (double)((*ret) / (_g_long_max + 1.0)));

	return *ret;
}


static int _random_int_from_int(int min, int max)
{
	_DEBUG("%s", __func__);
	int result;

	if (min == max)
	{
		result = min;
	}
	else
	{
		if (min > max)
		{
			/* swap */
			result = min;
			min    = max;
			max    = result;
		}

		if ((max == _g_int_max)
			&& (min == _g_int_min))
		{
			result = rand();
		}
		else
		{
			result = min + (int)((float)(max - min + 1) * (float)(rand() / (_g_int_max + 1.0)));
		}
	}
	_DEBUG("%s: (%d ~ %d) = %d", __func__, min, max, result);
	return result;
}


long AMCRandomLong(long min, long max)
{
	return _random_long_func(min, max);
}


int AMCRandomInt(int min, int max)
{
	return _random_int_func(min, max);
}
	
long long AMCRandomLongLong(long min, long max)
{
	return _random_longlong_func(min, max);
}


signed char AMCRandomChar(signed char min, signed char max)
{
	return _random_char_func(min, max);
}


int64_t AMCRandom64(int64_t min, int64_t max)
{
	return _random_64_func(min, max);
}


int32_t AMCRandom32(int32_t min, int32_t max)
{
	return _random_32_func(min, max);
}


int16_t AMCRandom16(int16_t min, int16_t max)
{
	return _random_16_func(min, max);
}


int8_t AMCRandom8(int8_t min, int8_t max)
{
	return _random_8_func(min, max);
}



