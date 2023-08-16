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
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "Console.h"
#include "client_server.h"
#include "socket.h"
#include "file.h"

void * acceptReceiver(void * arg);
void * connectReceiver(void * arg);
void * sendMsg(void * arg);
void * sendSeg2Peers(void* arg);
void * getSegmentFromSock(int serv_sock, int seg2RecvNum);
void * recvSegFromPeer(void * arg);
void * writeSeg2File(void * arg);

int recv_cnt = 0;
int recv_num;                   // connect를 요청해야할 thread 수
int total_recv = __INT_MAX__;   // 총 recever 수
int remain_seg_num;             // segment를 더 받아야 할 개수
int * recv_socks;

SocketInfo ** other_recv_info;
pthread_mutex_t clnt_mutx, remain_mutx;

Segment ** segment;
int seg_size, total_seg;
int seg2RecvNum;                 // Sender로부터 받아야 할 segment 수
int * all_seg_flag;              // 전체 segment 써졌는지 표시하는 flag
int * recv_seg_num;              // sender에게 받은 segment seq 표시

int client(int listen_port, char * ip, int port)
{
    int serv_sock, clnt_sock;
    int recv_sock, recv_adr_sz;
	struct sockaddr_in serv_addr, clnt_adr, recv_addr;
	pthread_t acpt_thread, rcv_thread, write_file_thread;
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
    recv_socks = (int *)malloc(sizeof(int) * total_recv);
    printf("total_recv: %d\n", total_recv);

    read(serv_sock, &recv_num, sizeof(int));
    cnct_thread = (pthread_t *)malloc(sizeof(recv_num) * recv_num);
    other_recv_info = (SocketInfo **)malloc(sizeof(SocketInfo *) * recv_num);
	for (int i = 0; i < recv_num; i++) {
		other_recv_info[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
	}
    
    // Sender에서 receiver_sock 정보 받고 다른 Receiver connect
    for (int i = 0; i < recv_num; i++) {
        readSocketInfo(serv_sock, other_recv_info[i]);
        pthread_create(&cnct_thread[i], NULL, connectReceiver, (void *)other_recv_info[i]);
    }
    
    // 파일 이름, segment 총 수 받기
    int fname_size;
    char * filename;

    read(serv_sock, &fname_size, sizeof(int));
    filename = malloc(fname_size);
    recvStr(serv_sock, filename, fname_size);
    read(serv_sock, &seg_size, sizeof(int));
    read(serv_sock, &total_seg, sizeof(int));
    printf("전체 segment 개수: %d\n", total_seg);

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

    // Sender에게 init complete all_seg_flag 보내기
    printf("Init complete\n");
    memcpy(msg, "Init complete", BUF_SIZE);
    write(serv_sock, msg, BUF_SIZE);
    
    // sender로부터 receive해야할 개수 읽기
    read(serv_sock, &seg2RecvNum, sizeof(int));
    printf("sender로부터 receive해야할 개수: %d\n", seg2RecvNum);
    
    // 다른 Receiver segment read하고 write하는 thread 열어놓기
    remain_seg_num = total_seg - seg2RecvNum;
    snd_thread = (pthread_t *)malloc(recv_cnt * sizeof(pthread_t));
    recv_seg_num = malloc(seg2RecvNum * sizeof(int));
    memset(recv_seg_num, -1, seg2RecvNum * sizeof(int));
    for (int i = 0; i < recv_cnt; i++) {
        pthread_create(&rcv_thread, NULL, recvSegFromPeer, (void *)&recv_socks[i]);
        pthread_create(&snd_thread[i], NULL, sendSeg2Peers, (void *)&recv_socks[i]);
    }

    // consumer thread 만들기
    pthread_create(&write_file_thread, NULL, writeSeg2File, (void *)filename);

    // Sender로부터 segmaent 받아오기
    getSegmentFromSock(serv_sock, seg2RecvNum);
    printf("\nSender로 부터 모든 socket을 받았습니다.\n");

    // 다른 recv에게 seg 다 보냈는지 출력
    for (int i = 0; i < recv_cnt; i++) {
        pthread_join(snd_thread[i], &thread_return);
        printf("\n%d번째 receiver에게 모든 segment을 보냈습니다.\n", i);
    }
    pthread_join(write_file_thread, &thread_return);
    printf("\n파일에 받아온 정보를 다 적었습니다.\n");

    pthread_detach(rcv_thread);
    printf("\n다른 receiver로부터 모든 segment를 받았습니다.\n");

    // free segment
    for (int i = 0; i < total_seg; i++) {
        free(segment[i]->content);
        free(segment[i]);
    }
    free(segment);
    
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

    while (recv_cnt < total_recv-1) {
        recv_adr_sz = sizeof(recv_adr);
        if ((recv_sock = accept(*clnt_sock, (struct sockaddr *)&recv_adr, &recv_adr_sz)) == -1) {
            perror("accept error");
        } else {
            pthread_mutex_lock(&clnt_mutx);
            recv_socks[recv_cnt++] = recv_sock;
            pthread_mutex_unlock(&clnt_mutx);
            printf("accept receiver: %d\n", *clnt_sock);
        }
    }
}

/* connect receiver thread */
void * connectReceiver(void * arg)
{
    SocketInfo * recv_info = (SocketInfo  *)arg;
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

    if (connect(recv_sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr)) == -1) {
        perror("connect() error");
    } else {
        pthread_mutex_lock(&clnt_mutx);
        recv_socks[recv_cnt++] = recv_sock;
        pthread_mutex_unlock(&clnt_mutx);
        printf("connect to ip:%s, port:%d\n", inet_ntoa(recv_addr.sin_addr), ntohs(recv_addr.sin_port));
    }
}

