#ifndef __MOTORCTL_H__
#define __MOTORCTL_H__

#define PERIOD 20000000
#define DUTY 1000000

int controlMotorAngle(int, int);
int enPwm();
int disPwm();

#endif	// __MOTORCTL_H__
