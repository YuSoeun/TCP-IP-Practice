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
       + segment 받을 때 malloc all_seg_flag[i] = 1로
    6. all_seg_flag[0]번부터 보면서 값이 1이면 파일에 적기
    7. 다 받았으면 client 종료
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
#include "progress.h"

#define MAX_CLNT 256

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
SocketInfo** recv_sokt_info;
pthread_mutex_t serv_mutx;
SendInfo * send_info;
RecvInfo ** recv_info;

void * readClntMsg(void * arg);
void * handleClnt(void * arg);
void removeDisconnectedClient(int sock);
void * printSendProgress();

int server(int listen_port, int recv_num, char* filename, int seg_size)
{
    int serv_sock, clnt_sock;
	int clnt_adr_sz;
	struct sockaddr_in serv_adr, clnt_adr;
	
	pthread_t console_thread;
	pthread_t *clnt_thread;
	void * thread_return;
	Segment ** segment;

	int total_seg = 0;

	recv_sokt_info = (SocketInfo **)malloc(sizeof(SocketInfo*) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		recv_sokt_info[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
	}
	
	// multi thead init and set socket
	pthread_mutex_init(&serv_mutx, NULL);
	if ((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket set error");
    }

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(listen_port);
	
	if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if (listen(serv_sock, 5)==-1)
		error_handling("listen() error");
	
	int optVal = 1;
	int optLen = sizeof(optVal);
	setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

	// (file size 총 수 / seg_size)해서 총 segment 갯수 구하기
	int file_size = filesize(filename);
	total_seg = file_size/seg_size + 1;
	segment = (Segment **)malloc(sizeof(Segment *) * total_seg);
	for (int i = 0; i < total_seg; i++) {
		segment[i] = (Segment *)malloc(sizeof(Segment));
	}

	// open file and save in segments
	total_seg = SaveFile2Seg(filename, segment, seg_size);
	printf("read all file\n");

	// set send/receive info for progress
	send_info = (SendInfo *)malloc(sizeof(SendInfo));
	setSendInfo(send_info, file_size, 0, 0.0);
	
	int seg_num = total_seg / recv_num;
	int remain = total_seg % recv_num;
	recv_info = (RecvInfo **)malloc(sizeof(RecvInfo *) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		int recv_total_seg = seg_num;
		if (remain > 0)
			recv_total_seg++;

		recv_info[i] = (RecvInfo *)malloc(sizeof(RecvInfo));
		setRecvInfo(recv_info[i], recv_total_seg, 0, 0, 0.0);
	}

	// accept receviers
	for (int i = 0; i < recv_num; i++) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&serv_mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&serv_mutx);
	
		recv_sokt_info[i]->id = i;
		memcpy(recv_sokt_info[i]->ip, inet_ntoa(clnt_adr.sin_addr), BUF_SIZE);
		read(clnt_sock, &recv_sokt_info[i]->listen_port, sizeof(int));
		printf("Connected client IP: %s, port: %d\n", recv_sokt_info[i]->ip, recv_sokt_info[i]->listen_port);
	}

	pthread_create(&console_thread, NULL, printSendProgress, (void *)&total_seg);
	
	int num = recv_num;
	clnt_thread = (pthread_t *)malloc(sizeof(pthread_t));
	for (int i = 0; i < recv_num; i++) {
		// receiver들끼리 연결 됐는지 확인하는 msg read하는 thread 열기
		pthread_create(&clnt_thread[i], NULL, readClntMsg, (void*)&clnt_socks[i]);

		// 각 recevier가 connect 요청해야 할 정보 (총 갯수, connect하는 수)
		write(clnt_socks[i], &recv_num, sizeof(int));
		num--;
		write(clnt_socks[i], &num, sizeof(int));
		for (int j = 0; j < recv_num ; j++) {
			if (i < j)
				writeSocketInfo(clnt_socks[i], recv_sokt_info[j]);
		}

		// 파일 이름, segment 총 수 보내주기
		int fname_size = (int)strlen(filename) + 1;
		write(clnt_socks[i], &fname_size, sizeof(int));
		write(clnt_socks[i], filename, fname_size);
		write(clnt_socks[i], &seg_size, sizeof(int));
		write(clnt_socks[i], &total_seg, sizeof(int));
	}

	// init complete 다 받았는지 확인
	for (int i = 0; i < recv_num; i++) {
		pthread_join(clnt_thread[i], &thread_return);

		// 각 receiver가 받을 segment 수 보내주기
		write(clnt_socks[i], &recv_info[i]->total_seg, sizeof(int));
	}

	// segment RR로 나눠서 보내주기
	clock_t start, end;
	double send_time;
	
	for (int i = 0; i < total_seg; i++) {
		start = clock();
		int clnt_index = i%recv_num;
		writeSegmentInfo(clnt_socks[clnt_index], segment[i]);
		end = clock();

		send_time = (double)(end - start) / CLOCKS_PER_SEC;
		updateSendInfo(send_info, segment[i]->size, send_time);
		updateRecvInfo(recv_info[clnt_index], segment[i]->size, send_time);
		sleep(0.5);
		// printf("write to %d seg[%d] %fs\n", clnt_index, send_info->snd_seg_num, send_info->time_spent);
	}
	pthread_join(console_thread, &thread_return);

	// for (int i = 0; i < recv_num; i++) {
	// 	removeDisconnectedClient(clnt_socks[i]);
	// }
	close(serv_sock);

    return 0;
}

