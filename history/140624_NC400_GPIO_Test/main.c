/**/

//#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_TIME
#define	CFG_LIB_DEVICE
#define	CFG_LIB_ERRNO
#include "AMCCommonLib.h"

#include "gpio.h"

#include "AMCTimeMark.h"
#include "tplink_4phase_motor.h"

//#define	CFG_MOTOR_STEPS		TP_MOTOR_Param_PositiveMax
#define	CFG_MOTOR_STEPS		TP_MOTOR_Param_NegativeMax
//#define	CFG_MOTOR_STEPS		0

#define	CFG_MOTOR_SPEED_DIV	1

#define	CFG_MOTOR_INDEX		0


#define	_PHASE_USEC	(500)

int trueMain(int argc, char* argv[])
{
	TP_4PhaseMotorConfig_st motorConfig;
	TP_4PhaseMotorCommndParam_st motorPara;
	int file = open("/dev/motor", O_RDONLY);
	int gpioFile;
	int ports01[4] = {25, 24, 28, 29};
	int ports02[4] = {33, 32, 31, 30};
	int *ports;
	unsigned long index = CFG_MOTOR_INDEX;
	int ret;
	int errnoCopy;

	if (0 == index)
	{
		ports = ports01;
	}
	else
	{
		ports = ports02;
	}

	DB_LOG("Now test vertical motor!\n");

	/****/
	openGpioDev(&gpioFile);
	//setGpioDirOut(ports, 4, gpioFile);
	closeGpioDev(&gpioFile);

	usleep(100000);

	
	/****/
	motorConfig.MotorIndex = index;
	motorConfig.GpioPinA = ports[0];
	motorConfig.GpioPinB = ports[1];
	motorConfig.GpioPinC = ports[2];
	motorConfig.GpioPinD = ports[3];
	motorConfig.GpioPinNegativeMax = -1;
	motorConfig.GpioPinPositiveMax = -1;
	ret = ioctl(file, TP_MOTOR_WCMD_SetMotorPinNumbers, &motorConfig);
	errnoCopy = errno;
	DB_LOG("Get ioctl return: %d, errno: %d\n", ret, errnoCopy);

	motorPara.MotorIndex = index;
	motorPara.parameter = CFG_MOTOR_SPEED_DIV;
	ret = ioctl(file, TP_MOTOR_WCMD_SetMotorSpeedDivided, &motorPara);	
	DB_LOG("Get ioctl return: %d\n", ret);

	motorPara.MotorIndex = index;
	motorPara.parameter = CFG_MOTOR_STEPS;
	ret = ioctl(file, TP_MOTOR_WCMD_SetMotorRunWithStep, &motorPara);
	DB_LOG("Get ioctl return: %d\n", ret);

	close(file);
	DB_LOG("Test ends.\n");
	
	return 0;
}

int main(int argc, char* argv[])
{
	int mainRet;
	
	printf ("\n"CFG_SOFTWARE_DISCRIPT_STR"\n");
	printf ("Version "CFG_SOFTWARE_VER_STR"\n");
	printf ("Author: "CFG_AUTHOR_STR"\n");
	printf ("main() build time: "__TIME__", "__DATE__"\n");
	printf ("----START----\n\n");

	mainRet = trueMain(argc, argv);
	//TEST_VALUE(mainRet);
	
	printf ("\n----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}
