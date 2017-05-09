/**/

#define DEBUG
#define CFG_LIB_STDOUT
#define CFG_LIB_TIME
#define CFG_LIB_SOCKET
#define CFG_LIB_DEVICE
#define CFG_LIB_FILE
#define CFG_LIB_ERRNO
#define CFG_LIB_DNS
#define CFG_LIB_NET
#define CFG_LIB_STRING
#include "AMCCommonLib.h"

#include "AMCLibevTools.h"
#include <openssl/ssl.h>
#include <openssl/err.h>

#define _CFG_HTTP_REQ_LEN	(2048)
static const char *g_httpRequestFormat = ""
	"GET / HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
	"Upgrade-Insecure-Requests: 1\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/47.0.2526.73 Safari/537.36 OPR/34.0.2036.25\r\n"
	"DNT: 1\r\n"
	"Accept-Encoding: gzip, deflate, lzma, sdch\r\n"
	"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n";

static char g_readBuff[4096];		// decrypt error
//static char g_readBuff[65536];

typedef struct SSL_SESSION {
	char                officalUrl[128];
	struct AMC_ev_io   *libevWatcher;
	struct sockaddr_in  serverIpInfo;
	SSL_CTX            *sslCtx;
	SSL                *sslSsl;
	int                 fileOut;
} SSLSession_st;


/**********/
/* Libev callback */
static void _libev_callback(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	ssize_t readLen = 0;
	int errnoCopy = 0;
	SSLSession_st *pSession = (SSLSession_st *)(((struct AMC_ev_io *)(watcher))->arg);

	if (EV_READ & revents) {
		AMCPrintf("EV_READ");

		/* main read function */
		readLen = SSL_read(pSession->sslSsl, g_readBuff, sizeof(g_readBuff));
		while (readLen > 0)
		{
			AMCDataDump(g_readBuff, readLen);

			if (pSession->fileOut > 0) {
				write(pSession->fileOut, g_readBuff, readLen);
			}

			readLen = SSL_read(pSession->sslSsl, g_readBuff, sizeof(g_readBuff));
		}

		errnoCopy = errno;
		if (0 == readLen)
		{
			AMCPrintf("Read EOF, we should close this socket");
			ev_io_stop(loop, watcher);
		}
		else if (readLen < 0)
		{
			if (EAGAIN == errnoCopy)
			{
				AMCPrintf("No further data to read.");
			}
			else if (ECONNRESET == errnoCopy)
			{
				AMCPrintf("Current %s, close this socket.", strerror(errnoCopy));
				ev_io_stop(loop, watcher);
			}
			else
			{
				AMCPrintErr("Read from socket error: %s", strerror(errnoCopy));
			}
		}
		else
		{
			/* OK */
		}
	}
	if (EV_TIMEOUT & revents) {
		AMCPrintf("EV_TIMEOUT");
	}
	if (EV_WRITE & revents) {
		AMCPrintf("EV_WRITE");
	}
	if (1) {
		AMCPrintf("revents: 0x%02X\n", revents);
	}
	
	return;
}


