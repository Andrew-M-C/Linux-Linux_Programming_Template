/**/

#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_STRING
#define	CFG_LIB_RAND
#define	CFG_LIB_TIME
#define	CFG_LIB_SOCKET
#define	CFG_LIB_ERRNO
#define	CFG_LIB_NET
#define	CFG_LIB_SIGNAL
#define	CFG_LIB_SYSTEM
#define	CFG_LIB_DEVICE
#include "AMCCommonLib.h"

#include "AMCTimeMark.h"

#include "ddp.h"


#ifndef	_MD5_LEN
#define	_MD5_LEN		16
#endif


void _printUsage(const char *appName)
{
	printf ("Usage:\n");
	printf ("\t%s FILE      Start server with bin file\n", appName);
	printf ("\t%s -h        Print this usage.\n", appName);
}





static int *_pTheSocket = NULL;
void _sigIntProcess(int signum)
{
	printf ("\nCaught Ctrl+C.\n");
	simpleSocketClose(*_pTheSocket);
	*_pTheSocket = 0;
	_pTheSocket = NULL;

	return;
}




/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	int sockFd;
	long tmp, ipCount, idxOfSubnet;
	int callStat = 0;
	int errnoCopy;
	uint8_t recvBuffer[2048];
	size_t recvLen = 0;
	sockaddr_in_st pktInfo;
	ifaddrs_st *pIfAddrs = NULL;
	in_addr_st ipv4Addrs[64];
	in_addr_st ipv4Masks[64];		
	AMCDdpErrno_t ddpErrno;
	AMCDdpFileService_st fileSvc;

	char ipStrBuffer[16];
	char maskStrBuffer[16];


	/****/
	/* initialize file */
	if (((argc > 1) && (0 == strcmp(argv[1], "-h"))) ||
		(argc < 2))
	{
		_printUsage(argv[0]);
		return 0;
	}
	else if (argc > 1)
	{
		
		AMCDdpOpenFileService(&fileSvc, argv[1]);

		if (fileSvc.file)
		{
			printf ("Start server with file \"%s\", file size %0.3fkB.\n", argv[1], ((float)(fileSvc.fileSize)) / 1024.0);
		}
		else
		{
			errnoCopy = errno;
			printf ("Failed to load file: %s.\nNow start server without file.\n", 
					strerror(errnoCopy));
		}
	}	
	
	/****/
	/* check interface */
	getifaddrs(&pIfAddrs);
	ipCount = ifaddrsGetAllIPv4(pIfAddrs, ipv4Addrs, ipv4Masks, 64);

	for (tmp = 0; tmp < ipCount; tmp++)
	{
		inet_n4top(ipv4Addrs[tmp], ipStrBuffer, 16);
		inet_n4top(ipv4Masks[tmp], maskStrBuffer, 16);

		printf ("IPAddress: %s (%s)\n", ipStrBuffer, maskStrBuffer);
	}
	
	/****/
	/* read and react with data */
	printf ("Initialize server.\n");
	sockFd = simpleSocketCreate_udp();
	if (sockFd)
	{
		_pTheSocket = &sockFd;
		simpleSigaction(SIGINT, _sigIntProcess);
		callStat = simpleSocketBind_udp(sockFd, CFG_LOCAL_PORT);
	}

	while (0 == callStat)
	{
		AMC_MARK_TIME();
		printf ("\n----------------\nListerning...\n");

		callStat = simpleSocketReceiveFrom_udp(sockFd, NULL, &pktInfo, recvBuffer, sizeof(recvBuffer), NULL);

		if (callStat >= 0)
		{
			recvLen = callStat;
			callStat = 0;
			printf ("Get data:\n");
			//printData(recvBuffer, recvLen);
			printf ("From %s:%d\n", inet_ntoa(pktInfo.sin_addr), ntohs(pktInfo.sin_port));
		}
		else
		{
			errnoCopy = errno;
			errPrintf ("Failed to receive: %s\n", strerror(errnoCopy));
		}

		/****/
		/* check where does the packet come from */
		if (0 == callStat)
		{
			uint32_t tmpAddr, targetAddr;
			targetAddr = pktInfo.sin_addr.s_addr;
				
			for (idxOfSubnet = 0; idxOfSubnet < ipCount; idxOfSubnet++)
			{
				tmpAddr = ipv4Addrs[idxOfSubnet].s_addr;

				/* check if in the same subnet */
				if ((targetAddr & ipv4Masks[idxOfSubnet].s_addr) == (tmpAddr & ipv4Masks[idxOfSubnet].s_addr))
				{
					break;
				}
			}

			/* assign to handler */
			if (idxOfSubnet < ipCount)
			{
				inet_n4top(ipv4Addrs[idxOfSubnet], ipStrBuffer, 16);
				callStat = 0;
				printf ("Out IP %s shares the same subnet with target.\n", ipStrBuffer);

				ddpErrno = AMCDdpReactWithData((AMCDdpContext_st*)recvBuffer, sockFd, &fileSvc, pktInfo.sin_port, pktInfo.sin_addr, ipv4Addrs[idxOfSubnet]);
				if (AMCDdpErrno_Success != ddpErrno)
				{
					errPrintf("Failed in AMCDDP: %s\n", AMCDdpStrError(ddpErrno));
					callStat = -1;
				}
				else
				{
					/* contnue */
				}
			}
			else
			{
				errPrintf("No local IP subnet match, ignore it.\n");
			}
		}
	}
	
	/****/
	/* ends */
	AMCDdpCloseFileService(&fileSvc);
	
	if (sockFd)
	{
		simpleSocketClose(sockFd);
		sockFd = 0;
	}

	if (pIfAddrs)
	{
		freeifaddrs(pIfAddrs);
		pIfAddrs = NULL;
	}

	if (callStat < 0)
	{
		if (NULL == _pTheSocket)
		{
			printf ("User request terminate the application.\n");
		}
		else
		{
			errnoCopy = errno;
			errPrintf("Failed in socket operation: %s\n", strerror(errnoCopy));
		}
	}

	_pTheSocket = NULL;
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
	//TEST_VALUE(mainRet);
	
	printf ("----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}

