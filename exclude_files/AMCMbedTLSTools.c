/*******************************************************************************
	Copyright (C), 2011-2015, Andrew Min Chang
	
	File name: AMCMbedTLSTools.c
	Description: 	
			This file provides implementation for corresponding header.
			
	Author:		Andrew Chang (Zhang Min) 
	History:
		2015-05-10: File created as "AMCMbedTLSTools.c"

--------------------------------------------------------------
	Copyright information: 
			This file was intended to be under GPL protocol. However, I may use this library
		in my working as I am an employee. And my company may require me to keep it se-
		cret. Therefore, this file is neither open source nor under GPL control. 
		
********************************************************************************/

#include "AMCMbedTLSTools.h"

#ifndef NULL
#define NULL	((void*)0)
#endif

#define _RETURN_IF_ERROR(callStat)		do{if(0 != callStat){return callStat;}}while(0)

int AMCMbedTlsClientInit(AMCMbedTLSSession_st *session, void *entropy, size_t entropyLen)
{
	if (NULL == session) {
		return -1;
	}
	
	int callStat;

	mbedtls_net_init(&(session->mbedNetCtx));
	mbedtls_ssl_init(&(session->mbedSslCtx));
	mbedtls_ssl_config_init(&(session->mbedSslConf));
	mbedtls_ctr_drbg_init(&(session->mbedDrbgCtx));
	mbedtls_x509_crt_init(&(session->mbedX509Crt));

	mbedtls_entropy_init(&(session->mbedEtpyCtx));
	callStat = mbedtls_ctr_drbg_seed(&(session->mbedDrbgCtx), mbedtls_entropy_func, &(session->mbedEtpyCtx), (unsigned char *)entropy, entropyLen);
	_RETURN_IF_ERROR(callStat);
	
	return 0;
}


int AMCMbedTlsClientStop(AMCMbedTLSSession_st *session)
{
	if (session)
	{
		mbedtls_net_free(&(session->mbedNetCtx));
		mbedtls_ssl_free(&(session->mbedSslCtx));
		mbedtls_ssl_config_free(&(session->mbedSslConf));
		mbedtls_ctr_drbg_free(&(session->mbedDrbgCtx));
		mbedtls_entropy_free(&(session->mbedEtpyCtx));
		mbedtls_x509_crt_free(&(session->mbedX509Crt));
		return 0;
	}
	else
	{
		return -1;
	}
}


int AMCMbedTlsClientConnect(AMCMbedTLSSession_st *session, const char *serverHost, int serverPort)
{
	if (NULL == session) {
		return -1;
	}

	int callStat;
	char portStrBuff[16];

	snprintf(portStrBuff, sizeof(portStrBuff), "%d", serverPort);
	callStat = mbedtls_net_connect(&(session->mbedNetCtx), serverHost, portStrBuff, MBEDTLS_NET_PROTO_TCP);
	_RETURN_IF_ERROR(callStat);

	callStat = mbedtls_ssl_config_defaults(&(session->mbedSslConf), MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
	_RETURN_IF_ERROR(callStat);

	mbedtls_ssl_conf_authmode(&(session->mbedSslConf), MBEDTLS_SSL_VERIFY_OPTIONAL);
	mbedtls_ssl_conf_ca_chain(&(session->mbedSslConf), &(session->mbedX509Crt), NULL);

	mbedtls_ssl_conf_rng(&(session->mbedSslConf), mbedtls_ctr_drbg_random, &(session->mbedDrbgCtx));
	//edtls_ssl_conf_dbg(&(session->mbedSslConf), NULL, NULL);

	mbedtls_ssl_set_bio(&(session->mbedSslCtx), &(session->mbedNetCtx), mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

	callStat = mbedtls_ssl_setup(&(session->mbedSslCtx), &(session->mbedSslConf));
	_RETURN_IF_ERROR(callStat);

	while((callStat = mbedtls_ssl_handshake(&(session->mbedSslCtx))) != 0)
	{
		if ((callStat != MBEDTLS_ERR_SSL_WANT_READ)
			&& (callStat != MBEDTLS_ERR_SSL_WANT_WRITE))
		{
			return callStat;
		}
	}

	return 0;
}


int AMCMbedTlsClientGetFd(AMCMbedTLSSession_st *session)
{
	if (session) {
		return session->mbedNetCtx.fd;
	}
	else {
		return -1;
	}
}


int AMCMbedTlsClientRead(AMCMbedTLSSession_st *session, void *buff, size_t readLen)
{
	if (session) {
		return mbedtls_ssl_read(&(session->mbedSslCtx), (unsigned char *)buff, readLen);
	}
	else {
		return -1;
	}
}


int AMCMbedTlsClientWrite(AMCMbedTLSSession_st *session, const void *buff, size_t writeLen)
{
	if (session) {
		return mbedtls_ssl_write(&(session->mbedSslCtx), (const unsigned char *)buff, writeLen);
	}
	else {
		return -1;
	}
}


int AMCMbedTlsClientSetNonblock(AMCMbedTLSSession_st *session)
{
	return mbedtls_net_set_nonblock(&(session->mbedNetCtx));
}


int AMCMbedTlsClientSetBlock(AMCMbedTLSSession_st *session)
{
	return mbedtls_net_set_nonblock(&(session->mbedNetCtx));
}



