/*
    자동 완성 기능 기반 Search Engine 구현
    1. 서버의 Listen 포트 번호와 검색어 데이터베이스 파일을 command line argument 로 넣어 프로그램을 실행
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
#include <netinet/in.h>
#include <pthread.h>

#include "Console.h"
#include "file.h"
#include "trie.h"

void * handle_clnt(void * arg);
void send_msg(char * msg, int len, int clnt_sock);
void error_handling(char * msg);

#define BUF_SIZE 1024
#define MAX_CLNT 256
#define NAME_LEN 256

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
    // Multi-Thread로 서버 구현 (TCP nodelay socket option 주기)
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;

	char filename[NAME_LEN];
	FILE * fp;

	if(argc != 3) {
		printf("Usage : %s <port> <file name>\n", argv[0]);
		exit(1);
	}
  
	// pthread_mutex_init(&mutx, NULL);
	// serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	// memset(&serv_adr, 0, sizeof(serv_adr));
	// serv_adr.sin_family=AF_INET; 
	// serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	// serv_adr.sin_port=htons(atoi(argv[1]));
	
	// if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
	// 	error_handling("bind() error");
	// if(listen(serv_sock, 5)==-1)
	// 	error_handling("listen() error");

	// TODO: 파일 가져와 읽고 Trie structure로 저장

	Trie *trie = getNewTrie();
	insert(trie, "hello");
    printf("%d ", search(trie, "hello"));       // 1을 출력
 
    insert(trie, "helloworld");
    printf("%d ", search(trie, "helloworld"));  // 1을 출력
 
    printf("%d ", search(trie, "helll"));       // 0을 출력합니다(존재하지 않음)
 
    insert(trie, "hell");
    printf("%d ", search(trie, "hell"));        // 1을 출력
 
    insert(trie, "h");
    printf("%d \n", search(trie, "h"));         // 1 + 개행을 출력

	deletion(trie->root, "hello");
    printf("%d ", search(trie, "hello"));       // 0 출력(hello 삭제됨)
    printf("%d ", search(trie, "helloworld"));  // 1을 출력
    printf("%d \n", search(trie, "hell"));      // 1 + 개행을 출력
 
    deletion(trie->root, "h");
    printf("%d ", search(trie, "h"));           // 0 인쇄(h 삭제됨)
    printf("%d ", search(trie, "hell"));        // 1을 출력
    printf("%d\n", search(trie, "helloworld")); // 1 + 개행을 출력
	// while(1)
	// {
	// 	clnt_adr_sz = sizeof(clnt_adr);
	// 	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
	// 	pthread_mutex_lock(&mutx);
	// 	clnt_socks[clnt_cnt++] = clnt_sock;
	// 	pthread_mutex_unlock(&mutx);
	
	// 	pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
	// 	pthread_detach(t_id);
	// 	printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	// }
	// close(serv_sock);

    

    // 검색어 중간/끝에 나오는 경우 고려

    return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE] = {};
	
	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) {
		msg[str_len] = 0;
        send_msg(msg, str_len, clnt_sock);
        printf("msg: %s\n", msg);
    }
	
	// remove disconnected client
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) {
		if (clnt_sock == clnt_socks[i]) {
			while(i++ < clnt_cnt-1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_msg(char * msg, int len, int clnt_sock)   // send to all
{
	int send_len;
	send_len = write(clnt_sock, msg, len);
	printf("send_len: %d\n", send_len);
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}