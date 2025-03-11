/*
	실행: ./c.exe <ip> <port>

	* TCP 기반 파일 다운로드 프로그램 구현
	1. 클라이언트가 서버에 접속 (TCP 이용) 
	2. 서버 프로그램이 실행 중인 디렉토리의 모든 파일 목록 (파일 이름, 파일 크기)을 클라이언트에게 전송 
	3. 클라이언트는 서버가 보내 온 목록을 보고 파일 하나를 선택 
	4. 서버는 클라이언트가 선택한 파일을 클라이언트에게 전송 
	5. 전송된 파일은 클라이언트 프로그램이 실행 중인 디렉토리에 동일한 이름으로 저장됨. 
	6. 2~5번 과정 반복 

	- 사용자 Interface는 자유롭게 해도 됨. 단, 사용하기 쉽도록 메뉴나 명령어에 대한 설명 필요 
	- 텍스트 파일 뿐만 아니라 바이너리 파일도 전송할 수 있어야 함
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "file_info.h"
#include "trim.h"

#define BUF_SIZE 1024
int open_socket(char *argv[]);
void error_handling(char *message);
FileInfoPacket* read_file_list(int clnt_sock);
int read_and_save_file(int clnt_sock, char* filename);

int main(int argc, char *argv[])
{
	int sock;
	char message[BUF_SIZE];
	char filename[BUF_SIZE];
	int str_len, recv_len, recv_cnt;

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = open_socket(argv);
	
	// 디렉토리의 모든 파일 목록 받기
	fputs("서버 디렉토리의 파일 목록 확인하기 (yes to continue): ", stdout);
	fgets(message, BUF_SIZE, stdin);

	str_len = write(sock, message, strlen(message));
	read_file_list(sock);
	close(sock);

	// 파일 목록 중 하나 선택, 이름 서버에 전송
	sock = open_socket(argv);
	fputs("받을 파일의 이름을 적어주세요: ", stdout);
	scanf("%s", filename);
	printf("%s\n", filename);
	write(sock, filename, BUF_SIZE);

	// 서버에서 받아온 파일 저장
	read_and_save_file(sock, filename);
	printf("%s is saved.\n", filename);
	
	close(sock);
	return 0;
}

/* open socket and connect server */
int open_socket(char *argv[])
{
	int sock;
	struct sockaddr_in serv_adr;

	sock = socket(PF_INET, SOCK_STREAM, 0);   
	if (sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));
	
	if (connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("connect() error!");
	else
		puts("Connected...........");
	
	return sock;
}

/* read file list from server */
FileInfoPacket* read_file_list(int clnt_sock)
{
	FileInfoPacket* file;
	int recv_cnt, recv_len = 0, buffer;
	char temp[BUF_SIZE] = {0};

	file = (FileInfoPacket*)malloc((int)sizeof(FileInfoPacket));
	
	while ((recv_cnt = read(clnt_sock, file, sizeof(FileInfoPacket))) != 0) {
		// printf("\npacket size: %d\n", recv_cnt);
		printf("name: %s, size: %d\n", file->name, file->size);

		// sizeof(FileInfoPacket)만큼 읽어들이지 못했다면 
		while (recv_cnt < sizeof(FileInfoPacket)) {
			buffer = read(clnt_sock, temp, sizeof(FileInfoPacket) - recv_cnt);
			// printf("buffer[%dbyte]: %s\n", buffer, temp);
			recv_cnt += buffer;
		}
	}
	printf("\n");

	if(recv_cnt == -1)
	error_handling("read() error!");

	return file;
}

/* read file content from server and save file */
int read_and_save_file(int clnt_sock, char* filename)
{
	char line[BUF_SIZE];
	int recv_cnt, recv_len = 0;
	FILE * fp;

	// file open and read
	if ((fp = fopen(filename, "wb")) == NULL) {
		printf("Failed to open file.\n");
		return 0;
	} else {
		while ((recv_cnt = read(clnt_sock, line, BUF_SIZE)) != 0) {
			// printf("%s\n", line);
			
			fwrite(line, sizeof(char), recv_cnt, fp);
		}
	}
	fclose(fp);

	return 1;
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}