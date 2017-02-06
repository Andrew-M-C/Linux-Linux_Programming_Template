/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCMbedTLSTools.h
	Description: 	
			This file provides simple interface for mbed TLS.  
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-05-10: File created as "AMCMbedTLSTools.h"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#ifndef _AMC_MBEDTLS_TOOLS_H
#define _AMC_MBEDTLS_TOOLS_H

#include <mbedtls/net.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

#include <stdio.h>

typedef struct AMC_MBEDTLS_SESSION {
	mbedtls_net_context      mbedNetCtx;
	mbedtls_ssl_context      mbedSslCtx;
	mbedtls_ssl_config       mbedSslConf;
	mbedtls_ctr_drbg_context mbedDrbgCtx;
	mbedtls_entropy_context  mbedEtpyCtx;
	mbedtls_x509_crt         mbedX509Crt;
} AMCMbedTLSSession_st;


int AMCMbedTlsClientInit(AMCMbedTLSSession_st *session, void *entropy, size_t entropyLen);
int AMCMbedTlsClientStop(AMCMbedTLSSession_st *session);

int AMCMbedTlsClientGetFd(AMCMbedTLSSession_st *session);

int AMCMbedTlsClientConnect(AMCMbedTLSSession_st *session, const char *serverHost, int serverPort);

int AMCMbedTlsClientRead(AMCMbedTLSSession_st *session, void *buff, size_t readLen);
int AMCMbedTlsClientWrite(AMCMbedTLSSession_st *session, const void *buff, size_t writeLen);

int AMCMbedTlsClientSetNonblock(AMCMbedTLSSession_st *session);
int AMCMbedTlsClientSetBlock(AMCMbedTLSSession_st *session);

#endif
