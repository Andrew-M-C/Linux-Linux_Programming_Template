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
#define	CFG_LIB_GETOPT
#include "AMCCommonLib.h"

#include "AMCBase64.h"
#include "AMCTimeMark.h"


typedef enum {
	_Type_Unknown,
	_Type_Dec,
	_Type_Enc,
}_OptType_t;



//#define _CFG_ENC_INPUT_STR	"Left and Right"

//#define	_CFG_ENC_INPUT_STR	"Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure."
/*
#define	_CFG_DEC_INPUT_STR	"TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlz\n"	\
								"IHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2Yg\n"	\
								"dGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGlu\n"	\
								"dWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRo\n"	\
								"ZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4="
*/
#define	_CFG_DEC_INPUT_STR	"JCgvdXNyL3NiaW4vdGVsbmV0ZCk"

static void _printHelp(int argc, char* argv[])
{
	printf ("%s: Base64 encoding/decoding.\n\tHelp message not implemented yet, sorry!\n", argv[0]);
	return;
}

#ifdef	_CFG_ENC_INPUT_STR
static int _testEncodeMain(int argc, char* argv[])
{
	char inputStr[] = _CFG_ENC_INPUT_STR;
	char outputStr[sizeof(inputStr) * 2];
	int callStat;
	
	callStat = AMCBase64Encode(inputStr, outputStr, sizeof(inputStr), TRUE);

	if (callStat < 0)
	{
		errPrintf("Failed to encode Base64.\n");
	}
	else
	{
		printf ("Encoding result:\n%s\n", outputStr);
	}

	return callStat;
}
#endif


#ifdef	_CFG_DEC_INPUT_STR
static int _testDecodeMain(int argc, char* argv[])
{
	char inputStr[] = _CFG_DEC_INPUT_STR;
	unsigned char outputStr[sizeof(inputStr)];
	int callStat;
	unsigned long outputLen;

	callStat = AMCBase64Decode(inputStr, outputStr, &outputLen);

	if (callStat < 0)
	{
		errPrintf("Failed to decode Base64.\n");
	}
	else
	{
		printf ("Decoding result:\n");
		printData(outputStr, outputLen);
	}

	return callStat;
}
#endif


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	int opt;
	char *inputFile = NULL;
	char *outputFile = NULL;
	_OptType_t opType = _Type_Unknown;
	BOOL isOptOK = TRUE;

	while ((TRUE == isOptOK) &&
			(-1 != (opt = getopt(argc, argv, "deo:i:-h"))))
	{
		switch (opt)
		{
			/****/
			case 'd':
				if (_Type_Enc == opType)
				{
					isOptOK = FALSE;
					errPrintf("You could only specify either ENCRYPT or DECRYPT mode.\n");
				}
				else
				{
					opType = _Type_Dec;
				}
				break;

			/****/
			case 'e':
				if (_Type_Dec == opType)
				{
					isOptOK = FALSE;
					errPrintf("You could only specify either ENCRYPT or DECRYPT mode.\n");
				}
				else
				{
					opType = _Type_Enc;
				}
				break;

			/****/
			case 'o':
				outputFile = optarg;
				break;

			/****/
			case 'i':
				inputFile = optarg;
				break;

			/****/
			case '-':
				errPrintf("%s: invalid option %s\n", argv[0], argv[optind]);
				isOptOK = FALSE;
				break;

			/****/
			case 'h':
				_printHelp(argc, argv);
				isOptOK = FALSE;
				break;

			/****/
			case '?':
				isOptOK = FALSE;
				break;

			/****/
			default:
				break;
		}
	}



	/********/
	if ((NULL == inputFile) &&
		(NULL == outputFile) &&
		_Type_Unknown == opType)
	{
		_printHelp(argc, argv);
	}
	else if (isOptOK)
	{
		if (inputFile)
		{
			printf ("Input file: %s\n", inputFile);
		}

		if (outputFile)
		{
			printf ("Output file: %s\n", outputFile);
		}
	}
	else
	{}
	
	
	/********/
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