/* get segment from sock and save to segment */
void * getSegmentFromSock(int serv_sock, int seg2RecvNum)
{
    int seq = 0;
    int i = 0;
    Segment * tmp_seg = (Segment *)malloc(sizeof(Segment));
    tmp_seg->content = (char *)malloc(seg_size);
    char * content = (char *)(malloc(seg_size));
    pthread_t * send_seg_thread;

    // seg server로부터 받기
    while (i < seg2RecvNum) {
        readSegmentInfo(serv_sock, tmp_seg, content, seg_size);
        seq = tmp_seg->seq;

        // 필요할 때만 segment malloc
        segment[seq] = (Segment *)malloc(sizeof(Segment));
        segment[seq]->content = (char *)malloc(seg_size);

        segment[seq]->seq = tmp_seg->seq;
        memcpy(segment[seq]->content, content, seg_size);
        segment[seq]->size = tmp_seg->size;
        printf("[Sender] - seg[%d]: %s (%d)\n", segment[seq]->seq, segment[seq]->content, segment[seq]->size);
        
        all_seg_flag[seq] = 1;
        recv_seg_num[i] = seq;
        i++;
    }

    return NULL;
}

/* send segment to peers thread */
void * sendSeg2Peers(void* arg)
{
    int sock = *(int *)arg;
    int seq, i = 0;

    while (i < seg2RecvNum) {
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
    int seq = 0;
    int i = 0;
    int sock = *(int *)arg; 
    Segment * tmp_seg = (Segment *)malloc(sizeof(Segment));
    tmp_seg->content = (char *)malloc(seg_size);
    char * content = (char *)(malloc(seg_size));
    pthread_t * send_seg_thread;

    // peer sock로부터 받기
    while (remain_seg_num > 0) {
        readSegmentInfo(sock, tmp_seg, content, seg_size);
        seq = tmp_seg->seq;

        // 필요할 때만 segment malloc
        segment[seq] = (Segment *)malloc(sizeof(Segment));
        segment[seq]->content = (char *)malloc(seg_size);

        segment[seq]->seq = tmp_seg->seq;
        memcpy(segment[seq]->content, content, seg_size);
        segment[seq]->size = tmp_seg->size;
        printf("[Receiver] - seg[%d]: %s (%d)\n", seq, segment[seq]->content, segment[seq]->size);
        
        all_seg_flag[seq] = 1;
        recv_seg_num[i] = seq;
        i++;
        pthread_mutex_lock(&remain_mutx);
        remain_seg_num--;
        pthread_mutex_unlock(&remain_mutx);
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
        printf("\n파일을 열었습니다.\n");
        while (i < total_seg) {
            if (all_seg_flag[i] == 1) {
                int fwrite_len = fwrite(segment[i]->content, sizeof(char), segment[i]->size, fp);
                printf("[File Save] - seg[%d]: %s\n", i, segment[i]->content);

                i++;
            }
        }
        fclose(fp);
    }

    return NULL;
}