/**/
/* Created on 2014-06-24 by Andrew Chang */

#define	DEBUG
#define	CFG_LIB_STDOUT
#define	CFG_LIB_ERROUT
#define	CFG_LIB_DEVICE
#define	CFG_LIB_ERRNO
#include "AMCCommonLib.h"


#define	CONFIG_RALINK_MT7620
#define	CONFIG_RALINK_RT2880
#include "ralink_gpio.h"


#include "motor.h"


#define	GPIO_DEV	"/dev/gpio"


#define	_PIN_NUM_PER_MOTOR	(4)

#define	_MOTOR_VERT_1_PIN		(25)
//#define	_MOTOR_VERT_2_PIN		(24)
#define	_MOTOR_VERT_2_PIN		(33)		/* DEBUG only */
#define	_MOTOR_VERT_3_PIN		(28)
#define	_MOTOR_VERT_4_PIN		(29)

#define	_MOTOR_HORI_1_PIN		(33)
#define	_MOTOR_HORI_2_PIN		(32)
#define	_MOTOR_HORI_3_PIN		(31)
#define	_MOTOR_HORI_4_PIN		(30)


static int _isGoioInit = 0;


const static int _const_vert_pins[_PIN_NUM_PER_MOTOR] = 
{
	_MOTOR_VERT_1_PIN,
	_MOTOR_VERT_2_PIN,
	_MOTOR_VERT_3_PIN,
	_MOTOR_VERT_4_PIN
};


const static int _const_hori_pins[_PIN_NUM_PER_MOTOR] = 
{
	_MOTOR_HORI_1_PIN,
	_MOTOR_HORI_2_PIN,
	_MOTOR_HORI_3_PIN,
	_MOTOR_HORI_4_PIN
};




/**********/
//#pragma mark - Prvate functions


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
			regMsk_71to40 |= (1 << (gpio - 72));
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


static int _setGpioDirIn(const int gpioNums[], int gpioCount, int fd)
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
			regMsk_71to40 |= (1 << (gpio - 72));
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
		ret = ioctl(fd, RALINK_GPIO_SET_DIR_IN, regMsk_23to00);
	}

	/* Port 39-24 */
	if ((ret >= 0) &&
		(0 != regMsk_39to24))
	{
		ret = ioctl(fd, RALINK_GPIO3924_SET_DIR_IN, regMsk_39to24);
	}

	/* Port 71-40 */
	if ((ret >= 0) &&
		(0 != regMsk_71to40))
	{
		ret = ioctl(fd, RALINK_GPIO7140_SET_DIR_IN, regMsk_71to40);
	}

	/* Port 72 */
	if ((ret >= 0) &&
		(0 != regMsk_72))
	{
		ret = ioctl(fd, RALINK_GPIO72_SET_DIR_IN, regMsk_72);
	}
	

	/******/
	/* return */
	return (ret < 0) ? ret : 0;
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



/**********/
//#pragma mark - Packages


static void _motorGpiosCheckInitSetting(int fd)
{
	if (0 == _isGoioInit)
	{
		if (fd > 0)
		{
			_setGpioDirOut(_const_vert_pins, _PIN_NUM_PER_MOTOR, fd);
			_setGpioDirOut(_const_hori_pins, _PIN_NUM_PER_MOTOR, fd);

			_isGoioInit = 1;
		}
	}
	else
	{}
}


static int _motorPinAllClear(const int *fourPins)
{
	int fd = 0;

	_openGpioDev(&fd);

	if (fd)
	{
		_motorGpiosCheckInitSetting(fd);
	
		_setGpioOff(fourPins[0], fd);
		_setGpioOff(fourPins[1], fd);
		_setGpioOff(fourPins[2], fd);
		_setGpioOff(fourPins[3], fd);

		_closeGpioDev(&fd);
	
		return 0;
	}
	else
	{
		return -1;
	}
}


