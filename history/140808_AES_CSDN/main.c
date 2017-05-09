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

void _printData(uint8_t *data, size_t size)
{
	int tmp;

	for (tmp = 0; tmp < size; tmp++)
	{
		printf ("%02x ", data[tmp]);
		if ((tmp + 1 < size) &&
			(0 == ((tmp + 1) & 0x0F)))
		{
			printf ("\n");
		}
	}
	printf ("\n");
}


void testCalStat(AES_RETURN stat)
{
	printf ("stat code: %d\n", stat);
}


#define	TST_STAT()		testCalStat(callStat);
int trueMain(int argc, char* argv[])
{
	uint8_t plain[64];
	uint8_t cipher[128];
	uint8_t key[5] = {'a', 'd', 'm', 'i', 'n'};
//	uint8_t key[5] = {'1', '2', '3', '4', '5'};
	uint8_t buffer01[256];
	uint8_t buffer02[256];
	uint8_t iv[16];
	int tmp;
	AES_RETURN callStat;

	aes_encrypt_ctx ctx;

	for (tmp = 0; tmp < sizeof(plain); tmp++)
	{
		plain[tmp] = tmp;
	}

	for (tmp = 0; tmp < sizeof(iv); tmp++)
	{
		iv[tmp] = (tmp & 0xF0);
	}

	

	/**********/
	/* Now start */
	callStat = aes_init();				TST_STAT();
	callStat = aes_mode_reset(&ctx);	TST_STAT();

	/**********/
	/* Encrypt */
	callStat = aes_cbc_encrypt(plain, cipher, sizeof(plain), iv, &ctx);	TST_STAT();
	printf ("Encrypted:\n");
	_printData(cipher, sizeof(cipher));

	/**********/
	

	/**********/
	/* return */
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
