/* Created on 2014-07-04 by Andrew Chang */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define	CONFIG_RALINK_MT7620
#define	CONFIG_RALINK_RT2880
#include "ralink_gpio.h"

#define	GPIO_DEV	"/dev/gpio"

void openGpioDev(int *fd)
{
	*fd = open(GPIO_DEV, O_RDONLY);
}


void closeGpioDev(int *fd)
{
	if (*fd)
	{
		close(*fd);
		*fd = 0;
	}
}


int setGpioOn(int gpioNum, int fd)
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


int setGpioOff(int gpioNum, int fd)
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



int setGpioDirOut(const int gpioNums[], int gpioCount, int fd)
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


