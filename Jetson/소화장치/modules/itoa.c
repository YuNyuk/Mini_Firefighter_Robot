#include <stdbool.h>
#include <time.h>
#include "itoa.h"


int waitCtl(float time){
	clock_t start_time = clock();
	clock_t end_time;
	double time_elapsed = 0;
       
	while (time_elapsed < time){
		end_time = clock();
		time_elapsed = (float)(end_time - start_time) / CLOCKS_PER_SEC;
	}

	return 0;
}

void reverse(char str[], int length) {
        int start = 0;
        int end = length - 1;
        while (start < end) {
                char temp = str[start];
                str[start] = str[end];
                str[end] = temp;
                start++;
                end--;
        }
}

char* itoa(int num, char* str, int base) {
        int i = 0;
        bool isNegative = false;

        // 0인 경우 바로 문자열 "0"으로 반환
        if (num == 0) {
                str[i++] = '0';
                str[i] = '\0';
                return str;
        }

        // 음수 처리 (10진수일 경우에만)
        if (num < 0 && base == 10) {
                isNegative = true;
                num = -num;
        }

        // 숫자를 문자열로 변환
        while (num != 0) {
                int rem = num % base;
                str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
                num = num / base;
        }

        // 음수일 경우 '-' 추가
        if (isNegative) {
                str[i++] = '-';
        }

        // 문자열 끝에 NULL 추가
        str[i] = '\0';

        // 문자열을 뒤집어서 올바른 순서로 정렬
        reverse(str, i);

        return str;
}

