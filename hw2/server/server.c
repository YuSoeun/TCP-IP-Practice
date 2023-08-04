/*
    Stop-and-Wait Protocol 구현
    • uecho_server.c와 uecho_client.c는 신뢰성 있는 데이터 전송을 보장하지 않습니다. (손실된 패킷을 복구하는 기능이 없음)
    • 이를 보완하여 신뢰성 있는 데이터 전송을 보장하는 Stop-and-WaitProtocol 기반의 프로그램을 구현하세요.
    • 파일전송이 완료되면 Throughput을 출력하세요.(Throughput = 받은 데이터양/전송 시간)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <time.h>
#include "packet.h"

// int get_file_list(packet** pkt_list);
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock, str_len;
	int pkt_count = 0;
	double throughput, total_size = 0;
	double total_time = 0;
	char message[MAX_BUF];
	clock_t start, end;
	socklen_t clnt_adr_sz;
	packet* pkt, *ack;

	FILE * fp;
	char * result;
	char filename[MAX_BUF];
	int line_size;
	char input[MAX_BUF];
	int i = 0;
	
    // socket setting
	struct sockaddr_in serv_adr, clnt_adr;
	if(argc!=2){
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(serv_sock == -1)
		error_handling("UDP socket creation error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	// file open and send
	pkt = (packet *) malloc(sizeof(packet));
	ack = (packet *) malloc(sizeof(packet));

	clnt_adr_sz = sizeof(clnt_adr);
	str_len = recvfrom(serv_sock, filename, 100, 0, 
							(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	printf("file name: %s\n", filename);

	// set socket receive timeout: 50ms
	struct timeval optVal = {0, 50000};
	int optLen = sizeof(optVal);
	setsockopt(serv_sock, SOL_SOCKET, SO_RCVTIMEO, &optVal, optLen);	
	
	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
		// send and receive data
		ack->seq = -1;

		while (feof(fp) == 0) {
			line_size = fread(input, 1, MAX_BUF-1, fp);
			input[line_size] = 0;
			// printf("\nline size: %d\n", line_size);
		
			pkt->seq = i;
			// memcpy같은거 써야함 null값까지만 복사해서
			memcpy(pkt->data, input, MAX_BUF);
			pkt->data_size = line_size;

			str_len = -1;
			while (str_len == -1) {
				clnt_adr_sz = sizeof(clnt_adr);
				sendto(serv_sock, pkt, sizeof(packet), 0, 
										(struct sockaddr*)&clnt_adr, clnt_adr_sz);
				printf("seq[%d]: '%s'\n", pkt->seq, pkt->data);	
				
				str_len = recvfrom(serv_sock, ack, sizeof(packet), 0, 
										(struct sockaddr*)&clnt_adr, &clnt_adr_sz);
				if (ack->seq < pkt->seq) {
					printf("----loss----\n");
				}
				// printf("<ack> seq[%d]: '%s'\n", ack->seq, ack->data);	
			}
			if (i == 0) start = clock();
			// if (i >= 4) break;
			i++;
			total_size += str_len;
		}

		// send -1 to notice EOF
		pkt->seq = -1;
		strcpy(pkt->data, "end");
		sendto(serv_sock, pkt, sizeof(packet), 0, 
										(struct sockaddr*)&clnt_adr, clnt_adr_sz);
	}
	end = clock();
	
	fclose(fp);
	close(serv_sock);
	
	// calculate throutput
	total_time = (double)(end - start) / CLOCKS_PER_SEC;
	throughput = total_size/total_time;
	printf("\ntotal_size : %f(MB)\n", total_size);
	printf("total_time: %f(s)\n", total_time);
	printf("throughput : %f (MB/s)\n", throughput);
	
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}