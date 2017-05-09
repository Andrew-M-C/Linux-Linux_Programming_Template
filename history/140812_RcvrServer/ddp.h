/**************************************************
 * File:        ddp.h
 * Create date: 2014-08-15
 * Author:      Andrew Chang
 */

#ifndef __DDP_H_
#define	__DDP_H_

#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "AMCmd5.h"

/**********/
/* AMCDdp definitions */
#define	CFG_ENABLE_MULTI_THREAD

typedef enum {
	AMCDdpErrno_Success = 0,
	AMCDdpErrno_Failed,
	AMCDdpErrno_ErrParam,
	AMCDdpErrno_ErrNetworkData,
	AMCDdpErrno_PkgTypeNotSupported,
	AMCDdpErrno_PkgOptNotSupported,
	AMCDdpErrno_FileTooLarge,
	AMCDdpErrno_ErrorSocketSend,
	AMCDdpErrno_ErrorFileStat,
	AMCDdpErrno_ErrorFileRead,
	AMCDdpErrno_FileServiceInvalid,

	AMCDdpErrno_UndefErr
} AMCDdpErrno_t;



#define	CFG_DEVICE_PORT		(1710)
#define	CFG_LOCAL_PORT		(4085)

#define	AMCDDP_PKG_MAX_CONTENT_LEN		(1024)

typedef	uint8_t AMCDdpPackageOption_t; enum{
	AMCDdpPkgOpt_NoOption		= 0,

	/* Old options used in TL-NS210 */
	AMCDdpPkgOpt_Reserved_EnterDdpMode		= 3,
	AMCDdpPkgOpt_Reserved_ReqAssignIp		= 4,
	AMCDdpPkgOpt_Reserved_ExitDdpMode		= 5,
	AMCDdpPkgOpt_Reserved_FileGood			= 6,
	AMCDdpPkgOpt_Reserved_FileIllegal		= 7,
	AMCDdpPkgOpt_Reserved_FileBurnOK		= 8,
	AMCDdpPkgOpt_Reserved_FileBurnFail		= 9,

	/* New options used in NC400 */
	AMCDdpPkgOpt_SearchUtility				= 10,
	AMCDdpPkgOpt_ReadUpgradeFile			= 11,
	AMCDdpPkgOpt_ReadFileMd5Digest			= 12,
	AMCDdpPkgOpt_ReadFileLength				= 13,
};


typedef uint8_t AMCDdpPackageType_t;enum{
	AMCDDP_PKG_TYPE_REQ = 0,
	AMCDDP_PKG_TYPE_ACK = 1,
	AMCDDP_PKG_TYPE_INFORM = 2,
	AMCDDP_PKG_HEARTBEAT = 3
};



typedef struct {
	AMCDdpPackageType_t		pkgType;
	AMCDdpPackageOption_t	pkgOption;
	uint16_t	contextLenth;
	uint32_t	additionalInfo;
	uint8_t		context[AMCDDP_PKG_MAX_CONTENT_LEN];	/* main content */
} AMCDdpContext_st;


typedef struct {
	FILE *file;
	off_t fileSize;
	off_t readOffset;
	uint8_t md5Digest[MV_MD5_MAC_LEN];
} AMCDdpFileService_st;

#define	AMCDDP_HEADER_SIZE	(sizeof(AMCDdpContext_st) - AMCDDP_PKG_MAX_CONTENT_LEN)


AMCDdpErrno_t AMCDdpOpenFileService(AMCDdpFileService_st *pService, const char *filePath);
AMCDdpErrno_t AMCDdpCloseFileService(AMCDdpFileService_st *pService);


const char *AMCDdpStrError(AMCDdpErrno_t err);
AMCDdpErrno_t AMCDdpReactWithData(
	AMCDdpContext_st 		*data, 
	int						sockFd, 
	AMCDdpFileService_st 	*fileSvc,
	uint16_t				portFrom,
	struct in_addr 			ipFrom, 
	struct in_addr 			localIp);


#endif

