/*
	실행: ./server.exe <ip> <port>

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
#include <dirent.h>
#include "file_info.h"

#define BUF_SIZE 1024
void send_file_list(int clnt_sock);
void send_file_content(int clnt_sock, char * filename);
void error_handling(char *message);

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	char message[BUF_SIZE];
	int str_len, i;
	
	struct sockaddr_in serv_adr;
	struct sockaddr_in clnt_adr;
	socklen_t clnt_adr_sz;
	
	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);   
	if (serv_sock == -1)
		error_handling("socket() error");
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	if (bind(serv_sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	
	if (listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	
	clnt_adr_sz = sizeof(clnt_adr);

	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	if (clnt_sock == -1)
		error_handling("accept() error");
	else
		printf("Connected client %d \n", i+1);

	// client가 message를 주면 디렉토리의 모든 파일 목록 (파일 이름, 파일 크기)을 클라이언트에게 전송 
	read(clnt_sock, message, BUF_SIZE);
	send_file_list(clnt_sock);

	shutdown(clnt_sock, SHUT_WR);
	close(clnt_sock);

	// 다시 socket열기
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
	if (clnt_sock == -1)
		error_handling("accept() error");
	else
		printf("Connected client %d \n", i+1);

	// 클라이언트가 선택한 파일읽고 내용 전송
	read(clnt_sock, message, BUF_SIZE);
	printf("\nfile name: '%s'\n", message);
	send_file_content(clnt_sock, message);
	
	shutdown(clnt_sock, SHUT_WR);
	close(clnt_sock);

	close(serv_sock);
	return 0;
}

/* send file list from current directory */
void send_file_list(int clnt_sock)
{
	DIR *dir;
	FileInfoPacket* file;
	struct dirent* directory;
	int dirSize, i = 0;
	
	dir = opendir ("./");
	file = (FileInfoPacket *) malloc(sizeof(FileInfoPacket));
	directory = (struct dirent*)malloc((int)sizeof(struct dirent) * 100);
	int write_len = 0;

	if (dir != NULL) {
		//  실행 중인 디렉토리의 모든 파일 목록 (파일 이름, 파일 크기)을 클라이언트에게 전송 
		while ((directory = readdir(dir)) != NULL) {
			if (directory->d_type == DT_REG) {
				strcpy(file->name, directory->d_name);
				file->size = filesize(directory->d_name);
				printf ("%s %d\n", directory->d_name, file->size);

				write_len = write(clnt_sock, file, sizeof(FileInfoPacket));
				// printf("write_len: %d\n", write_len);
				i++;
			}
		}
		closedir (dir);
	} else {
		/* could not open directory */
		perror ("Error: could not open directory");
		exit(EXIT_FAILURE);
	}					
}

/* send file content specific filename */
void send_file_content(int clnt_sock, char * filename)
{
	FILE * fp;
	char * result;
	int line_size;
	char input[BUF_SIZE];

	// file open and read
	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
		printf("file content is\n");
		
		while (feof(fp) == 0) {
			line_size = fread(input, 1, BUF_SIZE, fp);
		
			printf("%s\n", input);
			strlen(input);
			write(clnt_sock, input, line_size);
		}
	}
	fclose(fp);
}

/* error handling */
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}