#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>  // 선택 사항


#include "ai.h"
#include "itoa.h"

// 사진을 찍어서 filename으로 저장하는 함수
int takePic(const char *filename){
	int status;
	char pyPath[100];
	pid_t pid = fork();

	switch (pid) {
		case -1 : /* fork failed */
			perror("fork");
			return -1;

		case 0 : /* child process */
			getcwd(pyPath, 100);

			strcat(pyPath, "/img/takePic.py");

			status = execl("/usr/bin/python3", "python3", pyPath, filename, NULL);

			if(status < 0){
				perror("execl error");
				return -1;
			}

			return 0;
		default:
			while(wait(&status) != pid)
				continue;
			break;
	}
	return 0;
}

// 사진을 검사하는  ai의 스레드를 만드는 함수
int onAI(int *pid_ai){
        int status;
        char pyPath[100];
        pid_t pid = fork();
	sigset_t set;

	// SIGUSR1 handler 등록
	signal(SIGUSR1, handler);
	sigfillset(&set);
	sigdelset(&set, SIGUSR1);

        switch (pid) {
                case -1 : /* fork failed */
                        perror("fork");
                        return -1;

                case 0 : /* child process */
                        getcwd(pyPath, 100);

                        strcat(pyPath, "/ai/detectPic.py");

                        status = execl("/usr/bin/python3", "python3", pyPath, NULL);

                        if(status < 0){
                                perror("execl error");
                                return -1;
                        }

                        return 0;
		default:
			*pid_ai = pid;
			sigsuspend(&set);
			break;
	}

        return 0;
}

// 사진 검사 신호를 보내는 함수
int workAI(int *pid_ai){
	rmText();
	sigset_t set;

	// SIGUSR1 handler 등록
	signal(SIGUSR1, handler);
	sigfillset(&set);
	sigdelset(&set, SIGUSR1);

	if (kill(*pid_ai, SIGUSR1) == -1) {
		puts("kill -> SIGUSR1 error");
	}
	sigsuspend(&set);

        return 0;

}

// 사진을 검사하는 ai를 끄는 함수
int offAI(int *pid_ai){
	kill(*pid_ai, SIGKILL);
	rmText();

	return 0;
}

// SIGUSR1 handler
void handler(int signo) {
	// do nothing
	;
}

// 출력되어 있는 좌표값으로 화재의 유무와 위치를 판단하는 함수
bool findPos(int *angle_x, int *angle_y){

	// 사진 크기 - 640x480
		// 각 9.8픽셀당 1도로 상정
	FILE *rfp;
	char buf[16];
	const char *strPath[3] = {"img/front.txt", "img/left.txt", "img/right.txt"};

        for (int i=0;i<3;i++){
		if ((rfp = fopen(strPath[i], "r")) == NULL) {
			continue;
		}

		if (fgets(buf, BUFSIZ, rfp) != NULL) {
			*angle_x = (int)(atof(strtok(buf, " ")) / 9.8);
			*angle_y = (int)(atof(strtok(buf, " ")) / 9.8);

			if(i==0) *angle_x += 55;
			else if(i==2) *angle_x += 110;

			*angle_x = 180-*angle_x;
			*angle_y = 180-*angle_y;

			itoa(*angle_x, buf, 10);
			puts(buf);
			itoa(*angle_y, buf, 10);
			puts(buf);

			fclose(rfp);

			return true;
		}
	}

	rmText();

	return false;
}

// ai의 결과를 삭제하기 위해서 돌리는 함수
void rmText(){
        const char *strPath[3] = {"img/front.txt", "img/left.txt", "img/right.txt"};

        for (int i=0;i<3;i++)
                remove(strPath[i]);
}
