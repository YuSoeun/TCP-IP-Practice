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
       + segment 받을 때 all_seg_flag[i] = 1로
    6. all_seg_flag[0]번부터 보면서 값이 1이면 파일에 적기
    7. 다 받았으면 client 종료
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
#include "file.h"
#include "progress.h"

void * acceptReceiver(void * arg);
void * connectReceiver(void * arg);
void * sendMsg(void * arg);
void * sendSeg2Peers(void* arg);
void * getSegmentFromSock(int serv_sock, int seg2recv_num);
void * recvSegFromPeer(void * arg);
void * writeSeg2File(void * arg);
void * printRecvProgress();

int recv_cnt = 0;
int recv_num;                   // connect를 요청해야할 thread 수
int total_recv = __INT_MAX__;   // 총 recever 수
int remain_seg_num;             // segment를 더 받아야 할 개수
int * recv_socks;

SocketInfo ** other_recv_info;
pthread_mutex_t clnt_mutx, remain_mutx;
SendInfo * snd_info;
RecvInfo ** rcv_info;

Segment ** segment;
int seg_size, total_seg;
int seg2recv_num;                 // Sender로부터 받아야 할 segment 수
int * all_seg_flag;              // 전체 segment 써졌는지 표시하는 flag
int * recv_seg_num;              // sender에게 받은 segment seq 표시
int * progress;
int sender_seg_cnt;
int stored_size;
// void * printSendProgress();

