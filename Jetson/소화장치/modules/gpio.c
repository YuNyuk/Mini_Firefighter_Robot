#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "gpio.h"
#include "itoa.h"

// 중간중간에 일부러 문자열로 다 넣고 하드코딩 한 부분 있어요 
// 어차피 젯슨 나노로 장비는 고정되어 있으니까요!
/*
echo 0 > /sys/class/pwm/pwmchip0/export 
echo 20000000 > /sys/class/pwm/pwmchip0/pwm0/period 
echo 1000000 > /sys/class/pwm/pwmchip0/pwm0/duty_cycle 
echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable 
*/

// gpio 번호를 입력 해주면 export 설정
int exportGpio(int num){
	int export_fd;
	char num_str[4];

	if (num > 0) {
		export_fd = open("/sys/class/gpio/export", O_WRONLY | O_TRUNC);
	} 
	else {
		puts("GPIO number error");
		return -1;
	}
	
	// 파일 디스크립터 오류 처리 
	if (export_fd == -1) {
		perror("Open export_fd");
		return -1;
	}


	// 숫자를 문자열으로 변경
	itoa(num, num_str, 10);

	if (write(export_fd, num_str, strlen(num_str)) < 0) {
		perror("GPIO - export_fd - write error");
		return -1;
	}
	close(export_fd);
	
	return 0;
}

// 해당 gpio 번호를 입력으로 쓸지 출력으로 쓸지 설정
int setDirectionGpio(int num, const char *direction){
	int direct_fd;
	char directPath[50] = "/sys/class/gpio/gpio";
	char num_str[4];

	if (num > 0){
		// GPIO 번호를 문자열으로 변경
		itoa(num, num_str, 10);

		// 완전한 경로를 문자열으로 concat 
		strcat(directPath, num_str);
		strcat(directPath, "/direction");

		// 경로에 해당하는 파일 열기
		direct_fd = open(directPath, O_WRONLY | O_TRUNC);
	} 
	else {
		perror("GPIO number error");
		return -1;
	}
	
	//  파일 디스크립터 오류 처리	
	if (direct_fd == -1) {
		puts(directPath);
		perror("Open direct_fd error");
		return -1;
	}

	// 출력 해보고, 안되면 오류 처리 
	if (write(direct_fd , direction, strlen(direction)) != strlen(direction)) {
		perror("period_fd - write error");
		return -1;
	}
	close(direct_fd);
	
	return 0;
}

// gpio 번호를 입력 해주면 출력 값 설정
int setGpio(int num, int value){
	int value_fd;
	char num_str[4];
	char directPath[50] = "/sys/class/gpio/gpio";

	if (num > 0 && (value == 0 | value == 1)){
		// GPIO 번호를 문자열으로 변경
		itoa(num, num_str, 10);

		// 완전한 경로를 문자열으로 concat 
		strcat(directPath, num_str);
		strcat(directPath, "/value");

		// 경로에 해당하는 파일 열기
		value_fd = open(directPath, O_WRONLY | O_TRUNC);
	}
	else {
		perror("GPIO number error");
		return -1;
	}
	
	if (value_fd == -1) {
		puts(directPath);
		perror("Open value_fd ");
		return -1;
	}
	
	if (value == 0){
		if (write(value_fd, "0", 1) != 1){
			perror("GPIO - value_fd - write error");
			return -1;
		}
	}
	else{
		if (write(value_fd, "1", 1) != 1){
			perror("GPIO - value_fd - write error");
			return -1;
		}
	}
	close(value_fd);
	return 0;
}

// GPIO 번호를 입력 해주면 enable을 해제
int disableGpio(int num){
	int disable_fd;
	char num_str[3];
        char directPath[50] = "/sys/class/gpio/unexport";
	
	disable_fd = open(directPath, O_WRONLY | O_TRUNC);

        if (disable_fd == -1) {
                perror("Open value_fd ");
                return -1;
        }
	else{
	       itoa(num, num_str, 10);

                if (write(disable_fd, num_str, strlen(num_str)) < 0){
                        perror("GPIO - disable_fd - write error");
                        return -1;
                }
        }
        close(disable_fd);
        return 0;
}
