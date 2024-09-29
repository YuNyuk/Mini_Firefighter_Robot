#ifndef __PWM_H__
#define __PWM_H__

int exportPwm(int);
int setPwm(int, int, int);
int enablePwm(int);
int disablePwm(int);
int unexportPwm(int);

#endif		// __PWM_H__
