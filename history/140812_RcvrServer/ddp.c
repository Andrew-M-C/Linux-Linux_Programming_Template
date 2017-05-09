/**************************************************
 * File:        ddp.c
 * Create date: 2014-08-15
 * Author:      Andrew Chang
 */

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_NET
#define	CFG_LIB_SOCKET
#define	CFG_LIB_STRING
#define	CFG_LIB_FILE
#define	CFG_LIB_ERRNO
#define	CFG_LIB_MMAP
#define	CFG_LIB_DEVICE
#include "AMCCommonLib.h"

#include "ddp.h"
#include "AMCmd5.h"


const char *_strErrno[] = {
	"Operation Success",
	"Operation Failed",
	"Parameter error",
	"Network data invalid",
	"Package type not supported",
	"Package option not supported",
	"File too large",
	"Failed in sending socket",
	"Failed to read file status",
	"File service struct invalid",

	/* undefined error */
	"Undefined error"
};


#ifdef CFG_ENABLE_MULTI_THREAD
#define	STATIC	static
#else
#define	STATIC
#endif



/**********/
/* private function deciarations */
AMCDdpErrno_t _handleSearchUtility(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff);
AMCDdpErrno_t _handleFileSizeRequest(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc);
AMCDdpErrno_t _handleFileRequest	(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc);
AMCDdpErrno_t _handleFileMd5DigestRequest(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc);
size_t _sizeofDdpPkg(const AMCDdpContext_st *data);
int _getMd5DigestWithfile(const char *file, unsigned char *md5);



AMCDdpErrno_t AMCDdpOpenFileService(AMCDdpFileService_st *pService, const char *filePath)
{
	struct stat fileStat;
	int callStat = 0;
	unsigned long tmp;

	/* parameter check */
	if ((NULL == pService) ||
		(NULL == filePath))
	{
		errno = EINVAL;
		return AMCDdpErrno_ErrParam;
	}

	/* open file */
	pService->file = fopen(filePath, "r");
	pService->readOffset = 0;

	if (pService->file)
	{
		callStat = stat(filePath, &fileStat);

		if (0 == callStat)
		{
			pService->fileSize = fileStat.st_size;
		}
		else
		{
			AMCDdpCloseFileService(pService);
			callStat = AMCDdpErrno_Failed;
		}
	}
	else
	{
		callStat = AMCDdpErrno_Failed;
	}


	/* check file size (not support 2G+ files) */
	if (0 == callStat)
	{
		if (pService->fileSize > (uint32_t)(0x7FFFFFFF))
		{
			callStat = AMCDdpErrno_FileTooLarge;
			AMCDdpCloseFileService(pService);
		}
	}


	/* read file MD5 */
	if (0 == callStat)
	{
		callStat = _getMd5DigestWithfile(filePath, pService->md5Digest);

		if (0 == callStat)
		{
			printf ("MD5 digest: ");
			for (tmp = 0; tmp < MV_MD5_MAC_LEN; tmp++)
			{
				printf ("%02x", (unsigned int)(pService->md5Digest[tmp] & 0xFF));
			}
			puts ("\n");
		}
	}
	

	/* return */
	return callStat;
}


AMCDdpErrno_t AMCDdpCloseFileService(AMCDdpFileService_st * pService)
{
	/* parameter check */
	if (NULL == pService)
	{
		errno = EINVAL;
		return AMCDdpErrno_ErrParam;
	}

	/* close file */
	if (NULL == pService->file)
	{
		errno = EBADR;
		return AMCDdpErrno_ErrParam;
	}
	else
	{
		int ret = fclose(pService->file);
		pService->file = NULL;
		pService->readOffset = 0;
		pService->fileSize = 0;
		return (0 == ret) ? 0 : AMCDdpErrno_Failed;
	}
}



