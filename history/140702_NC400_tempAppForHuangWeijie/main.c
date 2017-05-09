/**/

//#include "AMCDataTypes.h"

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_TIME
#define	CFG_LIB_ERRNO
#define	CFG_LIB_DEVICE
#include "AMCCommonLib.h"

#define	CONFIG_RALINK_MT7620
#define	CONFIG_RALINK_RT2880
#include "ralink_gpio.h"


#define	GPIO_DEV	"/dev/gpio"

static void _openGpioDev(int *fd)
{
	*fd = open(GPIO_DEV, O_RDONLY);
}


static void _closeGpioDev(int *fd)
{
	if (*fd)
	{
		close(*fd);
		*fd = 0;
	}
}


static int _setGpioOn(int gpioNum, int fd)
{
	int ret;
	ralink_gpio_led_info gpio;

	gpio.gpio 	= gpioNum;
	gpio.on 	= RALINK_GPIO_LED_INFINITY;
	gpio.off 	= 0;
	gpio.blinks = 0;
	gpio.rests	= 0;
	gpio.times	= RALINK_GPIO_LED_INFINITY;


	/******/
	/* check parameters */
	if (fd <= 0)
	{
		errno = EINVAL;
		return -1;
	}


	ret = ioctl(fd, RALINK_GPIO_LED_SET, &gpio);

	return ret;
}


static int _setGpioOff(int gpioNum, int fd)
{
	int ret;
	ralink_gpio_led_info gpio;

	gpio.gpio 	= gpioNum;
	gpio.on 	= 0;
	gpio.off 	= RALINK_GPIO_LED_INFINITY;
	gpio.blinks = 0;
	gpio.rests	= 0;
	gpio.times	= RALINK_GPIO_LED_INFINITY;

	/******/
	/* check parameters */
	if (fd <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	ret = ioctl(fd, RALINK_GPIO_LED_SET, &gpio);

	return ret;
}



static int _setGpioDirOut(const int gpioNums[], int gpioCount, int fd)
{
	int tmp;
	unsigned int regMsk_23to00 = 0;
	unsigned int regMsk_39to24 = 0;
	unsigned int regMsk_71to40 = 0;
	unsigned int regMsk_72 = 0;
	unsigned gpio;
	int ret = 0;

	/******/
	/* check parameters */
	if (fd <= 0)
	{
		errno = EINVAL;
		return -1;
	}

	/******/
	/* read configuration */
	for (tmp = 0; tmp < gpioCount; tmp++)
	{
		gpio = gpioNums[tmp];

		if (gpio <= 23)
		{
			regMsk_23to00 |= (1 << gpio);
		}
		else if (gpio <= 39)
		{
			regMsk_39to24 |= (1 << (gpio - 24));
		}
		else if (gpio <= 71)
		{
			regMsk_71to40 |= (1 << (gpio - 40));
		}
		else if (gpio <= 72)
		{
			regMsk_72 |= (1 << (gpio - 72));
		}
		else
		{
			/* invalid */
		}
	}	/* ENDS: "for (tmp = 0; tmp < gpioCount; tmp++)" */


	/******/
	/* set direction settings */

	/* Port 23-00 */
	if ((ret >= 0) &&
		(0 != regMsk_23to00))
	{
		ret = ioctl(fd, RALINK_GPIO_SET_DIR_OUT, regMsk_23to00);
	}

	/* Port 39-24 */
	if ((ret >= 0) &&
		(0 != regMsk_39to24))
	{
		ret = ioctl(fd, RALINK_GPIO3924_SET_DIR_OUT, regMsk_39to24);
	}

	/* Port 71-40 */
	if ((ret >= 0) &&
		(0 != regMsk_71to40))
	{
		ret = ioctl(fd, RALINK_GPIO7140_SET_DIR_OUT, regMsk_71to40);
	}

	/* Port 72 */
	if ((ret >= 0) &&
		(0 != regMsk_72))
	{
		ret = ioctl(fd, RALINK_GPIO72_SET_DIR_OUT, regMsk_72);
	}
	

	/******/
	/* return */
	return (ret < 0) ? ret : 0;
}

int trueMain(int argc, char* argv[])
{
//	DB_LOG("Now test vertical motor!\n");

	int gpio;
	int gpioNums[2] = {25, 24, 18, 29};

	_openGpioDev(&gpio);
	_setGpioDirOut(gpioNums, 2, gpio);
	
	_setGpioOff(gpioNums[0], gpio);
	_setGpioOff(gpioNums[1], gpio);
	_setGpioOff(gpioNums[2], gpio);
	_setGpioOff(gpioNums[3], gpio);

//	_setGpioOn(gpioNums[0], gpio);
//	_setGpioOn(gpioNums[1], gpio);
//	_setGpioOn(gpioNums[2], gpio);
//	_setGpioOn(gpioNums[3], gpio);
	
	_closeGpioDev(&gpio);

	
//	DB_LOG("Test ends.\n");
	
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

	sleep(1);
	mainRet = trueMain(argc, argv);
	//TEST_VALUE(mainRet);
	
	printf ("\n----ENDS-----\n\n");
	exit(mainRet);
	return 0;
}
