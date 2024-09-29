#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include "itoa.h"
#include "gpio.h"

int toggleSpray(int gpioNum){
	if(gpioNum == 0)
		return 0;

//	exportGpio(gpioNum);
	setDirectionGpio(gpioNum, "high");

	setGpio(gpioNum, 0);
	waitCtl(0.5);
	setGpio(gpioNum, 1);

//	disableGpio(gpioNum);
	return 0;
}