const char *AMCDdpStrError(AMCDdpErrno_t err)
{
	if (((unsigned int)err) >= AMCDdpErrno_UndefErr)
	{
		return _strErrno[AMCDdpErrno_UndefErr];
	}
	else
	{
		return _strErrno[err];
	}
}


AMCDdpErrno_t AMCDdpReactWithData(
	AMCDdpContext_st 		*data, 
	int 					sockFd, 
	AMCDdpFileService_st 	*fileSvc,
	uint16_t				portFrom,
	struct in_addr 			ipFrom, 
	struct in_addr 			localIp)
{
	AMCDdpErrno_t err = 0;
	STATIC AMCDdpContext_st sendBuff;

	/********/
	/* check parameter */
	if ((NULL == data) ||
		(0 == sockFd))
	{
		err = AMCDdpErrno_ErrParam;
	}

	/********/
	/* check data format */
	if (0 == err)
	{
		switch (data->pkgOption)
		{
			case AMCDdpPkgOpt_SearchUtility:
				printf (">>>> Search Utility\n");
				err = _handleSearchUtility(data, sockFd, portFrom, ipFrom, &sendBuff);
				break;

			case AMCDdpPkgOpt_ReadUpgradeFile:
				printf (">>>> Read file\n");
				err = _handleFileRequest(data, sockFd, portFrom, ipFrom, &sendBuff, fileSvc);
				break;

			case AMCDdpPkgOpt_ReadFileMd5Digest:
				printf (">>>> Read MD5\n");
				err = _handleFileMd5DigestRequest(data, sockFd, portFrom, ipFrom, &sendBuff, fileSvc);
				break;

			case AMCDdpPkgOpt_ReadFileLength:
				printf (">>>> Read file length.\n");
				err = _handleFileSizeRequest(data, sockFd, portFrom, ipFrom, &sendBuff, fileSvc);
				break;
				
			default:
				errPrintf ("XXXX Unknown option: %d\n", data->pkgOption);
				err = AMCDdpErrno_PkgOptNotSupported;
				break;
		}
	}

	return err;
}



AMCDdpErrno_t _handleSearchUtility
	(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff)
{
	int callStat;
	const char reactMsg[] = "Hello, device!";
	STATIC char ipStr[16];
	AMCDdpErrno_t err = 0;

	if (AMCDDP_PKG_TYPE_REQ != data->pkgType)
	{
		err = AMCDdpErrno_PkgTypeNotSupported;
	}


	if (0 == err)
	{
		sendBuff->pkgType = AMCDDP_PKG_TYPE_ACK;
		sendBuff->pkgOption = AMCDdpPkgOpt_SearchUtility;
		sendBuff->contextLenth = htons((uint16_t)strlen(reactMsg));
		memcpy(sendBuff->context, reactMsg, strlen(reactMsg));

		inet_n4top(ipFrom, ipStr, sizeof(ipStr));
		callStat = simpleSocketSendto_udp
			(sockFd, ipStr, htons(portFrom), sendBuff, _sizeofDdpPkg(data));

		if (callStat < 0)
		{
			err = AMCDdpErrno_ErrorSocketSend;
		}
	}

	return err;
}


AMCDdpErrno_t _handleFileSizeRequest
	(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc)
{
	STATIC char ipStr[16];
	int callStat;
	AMCDdpErrno_t err = 0;
	uint32_t fileLen;

	/* check file service strcture */
	if ((NULL == pFileSvc) || 
		(NULL == pFileSvc->file))
	{
		err = AMCDdpErrno_FileServiceInvalid;
	}

	/* fill in data length */
	if (0 == err)
	{
		fileLen = pFileSvc->fileSize;
		fileLen = htonl(fileLen);

		sendBuff->pkgType = AMCDDP_PKG_TYPE_ACK;
		sendBuff->pkgOption = AMCDdpPkgOpt_ReadFileLength;
		sendBuff->contextLenth = htons(sizeof(fileLen));
		memcpy(sendBuff->context, &fileLen, sizeof(fileLen));
		printf (">>>> Send file length: %ld.\n", pFileSvc->fileSize);

		inet_n4top(ipFrom, ipStr, sizeof(ipStr));

		callStat = simpleSocketSendto_udp
			(sockFd, ipStr, htons(portFrom), sendBuff, _sizeofDdpPkg(sendBuff));

		if (callStat < 0)
		{
			err = AMCDdpErrno_ErrorSocketSend;
		}
	}

	return err;
}



