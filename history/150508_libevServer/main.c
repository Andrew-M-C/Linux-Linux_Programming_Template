/**/

#define DEBUG
#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#define CFG_LIB_FILE
#define CFG_LIB_RAND
#define CFG_LIB_TIME
#define CFG_LIB_THREAD
#define CFG_LIB_SLEEP
#define CFG_LIB_STRING
#define CFG_LIB_ERRNO
#define CFG_LIB_NET
#define CFG_LIB_DEVICE
#include "AMCCommonLib.h"
#include "AMCDataTypes.h"

#include "AMCLibevTools.h"

#define _PORT	80


static void _cbRead(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	char buff[2048] = "";
	int callStat, errCopy;
	struct sockaddr_in *clientAddr = ((struct AMC_ev_io*)watcher)->arg;
	char clientIpStr[16] = "";
	inet_n4top(clientAddr->sin_addr, clientIpStr, sizeof(clientIpStr));
	static char resp[] = 
		"<!DOCTYPE html><html><head><title>Hello!</title></head><body>\r"
		"<a href=\"http://www.bing.com\">Bing</a>\r"
		"</body></html>";

	AMCPrintf("Get revents: 0x%x", revents);

	// read event
	if (revents & EV_READ)
	{
		callStat = read(watcher->fd, buff, sizeof(buff));
		if (callStat > 0)
		{
			AMCPrintf("Got request from %s(%d):", clientIpStr, watcher->fd);
			printData(buff, callStat);
			callStat = 0;

			callStat = write(watcher->fd, resp, sizeof(resp));

			AMCPrintf("Write ends, now close the connection.");
			ev_io_stop(loop, watcher);
			AMCEvWatcher_free(watcher);
		}
		else if (0 == callStat)
		{
			AMCPrintf("Got EOF from %s(%d). Now close this connection.", clientIpStr, watcher->fd);
			ev_io_stop(loop, watcher);
			AMCEvWatcher_free(watcher);
		}
		else
		{
			errCopy = errno;
			AMCPrintErr("Read error: %s", strerror(errCopy));
		}
	}
	else
	{
		callStat = 0;
	}

	return;
}

static void _cbAccept(struct ev_loop *loop, struct ev_io *watcher, int revents)
{
	struct sockaddr_in clientAddr;
	socklen_t addrLen = sizeof(clientAddr);
	int clientFd;
	char clientIpStr[16] = "";
	struct AMC_ev_io *cliendEvIo = AMCEvIo_new(sizeof(clientAddr));

	/*if (revents & EV_RRROR)
	{
		AMCPrintErr("Got invalid event");
		AMCEvWatcher_free(cliendEvIo);
		return;
	}*/

	clientFd = accept(watcher->fd, &clientAddr, &addrLen);
	if (clientFd < 0)
	{
		AMCPrintErr("Cannot accept client connection");
		AMCEvWatcher_free(cliendEvIo);
		return;
	}

	inet_n4top(clientAddr.sin_addr, clientIpStr, sizeof(clientIpStr));
	memcpy(cliendEvIo->arg, &clientAddr, addrLen);
	AMCPrintf("Accept socket: %s", clientIpStr);

	ev_io_init(&(cliendEvIo->watcher), _cbRead, clientFd, EV_READ);
	ev_io_start(loop, &(cliendEvIo->watcher));
	return;
}

/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	struct ev_loop *loop = ev_loop_new(EVFLAG_AUTO);
	struct AMC_ev_io *watcher = AMCEvIo_TcpListenNew(_PORT, 2, 0);
	int errorLine = 0;

	if (loop && watcher)
	{}
	else
	{
		errorLine = __LINE__;
	}
	AMCPrintf("MARK");
	//
	if (0 == errorLine)
	{
		ev_io_init(&(watcher->watcher), _cbAccept, watcher->watcher.fd, EV_READ);
		ev_io_start(loop, &(watcher->watcher));
	}
	AMCPrintf("MARK");
	ev_loop(loop, 0);
	AMCPrintf("Loop exit\n");

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

