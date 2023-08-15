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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Console.h"
#include "client_server.h"
#include "socket.h"

// #define BUF_SIZE 1024
void* acceptReceiver(void* arg);
void* connectReceiver(void* arg);
void* sendMsg(void * arg);
void* recvSeg(void * arg);

int recv_cnt = 0;
int total_recv = __INT_MAX__;
int* recv_socks;
int len = 0;
char search_word[BUF_SIZE] = {};
char msg[BUF_SIZE] = {};
SocketInfo ** other_recv_info;
pthread_mutex_t clnt_mutx;

int client(int listen_port, char* ip, int port)
{
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_addr, clnt_adr, recv_addr;
<<<<<<< HEAD
	pthread_t* snd_thread;
	pthread_t acpt_thread, cnct_thread, rcv_thread;
=======
	pthread_t acpt_thread, snd_thread, rcv_thread;
	pthread_t* cnct_thread;
>>>>>>> 0499719b304c719e75f189cba5abd69b3279f48d
	void * thread_return;
    int recv_num;
    int recv_sock, recv_adr_sz;

    char alpha;
    int len = 0;
    
    // client listen socket init
    pthread_mutex_init(&clnt_mutx, NULL);

    if ((clnt_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket set error");
    }

    memset(&clnt_adr, 0, sizeof(clnt_adr));
	clnt_adr.sin_family = AF_INET; 
	clnt_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	clnt_adr.sin_port = htons(listen_port);
    
    if (bind(clnt_sock, (struct sockaddr*) &clnt_adr, sizeof(clnt_adr)) == -1)
		perror("bind() error");
	if (listen(clnt_sock, 5) == -1)
		perror("listen() error");
    printf("clnt_sock: %d\n", clnt_sock);
    
    int optVal = 1;
	int optLen = sizeof(optVal);
	setsockopt(clnt_sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

    // 다른 Receiver accept하는 thread 열기
    pthread_create(&acpt_thread, NULL, acceptReceiver, (void*)&clnt_sock);
    
    // server connect socket init
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	if (connect(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

    // 자신 listening port 주기
    write(serv_sock, &listen_port, sizeof(int));

    // receiver 갯수 받기
    read(serv_sock, &total_recv, sizeof(int));
    printf("total_recv: %d\n", total_recv);
    recv_socks = (int*)malloc(sizeof(int) * total_recv);

    read(serv_sock, &recv_num, sizeof(int));
    printf("recv_num: %d\n", recv_num);
    other_recv_info = (SocketInfo **)malloc(sizeof(SocketInfo*) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		other_recv_info[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
	}
    
    // Sender에서 receiver_sock 정보 받기
    for (int i = 0; i < recv_num; i++) {
        readSocketInfo(serv_sock, other_recv_info[i]);
        // printf("sender - ip:%s, port:%d, id:%d\n", other_recv_info[i]->ip, other_recv_info[i]->listen_port,other_recv_info[i]->id);

        // 다른 Receiver connect하는 thread
<<<<<<< HEAD
        pthread_create(&cnct_thread, NULL, connectReceiver, (void*)other_recv_info[i]);
=======
        pthread_create(&cnct_thread[i], NULL, connectReceiver, (void*)other_recv_info[i]);
>>>>>>> 0499719b304c719e75f189cba5abd69b3279f48d

        // 다른 Receiver segment read하는 thread
        pthread_create(&rcv_thread, NULL, recvSeg, (void*)&other_recv_info[i]);
    }
    // 파일 이름, segment 총 수 받기
    int fname_size, total_seg;
    char* filename;

    read(serv_sock, &fname_size, sizeof(int));
    filename = malloc(fname_size);
    recvStr(serv_sock, filename, fname_size);
    read(serv_sock, &total_seg, sizeof(int));

    pthread_join(acpt_thread, &thread_return);
    for (int i = 0; i < recv_num; i++) {
        pthread_join(cnct_thread[i], &thread_return);
    }

    // if (recv_cnt >= total_recv) {
    //     printf("Init complete\n");
    //     memcpy(msg, "Init complete", BUF_SIZE);
    //     write(serv_sock, msg, BUF_SIZE);
    // }

    // // 다른 Receiver 에게 받은 seg
    // snd_thread = (pthread_t *)malloc(sizeof(pthread_t));
    // for (int i = 0; i < recv_num; i++) {
    //     pthread_create(&snd_thread[i], NULL, sendMsg, (void*)&other_recv_info[i]);
    // }
    
    
    // Client console 출력
    // clrscr();
    // while (1) {

    // }

    pthread_join(acpt_thread, &thread_return);
    pthread_join(cnct_thread, &thread_return);
    close(clnt_sock);

    return 0;
}

void* acceptReceiver(void * arg)
{
    int* clnt_sock = (int *)arg;
    printf("void* acceptReceiver\n");
    int recv_sock, recv_adr_sz;
    struct sockaddr_in recv_adr;

    while (recv_cnt < total_recv-1) {
        recv_adr_sz = sizeof(recv_adr);
        printf("accept: %d (acceptReceiver - clnt_sock)\n", *clnt_sock);
        // recv_sock = accept(*clnt_sock, (struct sockaddr*)&recv_adr, &recv_adr_sz);
        if ((recv_sock = accept(*clnt_sock, (struct sockaddr*)&recv_adr, &recv_adr_sz)) == -1) {
            perror("accept error");
        }

        printf("recv_cnt: %d (acceptReceiver - before)", recv_cnt);
        pthread_mutex_lock(&clnt_mutx);
		recv_socks[recv_cnt++] = recv_sock;
		pthread_mutex_unlock(&clnt_mutx);
        recv_cnt++;
        printf("recv_cnt: %d (acceptReceiver) - after", recv_cnt);

    }
}

void* connectReceiver(void* arg)
{
    SocketInfo * recv_info = (SocketInfo *)arg;
    int recv_sock, recv_adr_sz;
    struct sockaddr_in recv_addr;

    if ((recv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket set error");
    }
    
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    // inet_aton(recv_info->ip, &recv_addr.sin_addr);
    recv_addr.sin_addr.s_addr = inet_addr(recv_info->ip);
    recv_addr.sin_port = htons(recv_info->listen_port);

    // printf("sender - ip:%s, port:%d\n", recv_info->ip, recv_info->listen_port);
    printf("sender - ip:%s, port:%d\n", inet_ntoa(recv_addr.sin_addr), recv_addr.sin_port);

    if (connect(recv_sock, (struct sockaddr*)&recv_addr, sizeof(recv_addr)) == -1)
        perror("connect() error");

    printf("recv_cnt: %d (connectReceiver - before)\n", recv_cnt);

    pthread_mutex_lock(&clnt_mutx);
    recv_socks[recv_cnt++] = recv_sock;
    pthread_mutex_unlock(&clnt_mutx);
    printf("recv_cnt: %d (connectReceiver - after)\n", recv_cnt);
}

void * sendMsg(void * arg)   // send thread main
{
    // char alpha;

	// int sock = *((int*)arg);
	// while(1) {
    //     gotoxy(12+len, 1);
	// 	alpha = getch();
    //     printf("%c", alpha);
        
    //     if ((alpha == 127 || alpha == 8)) {       // backspace
    //         if (len > 0) {
    //             len--;
    //         }
    //     } else { 
    //         search_word[len++] = alpha;
    //     }
    //     search_word[len] = 0;

	// 	if (!strcmp(search_word,"q\n")||!strcmp(search_word,"Q\n")) {
	// 		close(sock);
	// 		exit(0);
	// 	}

	// 	write(sock, search_word, strlen(search_word));
    //     clrscr();
	// }
	return NULL;
}
	
void * recvSeg(void * arg)   // read thread main
{
	// int sock = *((int*)arg);
    // int count, buffer;
    // char line[BUF_SIZE];
    // char msg[BUF_SIZE];
    // char temp[BUF_SIZE] = {0};
	// int str_len;
    // char *index;

	// while (1) {
	// 	str_len = read(sock, &count, sizeof(int));
	// 	str_len = read(sock, msg, BUF_SIZE);

	// 	if (str_len == -1) 
	// 		return (void*) -1;
        
    //     for (int i = 0; i < count; i++) {
	// 	    str_len = read(sock, line, BUF_SIZE);

    //         while (str_len < BUF_SIZE) {
    //             buffer = read(sock, temp, BUF_SIZE - str_len);
    //             str_len += buffer;
    //         }

    //         index = strstr(line, msg);
    //         gotoxy(3, 6+i);
    //         for (int i = 0; i < strlen(line); i++) {
    //             if (&line[i] >= index && &line[i] < index + strlen(msg)) {
    //                 printf("\033[38;2;130;150;200m%c", line[i]);
    //             } else {
    //                 printf("\033[38;2;255;255;255m%c", line[i]);
    //             }
    //         }
    //         printf("\n");
    //     }
	// }
	// 	msg[str_len] = 0;

	return NULL;
}