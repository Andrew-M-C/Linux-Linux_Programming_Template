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
#define CFG_LIB_SIGNAL
#include "AMCCommonLib.h"

#include "AMCLibevTools.h"
#include "AMCMbedTLSTools.h"

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

static unsigned char g_readBuff[4096];		// decrypt error

typedef struct SSL_SESSION {
	struct AMC_ev_io    *libevWatcher;
	int                  fileOut;
	AMCMbedTLSSession_st mbedIntf;
} SSLSession_st;

static SSLSession_st *g_session = NULL;

/**********/
/* signal callback */
static void _signal_handler(int signum)
{
	switch(signum)
	{
		case SIGTERM:
		case SIGINT:
			AMCPrintf("Got kill signal, should exit");
			if (g_session) {
				AMCMbedTlsClientStop(&(g_session->mbedIntf));
				g_session = NULL;
			}
			else {
				exit(127);
			}
			break;
	
		default:
			AMCPrintf("Unsupported signal: %d", signum);
			break;
	}
}


static void _register_signal(int signum, sighandler_t act)
{
	int callStat;
	struct sigaction action;

	sigemptyset(&(action.sa_mask));
	action.sa_handler = act;
	action.sa_flags   = 0;
	callStat = sigaction(signum, &action, NULL);
}


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
		while ((readLen = AMCMbedTlsClientRead(&(pSession->mbedIntf), g_readBuff, sizeof(g_readBuff))) > 0)
		{
			AMCDataDump(g_readBuff, readLen);

			if (pSession->fileOut > 0) {
				write(pSession->fileOut, g_readBuff, readLen);
			}
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
			else if ((ECONNRESET == errnoCopy)
					|| (EINTR == errnoCopy)
					|| (ERESTART == errnoCopy))
			{
				AMCPrintf("%s, close this socket.", strerror(errnoCopy));
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
	/* init signal catcher */
	{
		_register_signal(SIGTERM, _signal_handler);
		_register_signal(SIGINT,  _signal_handler);
		g_session = &session;
	}

	/****/
	/* init and set SSL */
	{
		int callStat = 0;
		char *pers = "hello world";

		callStat = AMCMbedTlsClientInit(&(session.mbedIntf), pers, strlen(pers));
		if (0 != callStat) {
			AMCPrintErr("Cannot init mbedTLS client: %d", callStat);
			goto ENDS;
		}
		AMCPrintf("Init mbedTLS OK");

		callStat = AMCMbedTlsClientConnect(&(session.mbedIntf), para, 443);
		if (0 != callStat) {
			AMCPrintErr("Cannot connect remote server: %d", callStat);
			goto ENDS;
		}
		AMCPrintf("Connect to %s OK", para);
	}
	/****/
	/* send request and run */
	{
		ssize_t callStat = 0;
		char request[_CFG_HTTP_REQ_LEN] = "";
		callStat= snprintf(request, sizeof(request), g_httpRequestFormat, para);

		AMCPrintf("Data to send:");
		AMCDataDump(request, callStat + 1);
		
		callStat = AMCMbedTlsClientWrite(&(session.mbedIntf), (unsigned char *)request, strlen(request) + 1);
		if (callStat < 0) {
			AMCPrintErr("Cannot write data: %s", callStat);
			goto ENDS;
		}
		AMCPrintf("Send request OK");

		/* setup libev */
		loop = ev_loop_new(EVFLAG_AUTO);
		session.libevWatcher = AMCEvIo_new();
		
		if ((NULL == loop) || (NULL == session.libevWatcher)) {
			AMCPrintErr("Cannot initialize libev");
			goto ENDS;
		}
		AMCPrintf("Setup libev OK");

		/* assign parameters */
		int fd = AMCMbedTlsClientGetFd(&(session.mbedIntf));
		session.libevWatcher->arg = &session;

		AMCFdSetNonBlock(fd);
		session.libevWatcher->watcher.fd = fd;
		ev_io_init(&(session.libevWatcher->watcher), _libev_callback, fd, EV_READ);
		ev_io_start(loop, &(session.libevWatcher->watcher));
		
		ev_loop(loop, 0);
	}
	
	/****/
	/* exit this process */
ENDS:
	AMCMbedTlsClientStop(&(session.mbedIntf));
	
	AMCEvWatcher_free(session.libevWatcher);
	session.libevWatcher = NULL;

	g_session = NULL;
	AMCPrintf("Application ends");
	return 0;
}

static int _test_main(void)
{
	TEST_VALUE(sizeof(double));
	TEST_VALUE(sizeof(float));

	if (AMCSysIsBigEndian()) {
		AMCPrintf("System big-endian");
	}
	else {
		AMCPrintf("System small-endian");
	}

	float testFloat1 = 100.2;
	float testFloat2 = 100.0;
	float testFloat3 = 0.2;

	TEST_BIN(testFloat1);
	TEST_BIN(testFloat2);
	TEST_BIN(testFloat3);

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

	//mainRet = trueMain(argc, argv);
	mainRet = _test_main();
	TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

