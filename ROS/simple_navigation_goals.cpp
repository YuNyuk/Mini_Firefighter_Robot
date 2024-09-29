#include <ros/ros.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <actionlib/client/simple_action_client.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>  // 신호 처리를 위한 헤더 추가

#define BUF_SIZE 100
#define NAME_SIZE 20

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

char name[NAME_SIZE] = "[Default]";
char msg[BUF_SIZE];
int sock;
double startX, startY, startZ, startW;  // 원래 위치
bool running = true;  // 스레드 실행중인지 확인
pthread_t rcv_thread;

void *recv_msg(void *arg);
void handle_sigint(int sig);  // SIGINT 처리 함수
void *run_extinguishing_system(void *arg);  // 소화 시스템 실행 함수
void *run_detecting_system(void *arg);  // 감지 시스템 실행 함수


int main(int argc, char** argv) {
	if (argc != 4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		return 1;
	}

	sprintf(name, "%s", argv[3]);

	// ROS 노드 초기화 및 액션 클라이언트 생성
	ros::init(argc, argv, "tcp_navigation_goals");
	ros::NodeHandle nh;
	MoveBaseClient ac("move_base", true);

	// 초기 위치 좌표
	startX = 1.75;
	startY = -6.18;
	startZ = 0.93;
	startW = 0.35;

	// 서버와 연결
	struct sockaddr_in serv_addr;
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock == -1) {
		perror("socket() error");
		return 1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_addr.sin_port = htons(atoi(argv[2]));

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("connect() error");
		return 1;
	}

	sprintf(msg, "[%s:PASSWD]", name);
	write(sock, msg, strlen(msg));

	// SIGINT 신호 처리기 설정 (Ctrl + C 처리)
	signal(SIGINT, handle_sigint);

	// 액션 서버가 준비될 때까지 대기
	while (!ac.waitForServer(ros::Duration(5.0))) {
		ROS_INFO("Waiting for the move_base action server to come up");
	}

	ROS_INFO("Detecting System Online");
	pthread_t detect_thread;
	pthread_create(&detect_thread, NULL, run_detecting_system, NULL);
	pthread_detach(detect_thread);  // 메인 스레드와 독립적으로 실행되도록 함

	// 수신 스레드 생성
	pthread_t rcv_thread;
	pthread_create(&rcv_thread, NULL, recv_msg, NULL);
	pthread_detach(rcv_thread);

	ros::spin();  // ROS 노드 실행 및 메시지 처리

	running = false;  // 종료 플래그 설정
	pthread_cancel(rcv_thread);  // 수신 스레드 종료 요청
	pthread_join(rcv_thread, NULL);  // 스레드 종료 대기
	close(sock);  // 소켓 종료

	return 0;
}