AMCDdpErrno_t _handleFileRequest
	(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc)
{
	const char pkgSizeStrKey[] = "Context Size Limit:";
	const char fileOffStrKey[] = "File Offset:";
	static size_t pkgSizeStrLen = 0;
	static size_t fileOffStrLen = 0;
	STATIC char ipStr[16];
	int callStat;
	AMCDdpErrno_t err = 0;
	char *ctx;
	char *pkgSizeStr = NULL;
	size_t pkgSize;
	char *fileOffStr = NULL;
	off_t fileOffset;

	
	if ((0 == pkgSizeStrLen) ||
		(0 == fileOffStrLen))
	{
		pkgSizeStrLen = strlen(pkgSizeStrKey);
		fileOffStrLen = strlen(fileOffStrKey);
	}
	
	/* check file service strcture */
	if ((NULL == pFileSvc) || 
		(NULL == pFileSvc->file))
	{
		err = AMCDdpErrno_FileServiceInvalid;
	}
	
	
	/* read context size limit */
	if (0 == err)
	{
		ctx = (char*)(data->context);

		pkgSizeStr = strstr(ctx, pkgSizeStrKey);
		pkgSizeStr += (pkgSizeStr) ? pkgSizeStrLen : 0;
		pkgSize = (pkgSizeStr) ? strtol(pkgSizeStr, NULL, 10) : AMCDDP_PKG_MAX_CONTENT_LEN;
		pkgSize = (pkgSize > AMCDDP_PKG_MAX_CONTENT_LEN) ? AMCDDP_PKG_MAX_CONTENT_LEN : pkgSize;
	}


	/* seek file offset */
	if (0 == err)
	{
		fileOffStr = strstr(ctx, fileOffStrKey);

		if (NULL == fileOffStr)
		{
			err = AMCDdpErrno_ErrNetworkData;

			printf ("Get invalid data:\n");
			printData((void*)data, _sizeofDdpPkg(data));
		}
		else
		{
			fileOffStr += fileOffStrLen;
			fileOffset = strtol(fileOffStr, NULL, 10);
			printf ("Request string: %s\n", ctx);
			pFileSvc->readOffset = fileOffset;
		}
	}
	

	/* read file offet */
	if (0 == err)
	{
		if (pFileSvc->readOffset >= pFileSvc->fileSize)
		{
			pkgSize = 0;
		}
		else if ((pFileSvc->readOffset + pkgSize) > (pFileSvc->fileSize))
		{
			pkgSize = pFileSvc->fileSize - pFileSvc->readOffset;
		}
		else
		{
			/* pkgSize unchanged */
		}

		sendBuff->pkgType = AMCDDP_PKG_TYPE_ACK;
		sendBuff->pkgOption = AMCDdpPkgOpt_ReadUpgradeFile;
		sendBuff->contextLenth = htons(pkgSize);
		sendBuff->additionalInfo = htonl((uint32_t)((pFileSvc->readOffset) & 0xFFFFFFFF));

		callStat = fseek(pFileSvc->file, pFileSvc->readOffset, SEEK_SET);

		if (0 != callStat)
		{
			err = AMCDdpErrno_ErrorFileRead;
		}
	}


	/* read file content */
	if (0 == err)
	{
		if (pkgSize)
		{
			callStat = fread(sendBuff->context, sizeof(uint8_t), pkgSize, pFileSvc->file);

			if (0 == callStat)
			{
				err = AMCDdpErrno_ErrorFileRead;
			}
			else
			{
				pkgSize = callStat;
				sendBuff->contextLenth = htons(pkgSize);
			}
		}
		else
		{}
	}


	/* send file content or end flag */
	if (0 == err)
	{
		if (pkgSize)
		{
			inet_n4top(ipFrom, ipStr, sizeof(ipStr));
			callStat = simpleSocketSendto_udp
				(sockFd, ipStr, htons(portFrom), sendBuff, _sizeofDdpPkg(sendBuff));

			if (callStat < 0)
			{
				err = AMCDdpErrno_ErrorSocketSend;
			}
			else
			{
				printf ("Send data from with length %d, transmitted size: %ld\n", callStat, pFileSvc->readOffset);
			}
		}
		else
		{
			sendBuff->contextLenth = htons(0);

			inet_n4top(ipFrom, ipStr, sizeof(ipStr));
			callStat = simpleSocketSendto_udp
				(sockFd, ipStr, htons(portFrom), sendBuff, AMCDDP_HEADER_SIZE + pkgSize);

			if (callStat < 0)
			{
				err = AMCDdpErrno_ErrorSocketSend;
			}
		}
	}	

	return err;
}



