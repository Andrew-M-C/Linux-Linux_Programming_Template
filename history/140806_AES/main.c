/**/

#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_STRING
#define	CFG_LIB_RAND
#define	CFG_LIB_TIME
#include "AMCCommonLib.h"

#include "aes.h"


#define	_TEST_RET(ret)		if (0 != ret){printf("Error, code: %d\n", ret);}

void _testData(uint8_t *data, size_t len)
{
	int tmp;

	
	for (tmp = 0; tmp < len; tmp++)
	{
		printf ("%02x ", data[tmp] & 0xFF);

		if (0 == ((tmp + 1) & 0xF))
		{
			printf ("\n");
		}
	}
	printf ("\n");
}


int trueMain(int argc, char* argv[])
{
	uint8_t iv[16] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
	};

	char key[32];
	int tmp;

	uint8_t plain[5] = {'a', 'd', 'm', 'i', 'n'};
	uint8_t buffer[1024];
	uint8_t buffer2[1024];
	size_t bufferLen, bufferLen2;
	int callRet;

	srand(time(0));
	for (tmp = 0; tmp < sizeof(key); tmp++)
	{
		key[tmp] = tmp;
	}

	printf ("Plain:\n");
	_testData(plain, sizeof(plain));

	bufferLen = aesCbcRequiredEncryptBufferSize(sizeof(plain));
	TEST_VALUE(bufferLen);

	callRet = aesCbcEncrypt(plain, sizeof(plain), key, 128, iv, buffer, bufferLen);

	_TEST_RET(callRet);
	printf ("Encrypted:\n");
	_testData(buffer, bufferLen);

	bufferLen2 = aesCbcRequiredDecryptBufferSize(bufferLen);
	TEST_VALUE(bufferLen2);

	callRet = aesCbcDecrypt(buffer, bufferLen, key, 128, iv, buffer2, bufferLen2);

	_TEST_RET(callRet);
	printf ("Decrypted:\n");
	_testData(buffer2, bufferLen2);
		
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
