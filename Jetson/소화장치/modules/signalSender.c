#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#define BUF_SIZE 4096
#define SERVER_IP "10.10.141.48"
#define PORT 5010
#define USR 21

void error_handling(const char *);

int sendSignal(int picNum) {
	int sock;
	int len = 0;
	struct sockaddr_in serv_addr;
	FILE *image;
	char buf[BUF_SIZE];
	const char *filename;

	// Connect to server
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		error_handling("socket() error");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
	serv_addr.sin_port = htons(PORT);

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	// Send command to server
	const char *command = "IMG";
	if(write(sock, command, strlen(command)) == -1)
        	error_handling("write() error");

	if (picNum == 0)
	{
		filename="/home/jetson/final_code/img/front.jpg", "wb";
//		puts("1231230000000000");
	}
	else if (picNum == 1)
	{
		filename="/home/jetson/final_code/img/right.jpg", "wb";
//		puts("123123111111");
	}
	else if (picNum == 2)
	{
		filename="/home/jetson/final_code/img/left.jpg", "wb";
//		puts("123123222222");
	}
	else
		return -1;


	if (filename == NULL)
		error_handling("Invalid picNum");

	image = fopen(filename, "wb");
	puts("img open");
	if (image == NULL)
		error_handling("fopen() error");

	// Receive and write data to file
	puts("write start");
	int sum = 0;
	while ((len = recv(sock, buf, BUF_SIZE, 0)) > 0) {
//		printf("Bytes received: %d\n", len); // 디버그 출력
		if(fwrite(buf, sizeof(char), len, image) != len)
		{
			error_handling("fwrite() error");
		}
		sum += len;
//		printf("sum: %d\n", sum); // 디버그 출력
	}
	puts("write end");
	if (len == 0) {
		puts("Connection closed by server");  // 서버에서 정상 종료
	} else if (len == -1) {
		error_handling("recv() error");  // 수신 오류 발생
	}

	//if (len == -1)
	//	error_handling("recv() error");
	puts("close start");
	fclose(image);
	close(sock);
	puts("close end");

	return 0;
}

void error_handling(const char *msg) {
	perror(msg);
	exit(1);
}
