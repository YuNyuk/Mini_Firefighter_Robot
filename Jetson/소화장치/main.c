#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>

#include "modules/ai.h"
#include "modules/itoa.h"
#include "modules/motorCtl.h"
#include "modules/gpioModule.h"
#include "modules/signalSender.h"



// 터틀봇 도착시 실행되는 함수 
int main(int argc, char* argv[]) {
	int pid_ai = 0;
	
	FILE *fp;
	char buf[6]; 

	bool fire_flag;
	int angle_x, angle_y;	

	// ai 스레드 초기화
//	onAI(&pid_ai);
//	workAI(&pid_ai);

	// ai thread의 pid 가져오기 	
	if ((fp = fopen("ai_process.txt", "r")) == NULL) {
		perror("fopen:ai ");
		exit(1);
	}
	fgets(buf, BUFSIZ, fp);
	pid_ai = atoi(buf);

	fclose(fp);

	// 현재 thread의 pid 넘겨주기
	if ((fp = fopen("main_process.txt", "w")) == NULL) {
                perror("fopen:main ");
                exit(1);
        }
	itoa(getpid(), buf, 10);
	fputs(buf, fp);

        fclose(fp);	


	// 카메라를 통해서 사진 촬영
	// 상하 각도를 정면을 바라보도록 변경 
	controlMotorAngle(0, 90);

	// 정면
	controlMotorAngle(2, 90);
	waitCtl(2);
//	takePic("front.jpg");
	sendSignal(0);

	// 좌측 
	controlMotorAngle(2, 20);
	waitCtl(2);
//	takePic("right.jpg");
	sendSignal(1);

	// 우측 
	controlMotorAngle(2, 160);
	waitCtl(2);
//	takePic("left.jpg");
	sendSignal(2);

	// 사진 수신 대기 
	waitCtl(0.3);

	// 사진 검사 
	workAI(&pid_ai);

	fire_flag = findPos(&angle_x, &angle_y);

	// 촬영한 방향에 불이 없으면
	if (!fire_flag) {
		// 터틀봇 회전


		// 다시 정면 좌우 촬영


		// 없으면 복귀
		
	}


	// 활영한 방향에 불이 있으면 
	if (fire_flag) {
		// 사진의 세로 픽셀 값을 동해서 각도 추정

		// 방향 회전	
		controlMotorAngle(0, angle_y);
		controlMotorAngle(2, angle_x);
		waitCtl(0.5);
//		printf("%d %d \n", angle_x, angle_y);

		// 스프레이 on
		toggleSpray(78);

		// 카메라에 붉은 픽셀이 일정 이상일 동안 
		while (fire_flag) {
			// 대기
			// cam에서 불 확인 

			// 테스트 용으로 5초 대기 후 종료
			waitCtl(10);
			break;
		}

		// 스프레이 off
		toggleSpray(78);
	}

	// 정면 보도록 조정 	
	controlMotorAngle(0, 90);
	controlMotorAngle(2, 90);
	waitCtl(0.5);

	// 모터 작동 종료 츌력
	puts("main ended");

	// ai 프로세스 끄기 
//	offAI(&pid_ai);

	return 0;
}