static int _motorPinSet(const int *fourPins, int pin1, int pin2, int pin3, int pin4)
{
	int fd = 0;

	_openGpioDev(&fd);

	if (fd)
	{
		_motorGpiosCheckInitSetting(fd);
	
		if (pin1){
			_setGpioOn(fourPins[0], fd);
		}
		else{
			_setGpioOff(fourPins[0], fd);
		}


		if (pin2){
			_setGpioOn(fourPins[1], fd);
		}
		else{
			_setGpioOff(fourPins[1], fd);
		}


		if (pin3){
			_setGpioOn(fourPins[2], fd);
		}
		else{
			_setGpioOff(fourPins[2], fd);
		}


		if (pin4){
			_setGpioOn(fourPins[3], fd);
		}
		else{
			_setGpioOff(fourPins[3], fd);
		}
			

		_closeGpioDev(&fd);
	
		return 0;
	}
	else
	{
		return -1;
	}
}




static int _motorSetStop(MotorSelect_t motor)
{
	int ret;

	switch (motor)
	{
		case MotorSelect_Vertical:
			ret = _motorPinAllClear(_const_vert_pins);
			break;

		case MotorSelect_Horizontal:
		default:
			ret = _motorPinAllClear(_const_hori_pins);
			break;
	}
	
	return ret;
}




static int _motorSetPhase_A_1000(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 1, 0, 0, 0);
	}
	else {
		return _motorPinSet(_const_hori_pins, 1, 0, 0, 0);
	}
}


static int _motorSetPhase_B_1200(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 1, 1, 0, 0);
	}
	else {
		return _motorPinSet(_const_hori_pins, 1, 1, 0, 0);
	}
}


static int _motorSetPhase_C_0200(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 0, 1, 0, 0);
	}
	else {
		return _motorPinSet(_const_hori_pins, 0, 1, 0, 0);
	}
}


static int _motorSetPhase_D_0230(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 0, 1, 1, 0);
	}
	else {
		return _motorPinSet(_const_hori_pins, 0, 1, 1, 0);
	}
}


static int _motorSetPhase_E_0030(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 0, 0, 1, 0);
	}
	else {
		return _motorPinSet(_const_hori_pins, 0, 0, 1, 0);
	}
}


static int _motorSetPhase_F_0034(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 0, 0, 1, 1);
	}
	else {
		return _motorPinSet(_const_hori_pins, 0, 0, 1, 1);
	}
}


static int _motorSetPhase_G_0004(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 0, 0, 0, 1);
	}
	else {
		return _motorPinSet(_const_hori_pins, 0, 0, 0, 1);
	}
}


static int _motorSetPhase_H_1004(MotorSelect_t motor)
{
	if (MotorSelect_Vertical == motor) {
		return _motorPinSet(_const_vert_pins, 1, 0, 0, 1);
	}
	else {
		return _motorPinSet(_const_hori_pins, 1, 0, 0, 1);
	}
}


/**********/
//#pragma mark - Public functions

int motorSetStop(MotorSelect_t motor)
{
	return _motorSetStop(motor);
}

int motorSetPhase_A_1000(MotorSelect_t motor)
{
	return _motorSetPhase_A_1000(motor);
}
int motorSetPhase_B_1200(MotorSelect_t motor)
{
	return _motorSetPhase_B_1200(motor);
}

int motorSetPhase_C_0200(MotorSelect_t motor)
{
	return _motorSetPhase_C_0200(motor);
}

int motorSetPhase_D_0230(MotorSelect_t motor)
{
	return _motorSetPhase_D_0230(motor);
}

int motorSetPhase_E_0030(MotorSelect_t motor)
{
	return _motorSetPhase_E_0030(motor);
}

int motorSetPhase_F_0034(MotorSelect_t motor)
{
	return _motorSetPhase_F_0034(motor);
}

int motorSetPhase_G_0004(MotorSelect_t motor)
{
	return _motorSetPhase_G_0004(motor);
}

int motorSetPhase_H_1004(MotorSelect_t motor)
{
	return _motorSetPhase_H_1004(motor);
}





