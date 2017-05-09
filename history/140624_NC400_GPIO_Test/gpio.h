/* Created on 2014-07-04 by Andrew Chang */

#ifndef	__GPIO_H__
#define		__GPIO_H__

#include <stdio.h>

void openGpioDev(int *fd);
void closeGpioDev(int *fd);
int setGpioOn(int gpioNum, int fd);
int setGpioOff(int gpioNum, int fd);
int setGpioDirOut(const int gpioNums[], int gpioCount, int fd);


#endif