AMCDdpErrno_t _handleFileMd5DigestRequest
	(const AMCDdpContext_st *data, int sockFd, uint16_t portFrom, in_addr_st ipFrom, AMCDdpContext_st *sendBuff, AMCDdpFileService_st *pFileSvc)
{
	AMCDdpErrno_t err = 0;
	STATIC char ipStr[16];
	int callStat;

	if (AMCDDP_PKG_TYPE_REQ != data->pkgType)
	{
		err = AMCDdpErrno_PkgTypeNotSupported;
	}

	if (0 == err)
	{
		sendBuff->pkgType = AMCDDP_PKG_TYPE_ACK;
		sendBuff->pkgOption = AMCDdpPkgOpt_ReadFileMd5Digest;
		sendBuff->contextLenth = htons(MV_MD5_MAC_LEN);
		memcpy(sendBuff->context, pFileSvc->md5Digest, MV_MD5_MAC_LEN);

		printf ("Now send MD5 digest back:\n");
		printData(sendBuff->context, MV_MD5_MAC_LEN);
		puts ("\n");
		inet_n4top(ipFrom, ipStr, sizeof(ipStr));
		callStat = simpleSocketSendto_udp
			(sockFd, ipStr, htons(portFrom), sendBuff, _sizeofDdpPkg(sendBuff));

		if (callStat < 0)
		{
			err = AMCDdpErrno_ErrorSocketSend;
		}
	}


	if (0 != err)
	{
		errPrintf ("errNum: %d\n", err);
	}
	return err;
}



size_t _sizeofDdpPkg(const AMCDdpContext_st *data)
{
	size_t dataLen = (size_t)ntohs(data->contextLenth);
	return (AMCDDP_HEADER_SIZE + dataLen);
}





int _getMd5DigestWithfile(const char *file, unsigned char *md5)
{
	void *fileContent;
	int fileFd = 0;
	stat_st fileStat;
	int callStat = 0;


	/* get file status */
	callStat = stat(file, &fileStat);

	/* open file */
	if (0 == callStat)
	{
		fileFd = open(file, O_RDONLY);

		if (0 == fileFd)
		{
			callStat = -1;
		}
	}


	/* map file */
	if (0 == callStat)
	{
		fileContent = mmap(NULL, fileStat.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);

		if (MAP_FAILED == fileContent)
		{
			callStat = -1;
			fileContent = NULL;
		}
	}

	/* calculate MD5 data */
	if (0 == callStat)
	{
		MD5(fileContent, fileStat.st_size, md5);
	}
	

	/* ENDS */
	if (fileContent)
	{
		munmap(fileContent, fileStat.st_size);
		fileContent = NULL;
	}

	if (fileFd)
	{
		close(fileFd);
		fileFd = 0;
	}
	
	return callStat;
}






