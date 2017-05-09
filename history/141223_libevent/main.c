/**/

#include "AMCDataTypes.h"

#define CFG_LIB_FORK
#define CFG_LIB_STDOUT
#define CFG_LIB_SLEEP
#include "AMCCommonLib.h"


/**********/
/* server.c */
int serverMain(void);


/**********/
/* main */
int trueMain(int argc, char* argv[])
{
	return serverMain();
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

