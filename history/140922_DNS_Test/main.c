/**/

#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define CFG_LIB_MEM
#define CFG_LIB_DNS
#define CFG_LIB_NET
#define CFG_LIB_SOCKET
#include "AMCCommonLib.h"

#include "AMCTimeMark.h"


void _test_gethostbyname()
{
	hostent_st hostInfo;
	hostent_st *pRet = NULL;
	char *ipv4Addr;
	long tmp;
	char ipStr[INET_ADDRSTRLEN];

	AMC_MARK_TIME();
	pRet = gethostbyname("www.google.com");
	AMC_MARK_TIME();

	if (pRet)
	{
		memcpy(&hostInfo, pRet, sizeof(*pRet));
		printf ("Get host infomation: %s\n", hostInfo.h_name);

		/* Alias */
		for (ipv4Addr = hostInfo.h_aliases[0], tmp = 0; 
			NULL != ipv4Addr;
			ipv4Addr = hostInfo.h_aliases[++tmp])
		{
			printf ("%s\n", ipv4Addr);
		}

		/* IPs */
		for (ipv4Addr = hostInfo.h_addr_list[0], tmp = 0; 
			NULL != ipv4Addr;
			ipv4Addr = hostInfo.h_addr_list[++tmp])
		{
			inet_ntop(hostInfo.h_addrtype, ipv4Addr, ipStr, sizeof(ipStr));
			printf ("%s\n", ipStr);
		}
	}
	else
	{
		printf ("Cannot get host name.\n");
	}

	printf ("----\n");
}


void _test_gethostbyaddr()
{
	hostent_st *pHost = NULL;
	in_addr_st ipAddr;

	inet_aton("8.8.8.8", &ipAddr);
	pHost = gethostbyaddr(&ipAddr, sizeof(ipAddr), AF_INET);

	if (pHost)
	{
		printf ("Get Host name: %s\n", pHost->h_name);
	}
	else
	{
		printf ("Cannot get host name.\n");
	}

	printf ("----\n");
}

void _test_getaddrinfo()
{
	addrinfo_st *addrInfo = NULL;
	addrinfo_st *tmpAddrInfo;
	addrinfo_st hints;
	int error;
#if (INET6_ADDRSTRLEN > INET_ADDRSTRLEN)
	char ipStr[INET6_ADDRSTRLEN];
#else
	char ipStr[INET_ADDRSTRLEN];
#endif

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
		
	error = getaddrinfo("www.google.com", NULL, &hints, &addrInfo);

	if (0 != error)
	{
		errPrintf("Failed to get DNS: %s", gai_strerror(error));
	}
	else
	{
		printf ("Get IPs:\n");
		tmpAddrInfo = addrInfo;
		while (NULL != tmpAddrInfo)
		{
			switch(tmpAddrInfo->ai_family)
			{
				case AF_INET:
					printf ("AF_INET  ");
					inet_ntop(AF_INET, &(((sockaddr_in_st*)(tmpAddrInfo->ai_addr))->sin_addr), ipStr, sizeof(ipStr));
					break;
				case AF_INET6:
					printf ("AF_INET6 ");
					inet_ntop(AF_INET6, &(((sockaddr_in6_st*)(tmpAddrInfo->ai_addr))->sin6_addr), ipStr, sizeof(ipStr));
					break;
				default:
					printf("Unknown family %d\t", tmpAddrInfo->ai_family);
					ipStr[0] = '\0';
					break;
			}
			printf ("%s, family %d\n", ipStr, tmpAddrInfo->ai_family);
		
			tmpAddrInfo = tmpAddrInfo->ai_next;
		}
	}


	if (addrInfo)
	{
		freeaddrinfo(addrInfo);
	}

	printf ("----\n");
}



void _test_strRev(void)
{
	char testStr[] = "1234567890A";
	unsigned long tmp, count;
	char tmpC;
	printf("\n");

	count = strlen(testStr);
	printf ("Orig string: %s (size: %ld)\n", testStr, count);
	for (tmp = 0; (2 * tmp) < count; tmp++)
	{
		tmpC = testStr[tmp];
		testStr[tmp] = testStr[count - tmp - 1];
		testStr[count - tmp - 1] = tmpC;
	}
	printf ("New String:  %s\n\n", testStr);
	
}


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	_test_gethostbyname();
	_test_gethostbyaddr();
	_test_getaddrinfo();
	_test_strRev();

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

