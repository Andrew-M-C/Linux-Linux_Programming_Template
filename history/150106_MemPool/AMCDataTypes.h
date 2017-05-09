/*******************************************************************************
	Copyright (C), 2011-2012, Andrew Min Chang
	
	File name: AMCDataTypes.h
	Description: 	
			This file provides basic data types. This is quite useful in embedded systems. Data
		types defined here are not leaded with "AMC" prefix.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2012-10-10: File created as "AMCDataTypes.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my work as I am an employee. And my company may require me to keep it secret. 
		Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/
#ifndef	_AMC_DATA_TYPES
#define	_AMC_DATA_TYPES


/* basic data types */
// boolean
typedef unsigned char	bool_t;
// unsigned types
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;
typedef unsigned long long	uint64_t;
// signed types
typedef signed char	sint8_t;
typedef signed short	sint16_t;
typedef signed int		sint32_t;
typedef signed long long	sint64_t;
/* data types below is defined by basic data types above */
// register data types
typedef uint32_t	regval_t;
// temporary counter types
typedef uint32_t	utmp8_t;
typedef uint32_t	utmp16_t;
typedef uint32_t	utmp32_t;
typedef uint64_t	utmp64_t;
typedef sint32_t	stmp8_t;
typedef sint32_t	stmp16_t;
typedef sint32_t	stmp32_t;
typedef sint64_t	stmp64_t;
// normal data type used to return function status
typedef int	retval_t;

/* boolean judgement */
#define	FALSE	(0)
#define	TRUE	(!FALSE)
#define	IS_TRUE(x)		(0 == (x))
#define	IS_FALSE(x)		(0 != (x))
#define	SET_TRUE(x)		((x) = 0)
#define	SET_FALSE(x)	((x) = (-1))

/* bit mask judgement */
#define IS_ALL_BITS_SET(val, bits)	((bits) == ((val) & (bits)))
#define IS_ANY_BITS_SET(val, bits)	(0 != ((val) & (bits)))
#define IS_ALL_BITS_CLR(val, bits)	(!IS_ANY_BITS_SET(val, bits))
#define IS_ANY_BITS_CLR(val, bits)	(!IS_ALL_BITS_SET(val, bits))
#define SET_BITS(val, bits)	((val) |= (bits))
#define CLR_BITS(val, bits)	((val) &= (~(bits)))



#endif
