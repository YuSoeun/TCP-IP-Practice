/*
    자동 완성 기능 기반 Search Engine 구현
    1. 서버의 Listen 포트 번호와 검색어 데이터베이스 파일을 command line argument로 넣어 프로그램을 실행
       - 서버는 Multi-Thread로 구현
     $ ./search_server 9090 data.txt
    2. 파일로부터 검색어와 검색어의 검색 횟수를 읽어 온다. Pohang 30000
    3. Client에서 검색어 입력
       - 연관 검색어 검색 횟수에 대해 정렬하여 실시간으로 표시 (Trie DS 사용, 최대 10개)
       - TCP nodelay option 줘보기
       - 검색어가 중간에 나오거나 끝에 나오는 경우 고려 (kmp algorithim)
    4. 검색어에 해당되는 부분은 임의의 색깔
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Console.h"

#define BUF_SIZE 1024

void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

int len = 0;
char search_word[BUF_SIZE] = {};
char msg[BUF_SIZE] = {};

int main(int argc, char *argv[])
{
    int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;

    char alpha;
    int len = 0;
    
	if(argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	 }
	
	sock=socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(atoi(argv[2]));
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1)
		error_handling("connect() error");

    // thread 열기
    pthread_create(&snd_thread, NULL, send_msg, (void*)&sock);
    pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
    
    // Client console 출력
    clrscr();
    while (1) {
        EnableCursor(1);
        DrawBorderLine('-', '|', 5);
        gotoxy(2, 4);
        printf("\033[38;2;255;255;255m* 연관 검색어 List");
        
        gotoxy(1, 1);
        printf("\033[38;2;159;75;153mSearch Word: ");

        gotoxy(14, 1);
        printf("\033[38;2;255;255;255m%s", search_word);
        EnableCursor(0);

        MySleep(100);
    }

    pthread_join(snd_thread, &thread_return);
    pthread_join(rcv_thread, &thread_return);
    close(sock);

    return 0;
}

void * send_msg(void * arg)   // send thread main
{
    char alpha;

	int sock = *((int*)arg);
	while(1) {
        gotoxy(12+len, 1);
		alpha = getch();
        printf("%c", alpha);
        
        if ((alpha == 127 || alpha == 8)) {       // backspace
            if (len > 0) {
                len--;
            }
        } else { 
            search_word[len++] = alpha;
        }
        search_word[len] = 0;

		if (!strcmp(search_word,"q\n")||!strcmp(search_word,"Q\n")) {
			close(sock);
			exit(0);
		}

		write(sock, search_word, strlen(search_word));
        clrscr();
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock = *((int*)arg);
    int count, buffer;
    char line[BUF_SIZE];
    char msg[BUF_SIZE];
    char temp[BUF_SIZE] = {0};
	int str_len;
    char *index;

	while (1) {
		str_len = read(sock, &count, sizeof(int));
		str_len = read(sock, msg, BUF_SIZE);

		if (str_len == -1) 
			return (void*) -1;
        
        for (int i = 0; i < count; i++) {
		    str_len = read(sock, line, BUF_SIZE);

            while (str_len < BUF_SIZE) {
                buffer = read(sock, temp, BUF_SIZE - str_len);
                str_len += buffer;
            }

            index = strstr(line, msg);
            gotoxy(3, 6+i);
            for (int i = 0; i < strlen(line); i++) {
                if (&line[i] >= index && &line[i] < index + strlen(msg)) {
                    printf("\033[38;2;130;150;200m%c", line[i]);
                } else {
                    printf("\033[38;2;255;255;255m%c", line[i]);
                }
            }
            printf("\n");
        }
	}
		msg[str_len] = 0;

	return NULL;
}
	
void error_handling(char *msg)
{
    gotoxy(3, 2);
	printf("%s\n", msg);
	exit(1);
}