void *recv_msg(void *arg) {
	int str_len;
	while (1) {
		memset(msg, 0x0, sizeof(msg));
		str_len = read(sock, msg, BUF_SIZE - 1);
		if (str_len <= 0) {
			perror("read() error");
			close(sock);
			return NULL;
		}

		msg[str_len] = '\0';
		ROS_INFO("Received message: %s", msg);

		// 메시지 파싱 및 목표 위치 설정
		char *goal_msg = strstr(msg, "GOAL@");  // "GOAL@" 이후의 문자열 추출
		if (goal_msg) {
			double x, y; // 변수 
			double z, w; // 하드코딩
			if (sscanf(goal_msg, "GOAL@%lf@%lf", &x, &y) == 2) {
				// 구역에 따라 z, w 값 설정
				if (x < 0.59 && y > -3.89) {
					// 1구역
					x = x + 0.3;
					y = y - 0.22;
					z = 0.94;
					w = 0.33;
					ROS_INFO("Fire on Area 1");
				} else if (x > 0.59 && y > -3.89) {
					// 2구역
					x = x - 0.3;
					y = y - 0.22;
					z = 0.34;
					w = 0.94;
					ROS_INFO("Fire on Area 2");
				} else if (x < 0.59 && y < -3.89) {
					if (x < -0.9) { 
						// 특별 구역
						x = x + 0.35;
						y = y;
						z = -1;
						w = 0;
						ROS_INFO("Fire on Area S");
					} else {
						// 3구역
						x = x + 0.3;
						y = y - 0.22;
						z = 0.94;
						w = 0.33;
						ROS_INFO("Fire on Area 3");
					}
				} else if (x > 0.59 && y < -3.89) {
					// 4구역
					x = x - 0.3;
					y = y + 0.22;
					z = 0.94;
					w = 0.33;
					ROS_INFO("Fire on Area 4");
				}
		
			// 보정된 값 출력
			ROS_INFO("extinguish site : x = %.2lf, y = %.2lf, z = %.2lf, w = %.2lf", x, y, z, w);


			move_base_msgs::MoveBaseGoal goal;
			goal.target_pose.header.frame_id = "map";
			goal.target_pose.header.stamp = ros::Time::now();
			goal.target_pose.pose.position.x = x;
			goal.target_pose.pose.position.y = y;
			goal.target_pose.pose.orientation.z = z;
			goal.target_pose.pose.orientation.w = w;

			ROS_INFO("Sending goal to target location");
			MoveBaseClient ac("move_base", true);
			ac.sendGoalAndWait(goal, ros::Duration(120.0, 0), ros::Duration(120.0, 0));

			// 목표 위치 도착 후
			if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) {
				ROS_INFO("Goal arrived!");

				// 소화 프로그램 실행
				pthread_t extinguish_thread;
				pthread_create(&extinguish_thread, NULL, run_extinguishing_system, NULL);
				pthread_join(extinguish_thread, NULL);  // 소화 프로그램이 끝날 때까지 대기

				// 'extinguish finished' 메시지를 서버로 전송
				sprintf(msg, "Situation Over!\n");
				write(sock, msg, strlen(msg));

				// 목표 지점 도착 후 원래 위치로 돌아가기
				goal.target_pose.pose.position.x = startX;
				goal.target_pose.pose.position.y = startY;
				goal.target_pose.pose.orientation.z = startZ;
				goal.target_pose.pose.orientation.w = startW;

				ROS_INFO("Returning to the start location");
				ac.sendGoalAndWait(goal, ros::Duration(120.0, 0), ros::Duration(120.0, 0));

				if (ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED)
					ROS_INFO("Successfully returned to the start location!");
				else
					ROS_INFO("Failed to return to the start location");
			} else {
				ROS_INFO("Failed to move to the goal");
			}
		} else {
			ROS_WARN("Received invalid goal format");
		}
	} else {
		ROS_WARN("Message does not contain 'GOAL@'");
	}
}
}

// SSH 명령어를 실행하고, "main ended" 메시지가 출력될 때까지 대기하는 함수
void *run_extinguishing_system(void *arg) {
	ROS_INFO("Running fire extinguishing program...");

	// SSH 명령 실행 후 출력 대기
	FILE* ssh_pipe = popen("ssh jetson@192.168.100.108 'cd ~/final_code && ./main.out && exit'", "r");
	if (!ssh_pipe) {
		ROS_ERROR("Failed to run SSH command.");
		return NULL;
	}

	char buffer[128];
	while (fgets(buffer, sizeof(buffer), ssh_pipe) != NULL) {
		// SSH 명령의 출력을 읽음
		std::string output(buffer);

		// "main ended" 메시지가 출력되었는지 확인
		if (output.find("main ended") != std::string::npos) {
			ROS_INFO("Fire extinguishing program completed. Returning to the start location.");
			break;
		}
	}

	// SSH 세션 종료
	pclose(ssh_pipe);

	return NULL;
}

// 감지 시스템을 실행하는 함수
void *run_detecting_system(void *arg) {
	// SSH 명령어 실행 (소화 감지 시스템 시작)
	system("ssh jetson@192.168.100.108 'cd ~/final_code && ./init.sh && nohup python3 ai/detectPic.py & sleep 2 && exit'");
	return NULL;
}

// SIGINT 처리기: Ctrl+C가 눌렸을 때 서버 연결을 안전하게 종료하고 프로그램 종료
void handle_sigint(int sig) {
	ROS_INFO("Shutting down. Closing socket.");
	running = false;  // 스레드 종료 요청
	system("ssh jetson@192.168.100.108 'cd ~/final_code && ./exit.sh && exit'");    
	close(sock);  // 서버 연결 종료
	ros::shutdown();  // ROS 노드 종료
	exit(0);  // 프로그램 종료
}

