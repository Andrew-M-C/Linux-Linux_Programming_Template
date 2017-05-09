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

#include "AMCMutableBuffer.h"

static int g_add_count = 0;

static AMCMutableBufferError_t _clean_buffer(struct AMCMutableBuffer *buffer)
{
	AMCMutableBufferError_t error = AMCMutableBuffer_DeleteAll(buffer);

	if (0 == error) {
		error = AMCMutableBuffer_Dump(buffer, 1);
	}

	return error;
}

static void _check_each_data_cb(void *data, size_t offset, size_t len, void *arg, BOOL isLastSegment)
{
	AMCPrintf("Dump data from offset 0x%08lX%s:", offset, isLastSegment ? " (last segment)" : "");
	AMCDataDump(data, len);
	return;
}


static AMCMutableBufferError_t _check_each_data(struct AMCMutableBuffer *buffer)
{
	char dataBuff[30];
	return AMCMutableBuffer_CheckAllData(buffer, _check_each_data_cb, dataBuff, sizeof(dataBuff), NULL);
}


static AMCMutableBufferError_t _add_data_with_len(struct AMCMutableBuffer *buffer, char *data, size_t len)
{
	AMCMutableBufferError_t error = 0;

	size_t tmp;

	g_add_count ++;
	for (tmp = 0; tmp < len; tmp++)
	{
		data[tmp]  = (char)(tmp & 0x0F);
		data[tmp] += (char)((g_add_count << 4) & 0xF0);
	}

	error = AMCMutableBuffer_Append(buffer, data, len);

	if (0 == error) {
		AMCPrintf("Memory usage: %d", AMCMutableBuffer_GetMemoryUsage(buffer));
		error = AMCMutableBuffer_Dump(buffer, 1);
	}

	return error;
}


static AMCMutableBufferError_t _insert_data(struct AMCMutableBuffer *buffer, char *data, size_t offset, size_t len)
{
	AMCMutableBufferError_t error = 0;

	size_t tmp;

	g_add_count ++;
	for (tmp = 0; tmp < len; tmp++)
	{
		data[tmp]  = (char)(tmp & 0x0F);
		data[tmp] += (char)((g_add_count << 4) & 0xF0);
	}

	error = AMCMutableBuffer_Insert(buffer, offset, data, len);

	if (0 == error) {
		error = AMCMutableBuffer_Dump(buffer, 1);
	}

	return error;
}


static AMCMutableBufferError_t _del_data(struct AMCMutableBuffer *buffer, size_t offset, size_t len)
{
	AMCMutableBufferError_t error = 0;
	size_t deletedLen = 0;

	error = AMCMutableBuffer_Delete(buffer, offset, len, &deletedLen);
	if (0 == error) {
		AMCPrintf("Deleted %d bytes, fragment size: %d", deletedLen, AMCMutableBuffer_FramentSize(buffer));
		AMCPrintf("Memory usage: %d", AMCMutableBuffer_GetMemoryUsage(buffer));
		error = AMCMutableBuffer_Dump(buffer, 1);
	}

	return error;
}


static AMCMutableBufferError_t _defragment(struct AMCMutableBuffer *buffer)
{
	AMCMutableBufferError_t error = AMCMutableBuffer_Defragment(buffer);
	if (0 == error) {
		AMCPrintf("Fragment size: %d", AMCMutableBuffer_FramentSize(buffer));
		AMCPrintf("Memory usage: %d", AMCMutableBuffer_GetMemoryUsage(buffer));
		error = AMCMutableBuffer_Dump(buffer, 1);
	}
	return error;
}


static AMCMutableBufferError_t _read_data(struct AMCMutableBuffer *buffer, BOOL shouldRemove)
{
	char buff[30];
	const size_t offset = 22;
	size_t readLen = 0;
	
	AMCMutableBufferError_t error = AMCMutableBuffer_Read(buffer, buff, offset, sizeof(buff), &readLen, shouldRemove);
	if (0 == error) {
		error = AMCMutableBuffer_Dump(buffer, 1);
		AMCPrintf("Read from %d, length %d", offset, sizeof(buff));
		AMCPrintf("Memory usage: %d", AMCMutableBuffer_GetMemoryUsage(buffer));
		AMCDataDump(buff, readLen);
	}
	
	return error;
}


/**********/
/* main */
#define _TEST_ERROR_AND_DO(error, op)	do{\
		if (0 == error) { \
			error = op; \
		} \
	}while(0)
static int trueMain(int argc, char* argv[])
{
	char data[4096];
	AMCMutableBufferError_t error = 0;
	struct AMCMutableBuffer *buffer = AMCMutableBuffer_New(33, 50, &error, TRUE);

	if (NULL == buffer) {
		goto ENDS;
	}

	/****/
	_TEST_ERROR_AND_DO(error, _add_data_with_len(buffer, data, 32));
	_TEST_ERROR_AND_DO(error, _add_data_with_len(buffer, data, 32));
	_TEST_ERROR_AND_DO(error, _add_data_with_len(buffer, data, 32));
	_TEST_ERROR_AND_DO(error, _add_data_with_len(buffer, data, 32));
	_TEST_ERROR_AND_DO(error, _add_data_with_len(buffer, data, 50));
	_TEST_ERROR_AND_DO(error, _del_data(buffer, 50, 51));
	_TEST_ERROR_AND_DO(error, _del_data(buffer, 20, 37));
	_TEST_ERROR_AND_DO(error, _del_data(buffer, 48, 5));
	//_TEST_ERROR_AND_DO(error, _del_data(buffer, 0, 11));
	//_TEST_ERROR_AND_DO(error, _defragment(buffer));	
	//_TEST_ERROR_AND_DO(error, _check_each_data(buffer));
	//_TEST_ERROR_AND_DO(error, _read_data(buffer, FALSE));
	_TEST_ERROR_AND_DO(error, _insert_data(buffer, data, 21, 44));
	//_TEST_ERROR_AND_DO(error, _insert_data(buffer, data, 90, 40));
	
	//_TEST_ERROR_AND_DO(error, _defragment(buffer));	
	//_TEST_ERROR_AND_DO(error, _clean_buffer(buffer));

	/****/
	/* ENDS */
ENDS:
	if (buffer) {
		AMCMutableBuffer_Destroy(buffer);
		buffer = NULL;
	}
	else {
		AMCPrintErr("Error: %s", AMCMutableBuffer_StrError(error));
	}
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