int client(int listen_port, char * ip, int port)
{
    int serv_sock, clnt_sock;
    int recv_sock, recv_adr_sz;
	struct sockaddr_in serv_addr, clnt_adr, recv_addr;
	pthread_t acpt_thread, rcv_thread, write_file_thread, console_thread;
	pthread_t * cnct_thread, * snd_thread;
	void * thread_return;
    
    char msg[BUF_SIZE] = {};
    
    // client listen socket init
    pthread_mutex_init(&clnt_mutx, NULL);
    pthread_mutex_init(&remain_mutx, NULL);

    if ((clnt_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket set error");
    }

    memset(&clnt_adr, 0, sizeof(clnt_adr));
	clnt_adr.sin_family = AF_INET; 
	clnt_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	clnt_adr.sin_port = htons(listen_port);
    
    if (bind(clnt_sock, (struct sockaddr *) &clnt_adr, sizeof(clnt_adr)) == -1) {
		perror("bind() error");
        return -1;
    }
	if (listen(clnt_sock, 5) == -1)
		perror("listen() error");
    
    int optVal = 1;
	int optLen = sizeof(optVal);
	setsockopt(clnt_sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));

    // 다른 Receiver accept하는 thread 열기
    recv_socks = (int *)malloc(sizeof(int) * total_recv);
    pthread_create(&acpt_thread, NULL, acceptReceiver, (void *)&clnt_sock);
    
    // server connect socket init
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	if (connect(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

    // 자신 listening port 주기
    write(serv_sock, &listen_port, sizeof(int));

    // receiver 개수 받기
    read(serv_sock, &total_recv, sizeof(int));
    pthread_mutex_lock(&clnt_mutx);
    recv_socks = realloc(recv_socks, sizeof(int) * total_recv);
    pthread_mutex_unlock(&clnt_mutx);

    read(serv_sock, &recv_num, sizeof(int));
    cnct_thread = (pthread_t *)malloc(sizeof(recv_num) * recv_num);
    other_recv_info = (SocketInfo **)malloc(sizeof(SocketInfo *) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		other_recv_info[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
	}

    // progress 현황 출력하는 thread 생성
    pthread_t prgs_thread;
    progress = (int *)malloc(sizeof(recv_num) * recv_num);
    memset(progress, 0, sizeof(recv_num));
    
    // Sender에서 receiver_sock 정보 받고 다른 Receiver connect
    for (int i = 0; i < recv_num; i++) {
        readSocketInfo(serv_sock, other_recv_info[i]);
        pthread_create(&cnct_thread[i], NULL, connectReceiver, (void *)other_recv_info[i]);
    }
    
    // 파일 이름, segment 총 수 받기
    int fname_size, file_size;
    char * filename;

    read(serv_sock, &fname_size, sizeof(int));
    filename = malloc(fname_size);
    recvStr(serv_sock, filename, fname_size);
    read(serv_sock, &file_size, sizeof(file_size));
    read(serv_sock, &seg_size, sizeof(int));
    read(serv_sock, &total_seg, sizeof(int));

    // connect + accept 일정 개수 했는지 확인
    while (recv_cnt < total_recv-1) {;}
    
    for (int i = 0; i < recv_num; i++) {
        pthread_join(cnct_thread[i], &thread_return);
    }
    pthread_detach(acpt_thread);

    // 미리 segment, all_seg_flag malloc
    all_seg_flag = (int *)malloc(total_seg * sizeof(int));
    memset(all_seg_flag, 0, total_seg * sizeof(int));
    segment = (Segment **)malloc(total_seg * sizeof(Segment *));
    for (int i = 0; i < total_seg; i++) {
        segment[i] = (Segment *)malloc(sizeof(Segment));
        segment[i]->content = (char *)malloc(seg_size * sizeof(char));
    }

    // Sender에게 init complete all_seg_flag 보내기
    memcpy(msg, "Init complete", BUF_SIZE);
    write(serv_sock, msg, BUF_SIZE);
    
    // sender로부터 receive해야할 개수 읽기
    read(serv_sock, &seg2recv_num, sizeof(int));

    // sendInfo, recvInfo init
    snd_info = (SendInfo *)malloc(sizeof(SendInfo));
	setSendInfo(snd_info, file_size, 0, 0.0);
	
	rcv_info = (RecvInfo **)malloc(sizeof(RecvInfo *) * recv_cnt);
	for (int i = 0; i < recv_cnt; i++) {
		rcv_info[i] = (RecvInfo *)malloc(sizeof(RecvInfo));
		setRecvInfo(rcv_info[i], seg2recv_num, 0, 0, 0.0);
	}
    stored_size = 0;
    
    // 다른 Receiver segment read하고 write하는 thread 열어놓기
    remain_seg_num = total_seg - seg2recv_num;
    snd_thread = (pthread_t *)malloc(recv_cnt * sizeof(pthread_t));
    recv_seg_num = malloc(seg2recv_num * sizeof(int));
    memset(recv_seg_num, -1, seg2recv_num * sizeof(int));
    for (int i = 0; i < recv_cnt; i++) {
        pthread_create(&rcv_thread, NULL, recvSegFromPeer, (void *)&i);
        pthread_create(&snd_thread[i], NULL, sendSeg2Peers, (void *)&recv_socks[i]);
    }

    // console print, consumer thread 만들기
	pthread_create(&console_thread, NULL, printRecvProgress, (void *)&total_seg);
    pthread_create(&write_file_thread, NULL, writeSeg2File, (void *)filename);
    
    // Sender로부터 segment 받아오기
    getSegmentFromSock(serv_sock, seg2recv_num);

    // 다른 recv에게 seg 다 보냈는지 출력
    for (int i = 0; i < recv_cnt; i++) {
        pthread_join(snd_thread[i], &thread_return);
    }

    pthread_join(write_file_thread, &thread_return);
    pthread_join(console_thread, &thread_return);
    pthread_detach(rcv_thread);

    // free segment
    for (int i = 0; i < total_seg; i++) {
        free(segment[i]->content);
        free(segment[i]);
    }
    free(segment);
    
    // TODO: sendInfo, recvInfo 사용 시 필요없는 코드 정리
    // TODO: Receiver 진행상황 console 출력
    
    close(clnt_sock);

    return 0;
}

/* accept receiver thread */
void * acceptReceiver(void * arg)
{
    int * clnt_sock = (int *)arg;
    int recv_sock, recv_adr_sz;
    struct sockaddr_in recv_adr;
    recv_adr_sz = sizeof(recv_adr);

    while (recv_cnt < total_recv-1) {
        if ((recv_sock = accept(*clnt_sock, (struct sockaddr *)&recv_adr, &recv_adr_sz)) == -1) {
            // perror("accept error");
        } else {
            pthread_mutex_lock(&clnt_mutx);
            recv_socks[recv_cnt++] = recv_sock;
            pthread_mutex_unlock(&clnt_mutx);
        }
    }
}

/* connect receiver thread */
void * connectReceiver(void * arg)
{
    SocketInfo * rcv_sck_info = (SocketInfo  *)arg;
    int recv_sock, recv_adr_sz;
    struct sockaddr_in recv_addr;

    if ((recv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket set error");
    }
    
    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = inet_addr(rcv_sck_info->ip);
    recv_addr.sin_port = htons(rcv_sck_info->listen_port);

    if (connect(recv_sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) == -1) {
        perror("connect() error");
    } else {
        pthread_mutex_lock(&clnt_mutx);
        recv_socks[recv_cnt++] = recv_sock;
        pthread_mutex_unlock(&clnt_mutx);
    }
}

/* get segment from sock and save to segment */
void * getSegmentFromSock(int serv_sock, int seg2recv_num)
{
    int seq = 0;
    char * content;
    double send_time;
    Segment * tmp_seg;
    pthread_t * send_seg_thread;
    clock_t start, end;

    sender_seg_cnt = 0;
    tmp_seg = (Segment *)malloc(sizeof(Segment));
    tmp_seg->content = (char *)malloc(seg_size);
    content = (char *)(malloc(seg_size));

    // seg server로부터 받기
    start = clock();
    while (sender_seg_cnt < seg2recv_num) {
        readSegmentInfo(serv_sock, tmp_seg, content, seg_size);
        seq = tmp_seg->seq;

        segment[seq]->seq = tmp_seg->seq;
        memcpy(segment[seq]->content, content, seg_size);
        segment[seq]->size = tmp_seg->size;
        
		end = clock();
		send_time = (double)(end - start) / CLOCKS_PER_SEC;
        updateSendInfo(snd_info, segment[seq]->size, send_time);

        all_seg_flag[seq] = 1;
        recv_seg_num[sender_seg_cnt] = seq;
        sender_seg_cnt++;
    }

    return NULL;
}

/* send segment to peers thread */
void * sendSeg2Peers(void* arg)
{
    int sock = *(int *)arg;
    int seq, i = 0;

    while (i < seg2recv_num) {
        seq = recv_seg_num[i];
        if (seq > -1) {
            writeSegmentInfo(sock, segment[seq]);
            i++;
        }
    }
    
    return NULL;
}

/* recv segment from peer thread */
void * recvSegFromPeer(void * arg)
{
    int send_time, seq = 0, i = 0;
    int recv_idx = *(int *)arg;
    int sock = recv_socks[recv_idx]; 
    clock_t start, end;

    Segment * tmp_seg = (Segment *)malloc(sizeof(Segment));
    tmp_seg->content = (char *)malloc(seg_size);
    char * content = (char *)(malloc(seg_size));
    start = clock();

    // peer sock로부터 받기
    while (remain_seg_num > 0) {
        readSegmentInfo(sock, tmp_seg, content, seg_size);
        seq = tmp_seg->seq;

        segment[seq]->seq = tmp_seg->seq;
        memcpy(segment[seq]->content, content, seg_size);
        segment[seq]->size = tmp_seg->size;
        
        all_seg_flag[seq] = 1;
        i++;
        pthread_mutex_lock(&remain_mutx);
        remain_seg_num--;
        pthread_mutex_unlock(&remain_mutx);

        end = clock();
		send_time = (double)(end - start) / CLOCKS_PER_SEC;
        updateRecvInfo(rcv_info[recv_idx], segment[seq]->size, send_time);
    }

    return NULL;
}

/* write segment to file thread */
void * writeSeg2File(void * arg)
{
    char * filename = (char *)arg;
    int i = 0;
    FILE * fp;
    char buffer[NAME_LEN];
    sprintf(buffer, "%d_%s", recv_num, filename);

    if ((fp = fopen(buffer, "wb")) == NULL) {
		printf("\nFailed to open file.\n");
	} else {
        while (i < total_seg) {
            if (all_seg_flag[i] == 1) {
                int fwrite_len = fwrite(segment[i]->content, sizeof(char), segment[i]->size, fp);
                stored_size += segment[i]->size;

                i++;
            }
        }
        fclose(fp);
    }

    return NULL;
}

void * printRecvProgress()
{
    clock_t start, end;
	int bar_width;
    int file_size, total_seg, seg_num, cur_size, size_per_recv;
	double sec, total_sec, size_per_sec = 0, percent = 0;
	char tmp[BUF_SIZE];

    start = clock();
	clrscr();
	EnableCursor(0);

    while(1) {
        getSendInfo(snd_info, &file_size, &cur_size, &sec);
		if (sec > 0)
			size_per_sec = (double)cur_size / sec;
        gotoxy(0, 0);
        printf("\nFrom Sending Peer : %.2lf Mbps (%d Bytes Sent / %.2lfs)     \n",
				size_per_sec, cur_size, sec);
        total_sec = sec;

		// print receiver progress bar
		for (int i = 0; i < recv_cnt; i++) {
			getRecvInfo(rcv_info[i], &total_seg, &seg_num, &size_per_recv, &sec);
            if (sec > 0)
			    size_per_sec = (double)size_per_recv / sec;

			printf("From Receiving Peer #%d : %.2lf Mbps (%d Bytes Sent / %.2lfs)     \n", i+1, size_per_sec, size_per_recv, sec);
			cur_size += size_per_recv;
            total_sec += sec;
		}
        printf("\n");

        // total recv size, file save size
        if (total_sec > 0)
			size_per_sec = (double)cur_size / total_sec;
        percent = (double)cur_size / (double)file_size;

        sprintf(tmp, "%d", file_size);
        bar_width = getWindowWidth() - (strlen("Total receive size [] %% ( /  Bytes)  Mbps (s)     \n")
				+ 5 + strlen(tmp) * 3 + 3 + 10);     
        printf("Total receive size [");
		printBar(bar_width, percent);
		printf("] %3.2lf%% (%d / %d Bytes) %.2lf Mbps (%.2lfs)     \n",
				100.0 * percent, cur_size, file_size, size_per_sec, sec);

        percent = (double)stored_size / (double)file_size;
        bar_width += strlen("Total receive size") - strlen("File Save");
        printf("File Save [");
		printBar(bar_width, percent);
		printf("] %3.2lf%% (%d / %d Bytes)     \n",
				100.0 * percent, stored_size, file_size);

        // TODO: receiving peer id 추가
		if (cur_size >= file_size && stored_size >= file_size) {
			printf("\ncur_size: %d, file_size: %d\n", cur_size, file_size);
            EnableCursor(1);
            break;
        }
    }
	EnableCursor(1);

    return NULL;
}