/* read client msg thread */
void * readClntMsg(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len = 0, i;
	char msg[BUF_SIZE] = {};
	
	recvStr(clnt_sock, msg, BUF_SIZE);
	// printf("\nreceived msg[%d]: %s\n", clnt_sock, msg);

	return NULL;
}

/* remove disconnected client */
void removeDisconnectedClient(int sock)
{
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

void * printSendProgress()
{
    clock_t start, end;
	int bar_width;
    int file_size, total_seg, seg_num, cur_size, size_per_recv;
	double sec, size_per_sec = 0, snd_percent = 0;
	char tmp[BUF_SIZE];

    start = clock();
	clrscr();
	EnableCursor(0);

    while(1) {
		getSendInfo(send_info, &file_size, &cur_size, &sec);
		snd_percent = (double)cur_size / (double)file_size;
		if (sec > 0)
			size_per_sec = (double)cur_size / sec;

		// print sender progress bar
		sprintf(tmp, "%d", cur_size);
		bar_width = getWindowWidth() - (strlen("Sending Peer [] %% (/) Mbps (s)     \n")
				+ 5 + strlen(tmp) * 3 + 11);

		gotoxy(0, 0);
        printf("Sending Peer [");
		printBar(bar_width, snd_percent);
		printf("] %3.2lf%% (%d/%dBytes) %.2lfMbps (%.2lfs)     \n",
				100.0 * snd_percent, cur_size, file_size, size_per_sec, sec);

		// print receiver progress bar
		for (int i = 0; i < clnt_cnt; i++) {
			getRecvInfo(recv_info[i], &total_seg, &seg_num, &size_per_recv, &sec);
			snd_percent = (double)seg_num / (double)total_seg;
			sprintf(tmp, "%d", total_seg);
			bar_width = getWindowWidth() - (strlen("Receiving Peer  [] %% ( Bytes Sent / s)     \n")
					+ 5 + strlen(tmp) * 3 + 11);
			
			gotoxy(0, 2+i);
			printf("Receiving Peer %d [", i+1);
			printBar(bar_width, snd_percent);
			printf("] %3.2lf%% (%d Bytes Sent / %.2lfs)     \n", 100.0 * snd_percent, size_per_recv, sec);
		}
        
		gotoxy(0, clnt_cnt+2);

		if (cur_size >= file_size) {
			gotoxy(0, clnt_cnt+3);
			printf("seg_num: %d, total_seg: %d\n", seg_num, total_seg);
            break;
        }
    }

	EnableCursor(1);


    return NULL;
}