/**********/
/* main */
static int trueMain(int argc, char* argv[])
{
	if (argc < 2) {
		return -1;
	}

	char *para = argv[1];
	char *protocol = "https";

	int callStat = 0;
	struct addrinfo *dnsResult = NULL;
	SSLSession_st session;

	memset(&session, 0, sizeof(session));
	struct ev_loop *loop = NULL;

	if (argc > 2) {
		session.fileOut = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC);
	}
	else {
		session.fileOut = -1;
	}
	

	/****/
	/* resolve DNS */
	strncpy(session.officalUrl, para, sizeof(session.officalUrl));

	callStat = AMCDnsGetAddrinfo(para, protocol, &dnsResult);
	if (callStat < 0) {
		AMCPrintErr("Cannot resolve hostname \"%s\": %s", para, gai_strerror(callStat));
		goto ENDS;
	}
	else {
		AMCPrintf("Get addrinfo of %s:", para);
		AMCDnsDumpAddrinfo(dnsResult);
	}

	/****/
	/* fetch valid IP address */
	{
		struct addrinfo *pDnsInfo = dnsResult;
	
		/* read port from dns info */
		while (pDnsInfo
				&& (AF_INET != pDnsInfo->ai_family)
				&& (SOCK_STREAM != pDnsInfo->ai_protocol))
		{
			if (pDnsInfo->ai_canonname) {
				strncpy(session.officalUrl, pDnsInfo->ai_canonname, sizeof(session.officalUrl));
			}
			pDnsInfo = pDnsInfo->ai_next;
		}

		if (NULL == pDnsInfo) {
			AMCPrintErr("No valid IPv4 TCP server.");
			goto ENDS;
		}
		else {
			struct sockaddr_in *pSockinfo = (struct sockaddr_in *)(pDnsInfo->ai_addr);
			memcpy(&(session.serverIpInfo), pSockinfo, sizeof(*pSockinfo));

			if (pDnsInfo->ai_canonname) {
				strncpy(session.officalUrl, pDnsInfo->ai_canonname, sizeof(session.officalUrl));
			}
		}
	}

	/****/
	/* configure libev */
	{
		/* init libev lib */
		loop = ev_loop_new(EVFLAG_AUTO);
		session.libevWatcher = AMCEvIo_new();
		
		if ((NULL == loop) || (NULL == session.libevWatcher)) {
			AMCPrintErr("Cannot initialize libev");
			goto ENDS;
		}

		/* assign parameters */
		session.libevWatcher->arg = &session;
	}

	/****/
	/* create socket */
	{
		struct timeval timeout = {.tv_sec = 30, .tv_usec = 0};
		char ipAddrStr[32];
		int callStat = 0;
		int fd = -1;
		
		fd = AMCSocketCreate_tcp();
		if (fd < 0) {
			AMCPrintErr("Cannot create socket: %s", strerror(errno));
			goto ENDS;
		}

		//AMCFdSetNonBlock(fd);		// should not set nonblock in openssl
		inet_n4top(session.serverIpInfo.sin_addr, ipAddrStr, sizeof(ipAddrStr));

		AMCPrintf("Now start connect %s(%s)", ipAddrStr, session.officalUrl);
		
		callStat = AMCSocketConnect_internet(fd, ipAddrStr, ntohs(session.serverIpInfo.sin_port), &timeout);
		if (callStat < 0) {
			AMCPrintErr("Cannot connect to target: %s", strerror(errno));
			goto ENDS;
		}
		else {
			AMCPrintf("Connect to %s:%d OK", para, ntohs(session.serverIpInfo.sin_port));
		}

		/* use libev */
		session.libevWatcher->watcher.fd = fd;
		ev_io_init(&(session.libevWatcher->watcher), _libev_callback, fd, EV_READ);
		ev_io_start(loop, &(session.libevWatcher->watcher));
	}

	/****/
	/* init and set SSL */
	{
		int callStat = 0;
		SSL_library_init();
		SSL_load_error_strings();
	
		session.sslCtx = SSL_CTX_new(SSLv23_client_method());
		if (NULL == session.sslCtx) {
			AMCPrintErr("Failed to init CTX");
			goto ENDS;
		}

		session.sslSsl = SSL_new(session.sslCtx);
		if (NULL == session.sslCtx) {
			AMCPrintErr("Failed to init SSL");
			goto ENDS;
		}

		AMCPrintf("Socket: %d", session.libevWatcher->watcher.fd);
		callStat = SSL_set_fd(session.sslSsl, session.libevWatcher->watcher.fd);
		if (FALSE == callStat) {
			AMCPrintErr("Failed to set fd in SSL");
			ERR_print_errors_fp(stderr);
			goto ENDS;
		}
	}

	/****/
	/* send request and run */
	{
		ssize_t callStat = 0;
		char request[_CFG_HTTP_REQ_LEN] = "";
		snprintf(request, sizeof(request), g_httpRequestFormat, para);

		errno = 0;
		callStat = SSL_connect(session.sslSsl);
		if (-1 == callStat) {
			AMCPrintErr("Failed to connect SSL: %s", strerror(errno));
			ERR_print_errors_fp(stderr);
			goto ENDS;
		}

		X509 *x509 = SSL_get_peer_certificate(session.sslSsl);
		X509_NAME *x509Name = X509_get_subject_name(x509);
		AMCPrintf("Certificate owner:");
		X509_NAME_print_ex_fp(stdout, x509Name, '\n', 0);
		printf("\n");

		callStat = SSL_write(session.sslSsl, request, strlen(request) + 1);
		if (callStat <= 0) {
			AMCPrintErr("Failed to send SSL request");
			goto ENDS;
		}
		
		ev_loop(loop, 0);
	}

	/****/
	/* exit this process */
ENDS:
	// TODO: Deallocate all memory

	AMCDnsFreeAddrinfo(dnsResult);
	dnsResult = NULL;
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

	mainRet = trueMain(argc, argv);
	TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

