#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pwm.h"
#include "itoa.h"

// 중간중간에 일부러 문자열로 다 넣고 하드코딩 한 부분 있어요 
// 어차피 젯슨 나노로 장비는 고정되어 있으니까요!
/*
echo 0 > /sys/class/pwm/pwmchip0/export 
echo 20000000 > /sys/class/pwm/pwmchip0/pwm0/period 
echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle 
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable 
*/

// pwm 번호를 입력 해주면 export 설정
int exportPwm(int num){
	int export_fd;
	if (num == 0 | num == 2) {
		export_fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY | O_TRUNC);
	} 
	else {
		puts("ex - PWM number error");
		return -1;
	}
	
	if (export_fd < 0) {
		perror("Open export_fd");
		return -1;
	}
	if (num == 0){
		if (write(export_fd, "0", 1) < 0){
			perror("export_fd - write error");
			return -1;
		}
	}
	if (num == 2){
	if (write(export_fd, "2", 1)  < 0){
			perror("export_fd - write error");
			return -1;
		}
	}
	close(export_fd);
	
	return 0;
}

// pwm 번호와 설정하고 싶은 주기와 듀티(1일 시간)을 nsec 단위로 입력하면 설정
int setPwm(int num, int period, int duty){
	int period_fd, duty_fd;
	int tmp;
	char period_str[10], duty_str[10];

	if (num == 0){
		period_fd = open("/sys/class/pwm/pwmchip0/pwm0/period", O_WRONLY | O_TRUNC);
		duty_fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY | O_TRUNC);
	} 
	else if(num == 2){
		period_fd = open("/sys/class/pwm/pwmchip0/pwm2/period", O_WRONLY | O_TRUNC);
		duty_fd = open("/sys/class/pwm/pwmchip0/pwm2/duty_cycle", O_WRONLY | O_TRUNC);
	} 
	else {
		perror("set - PWM number error");
		return -1;
	}
	
	if ((period_fd == -1) | (duty_fd == -1)) {
		perror("Open period_fd | duty_fd error");
		return -1;
	}

	itoa(period, period_str, 10);
	itoa(duty, duty_str, 10);

	tmp = strlen(period_str);
	if (write(period_fd, period_str, tmp) != tmp) {
		perror("period_fd - write error");
		return -1;
	}

	tmp = strlen(duty_str);
	if (write(duty_fd, duty_str, tmp ) != tmp) {
		perror("duty_fd - write error");
		return -1;
	}
	close(period_fd);
	close(duty_fd);
	
	return 0;
}

// pwm 번호를 입력 해주면 enable 설정
int enablePwm(int num){
	int enable_fd;
	if (num == 0){
		enable_fd = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY | O_TRUNC);
	} 
	else if(num == 2){
		enable_fd = open("/sys/class/pwm/pwmchip0/pwm2/enable", O_WRONLY | O_TRUNC);
	} 
	else {
		perror("PWM number error");
		return -1;
	}
	
	if (enable_fd < 0) {
		perror("Open enable_fd ");
		return -1;
	}
	
	if (write(enable_fd, "1", 1) != 1){
		perror("enable_fd - write error");
		return -1;
	}
	close(enable_fd);
	return 0;
}

// pwm 번호를 입력 해주면 enable을 해제
int disablePwm(int num){
	int enable_fd;
	if (num == 0){
		enable_fd = open("/sys/class/pwm/pwmchip0/pwm0/enable", O_WRONLY | O_TRUNC);
	} 
	else if(num == 2){
		enable_fd = open("/sys/class/pwm/pwmchip0/pwm2/enable", O_WRONLY | O_TRUNC);
	} 
	else {
		perror("PWM number error");
		return -1;
	}
	
	if (enable_fd < 0) {
		perror("Open disable_fd ");
		return -1;
	}

	if (write(enable_fd, "0", 1) < 0){
		perror("pwm - disable_fd - write error");
		return -1;
	}
	close(enable_fd);
	return 0;
}

// pwm 번호를 입력 해주면 unexport
int unexportPwm(int num){
	int unexport_fd;
	if (num == 0 | num == 2){
		unexport_fd = open("/sys/class/pwm/pwmchip0/unexport", O_WRONLY | O_TRUNC);
	} 
	else {
		perror("PWM number error");
		return -1;
	}
	
	if (unexport_fd < 0) {
		perror("Open unexport");
		return -1;
	}
	
	if (num == 0){
		if (write(unexport_fd, "0", 1) < 0){
			perror("unexport_fd - write error");
			return -1;
		}
	}
	else if (num == 2){
		if (write(unexport_fd, "2", 1) < 0){
			perror("unexport_fd - write error");
			return -1;
		}
	}
	close(unexport_fd);
	return 0;
}
