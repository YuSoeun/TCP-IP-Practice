/*
    file sharing을 위한 P2P File Sharing 구현
    - Sending Peer가 가진 File을 나머지 N개의 Receiving Peer들과 공유
    - file은 여러 Segment(동일한 크기)로 나눠 단 하나의 Receving Peer(RR로 선정)에만 전송
    - multi thread로 구현

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
#include <ctype.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "Console.h"
#include "file.h"
#include "client_server.h"
#include "socket.h"

#define MAX_CLNT 256
// #define BUF_SIZE 1024

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
SocketInfo** clnt_info;
pthread_mutex_t serv_mutx;

void sendRecvSocksInfo();
void * readClntMsg(void * arg);
void * handle_clnt(void * arg);
void send_msg(int count, char msg[BUF_SIZE], int clnt_sock);
// void bubbleSort(Result** arr, int count);

int server(int listen_port, int recv_num, char* filename, int seg_size)
{
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	pthread_t *clnt_thread;
	char ** result;
	void * thread_return;

	clnt_info = (SocketInfo **)malloc(sizeof(SocketInfo*) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		clnt_info[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
	}
	
	// multi thead init and set socket
	pthread_mutex_init(&serv_mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(listen_port);
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	int optVal = 1;
	int optLen = sizeof(optVal);
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	// TODO: open file and save in segments (seg 수, 파일 이름 따로라도 저장해서 보내주기)

	for (int i = 0; i < recv_num; i++) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&serv_mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&serv_mutx);
	
		clnt_info[i]->id = clnt_sock;
		memcpy(clnt_info[i]->ip, inet_ntoa(clnt_adr.sin_addr), sizeof(clnt_adr.sin_addr));
		clnt_info[i]->port = (int)clnt_adr.sin_port;
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}

	// receiver 갯수 보내기
	for (int i = 0; i < recv_num; i++) {
		write(clnt_socks[i], &recv_num, sizeof(int));
	}

	clnt_thread = (pthread_t *)malloc(sizeof(pthread_t));
	// receiver들의 정보 자신 것 빼고 모든 receiver에게
	for (int i = 0; i < recv_num; i++) {
		for (int j = 0; j < recv_num ; j++) {
			if (i != j)
				writeSocketInfo(clnt_socks[i], clnt_info[j]);
		}

		// receiver들끼리 connect complete msg read하는 thread 생성
		pthread_create(&clnt_thread[i], NULL, readClntMsg, (void*)&clnt_socks[i]);
	}
	for (int i = 0; i < recv_num; i++) {
		pthread_join(clnt_thread[i], &thread_return);
		printf("thread[%d] return %p", i, thread_return);
	}

	close(serv_sock);

    return 0;
}

/* handle client to receive and send msg */
void * readClntMsg(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE] = {};
	
	for (int i = 0; i < clnt_cnt; i++) {
		recvStr(clnt_sock, msg, BUF_SIZE);
		printf("\nreceived msg: %s\n", msg);

		return NULL;
	}
}

void removeDisconnectedClient(int sock)
{
	// remove disconnected client
	pthread_mutex_lock(&serv_mutx);
	for (int i = 0; i < clnt_cnt; i++) {
		if (sock == clnt_socks[i]) {
			while (i++ < clnt_cnt - 1)
				clnt_socks[i] = clnt_socks[i + 1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&serv_mutx);
	close(sock);
}

/* send msg to clnt_socket */
void send_msg(int count, char msg[BUF_SIZE], int clnt_sock)
{
	int send_len;
	char line[BUF_SIZE];

	// send line count and origin msg 
	send_len = write(clnt_sock, &count, sizeof(int));
	send_len = write(clnt_sock, msg, BUF_SIZE);

	for (int i = 0; i < count; i++) {
		printf("%s\n", line);
		send_len = write(clnt_sock, line, BUF_SIZE);
	}
}


/* bubble sort for Result */
// void bubbleSort(Result** arr, int count)
// {
//     Result* temp;
//     for (int i = 0; i < count; i++) {
//         for (int j = 0; j < count - i - 1; j++) {
//             if (arr[j]->cnt < arr[j+1]->cnt) {    // swap
//                 temp = arr[j];
//                 arr[j] = arr[j+1];
//                 arr[j+1] = temp;
//             }
//         }
//     }
// }

