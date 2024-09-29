#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEVICE_FILENAME "/dev/buzzer_dev" // 모듈의 주소
#define SERVER_IP "10.10.141.41" // 서버의 IP 주소
#define SERVER_PORT 5000 // 서버의 포트 번호

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];
    int device;
    char buzzer_on;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid address/ Address not supported");
        return EXIT_FAILURE;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection to server failed");
        return EXIT_FAILURE;
    }

    // 부저 장치 파일 열기
    device = open(DEVICE_FILENAME, O_WRONLY);
    if (device < 0)
    {
        perror("Failed to open device");
        close(sock);
        return EXIT_FAILURE;
    }

    while (1)
    {
        // 서버로부터 메시지 읽기
        int n = read(sock, buffer, sizeof(buffer) - 1); // 읽은 바이트 수 반환
        if (n < 0)
        {
            perror("Read failed");
            break;
        }
        // 버퍼의 마지막에 널문자 추가. n은 버퍼의 바이트만큼 이기 때문에
        // n개의 바이트 만큼 읽었을 테니 buffer[n]은 마지막을 의미함.
        buffer[n] = '\0';

        // "good" 메시지 확인
        if (strcmp(buffer, "buzzer_on") == 0)
        {
            buzzer_on = 1;
            printf("Turning on buzzer...\n");
            write(device, &buzzer_on, sizeof(buzzer_on)); // 부저 켬
            printf("Device file descriptor: %d\n", device);
            printf("Turning on buzzer... Current state: %d\n", buzzer_on);
            sleep(2); // 부저를 2초 동안 울리기
        }
        if (strcmp(buffer, "buzzer_off") == 0)
        {
            buzzer_on = 0;
            write(device, &buzzer_on, sizeof(buzzer_on)); // 부저 끔
            printf("Turning off buzzer... Current state: %d\n", buzzer_on);
        }
    }

    // 닫기
    close(device);
    close(sock);
    return EXIT_SUCCESS;
}
