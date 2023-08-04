#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "packet.h"

void error_handling(char *message);

int main(int argc, char *argv[])
{
	int sock, pre_seq = -1;
	char message[MAX_BUF];
	int str_len = 1;
	socklen_t adr_sz;
	packet* pkt, *ack;
	char filename[MAX_BUF];
	FILE * fp;
	
	// socket setting
	struct sockaddr_in serv_adr, from_adr;
	if (argc!=3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = socket(PF_INET, SOCK_DGRAM, 0);   
	if (sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
	serv_adr.sin_port = htons(atoi(argv[2]));
	
	// get filename from user
<<<<<<< HEAD
	printf("'hi.txt' 파일이 있습니다.\n");
	printf("파일을 다운받으려면 파일 이름을 입력하시오: ");
=======
	fputs("'hi.txt, 12.png' 파일이 있습니다.\n", stdout);
	fputs("파일을 다운받으려면 파일 이름을 입력하시오: ", stdout);
>>>>>>> d870ad3fb3ecc2ab360f704ab1c9403b7e70a58c
	scanf("%s", filename);
	printf("'%s'", filename);
	sendto(sock, filename, 100, 0, 
				(struct sockaddr*)&serv_adr, sizeof(serv_adr));

	// open file and restore received data
	pkt = (packet *) malloc(sizeof(packet));
	ack = (packet *) malloc(sizeof(packet));

	if ((fp = fopen(filename, "wb")) == NULL) {
		printf("Failed to open file.\n");
		return 0;
	} else {
		while (str_len > 0) {
			adr_sz = sizeof(from_adr);
			
			str_len = recvfrom(sock, pkt, sizeof(packet), 0, 
						(struct sockaddr*)&from_adr, &adr_sz);
			if (pkt->seq == -1) break;							// 파일 끝났음 의미
			
			printf("seq[%d]: '%s'\n", pkt->seq, pkt->data);
			sendto(sock, pkt, sizeof(packet), 0, 
						(struct sockaddr*)&serv_adr, sizeof(serv_adr));

			// 이전에 받았던 seq num이면 안쓰기
			if (pre_seq < pkt->seq) {
				fwrite(pkt->data, sizeof(char), pkt->data_size, fp);
			}
			pre_seq = pkt->seq;
		}
		fclose(fp);
	}

	close(sock);
	return 0;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}