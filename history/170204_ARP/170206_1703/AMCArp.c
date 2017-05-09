/*******************************************************************************
	Copyright (C), 2011-2016, Andrew Min Chang

	File name: 	AMCArp.c
	Author:		Andrew Chang (Zhang Min) 
	
	Description: 	
			This file implements ARP send and listen tools.
			
	History:
		2017-02-04: File created as AMCArp.c

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

/********/
#define __HEADERS
#ifdef __HEADERS

#include "AMCArp.h"
#include "AMCMemPool.h"

#define DEBUG
#include "AMCCommonLib.h"

#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#include <netpacket/packet.h>
#include <sys/fcntl.h>
#include <net/ethernet.h>
#include <asm/types.h>
#include <netinet/if_ether.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#endif


/********/
#define __PUBLIC_INTERFACES
#ifdef __PUBLIC_INTERFACES




#endif

/********/
/* end of file */

