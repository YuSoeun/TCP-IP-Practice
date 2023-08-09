/*
    file sharing을 위한 P2P File Sharing 구현
    - Sending Peer가 가진 File을 나머지 N개의 Receiving Peer들과 공유
    - file은 여러 Segment(동일한 크기)로 나눠 단 하나의 Receving Peer(RR로 선정)에만 전송
    - multi thread로 구현

    sender: ./p2p -s -n (최대 receving peer) -f (filename) -g (segment 크기 KB)
    receiver: ./p2p -r -a 192.168.10.2 9090

    1. Sending peer가 접속한 peer들의 ID, ip, port 정보 분배
    2. Receiver끼리 각각 연결
    3. Peer 간 연결 완료 시 Sender에게 msg 전송
    4. Sender가 모든 Receiver에게 총 seg 수, 파일 이름 보내기
    5. Receiver가 seg를 받으면 다른 peer에게 전송
       + 동시에 다른 peer에게 seg 받기
    6. seg 순서대로 조합
    7. 다 받았으면 알리거나 client 종료
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Console.h"
#include "client_server.h"

#define BUF_SIZE 1024

void * send_msg(void * arg);
void * recv_msg(void * arg);
// 

int len = 0;
char search_word[BUF_SIZE] = {};
char msg[BUF_SIZE] = {};

int client(char* ip, int port)
{
    int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;

    char alpha;
    int len = 0;
	
	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
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