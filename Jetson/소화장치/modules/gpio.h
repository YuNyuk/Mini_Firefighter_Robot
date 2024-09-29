#ifndef __GPIO_H__
#define __GPIO_H__

int exportGpio(int num);
int setDirectionGpio(int num, const char *direction);
int setGpio(int num, int value);
int disableGpio(int num);

#endif		// __GPIO_H__
