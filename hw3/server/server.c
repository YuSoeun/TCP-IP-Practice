/*
	• 클라이언트가 서버에 접속하여 서버의 디렉토리와 파일정보를 확인할 수 있는 프로그램을 작성하세요.
	• 클라이언트가 서버에 접속하면, 서버는 서버 프로그램이 실행되고 있는 디렉토리 위치 및 파일 정보(파일명 + 파일 크기)를 클라이언트에게 전달합니다.
	• 클라이언트는 서버로부터 수신한 정보들을 출력합니다.
	• 클라이언트는 서버의 디렉토리를 변경해가며 디렉토리 정보와 디렉토리 안에있는 파일 정보(파일명 + 파일크기)를 확인할 수 있습니다.
	• 클라이언트가 원하는 파일을 서버로부터 다운로드 받을 수 있어야 합니다.
	• 클라이언트는 자신이 소유한 파일을 서버에게 업로드할 수 있어야 합니다.
	• 클라이언트가 서버에 접속하면 클라이언트는 서버 프로그램의 권한과 동일한 권한을 갖습니다.
	즉, 서버 프로그램에게 허용되지 않은 디렉토리와 파일은 클라이언트도 접근할 수 없습니다.
	• 서버는 여러 클라이언트의 요청을 동시에 지원할 수 있어야 합니다.
		→I/O Multiplexing 사용!
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <time.h>
#include "file_info.h"

#define BUF_SIZE 1024
void send_file_list(int clnt_sock);
void send_file_content(int clnt_sock, char *filename);
int read_and_save_file(int clnt_sock, char *filename);
void error_handling(char *message);

int main(int argc, char *argv[])
{
	char message[BUF_SIZE];
	char dir_name[NAME_LEN] = "./";
	char buf[BUF_SIZE];

	FileInfoPacket* recv_pkt;
	struct sockaddr_in serv_adr;
	struct sockaddr_in clnt_adr;
	int serv_sock, clnt_sock, flag;
	socklen_t clnt_adr_sz;

	int str_len, i, fd_num, fd_max;
	fd_set reads, cpy_reads;
	struct timeval timeout;
	
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

	FD_ZERO(&reads);
	FD_SET(serv_sock, &reads);
	fd_max = serv_sock;

	while (1) {
		cpy_reads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;

		if ((fd_num = select(fd_max+1, &cpy_reads, 0, 0, &timeout)) == -1)
			break;
		
		if (fd_num == 0)
			continue;

		for (i = 0; i < fd_max + 1; i++) {
			if (FD_ISSET(i, &cpy_reads)) {
				if (i == serv_sock) {					// connection request!
					clnt_adr_sz = sizeof(clnt_adr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
					FD_SET(clnt_sock, &reads);
					if (fd_max < clnt_sock)
						fd_max = clnt_sock;
					printf("connected client: %d \n", clnt_sock);
				} else {				 				// read message!
					strcpy(dir_name, "./");
					recv_pkt = (FileInfoPacket *) malloc(sizeof(FileInfoPacket));

					// client에게 시작한다는 message 받기
					read(clnt_sock, message, BUF_SIZE);

					// 현재 디렉토리 위치 보내기
					write(clnt_sock, dir_name, NAME_LEN);

					// 디렉토리의 모든 파일 목록 (파일 이름, 파일 크기)을 클라이언트에게 전송 
					send_file_list(clnt_sock);
					if (read(clnt_sock, recv_pkt, sizeof(FileInfoPacket)) <= 0) {
						FD_CLR(i, &reads);
						close(i);
						printf("closed client: %d \n", i);
						break;
					}

					if (strcmp(recv_pkt->type, "dir") == 0) {				// 디렉토리일 경우
						strcat(dir_name, recv_pkt->name);
						strcat(dir_name, "/");
						printf("%s\n", dir_name);
						flag = chdir(dir_name);
						if (read(clnt_sock, recv_pkt, sizeof(FileInfoPacket)) <= 0) {
							break;
						}

						printf("flag:  %d\n", flag);
					} else if (strcmp(recv_pkt->type, "file") == 0) {		// 파일 다운 요청일 경우
						send_file_content(clnt_sock, recv_pkt->name);
					} else if (strcmp(recv_pkt->type, "upload") == 0) {		// 파일 업로드일 경우
						read_and_save_file(clnt_sock, recv_pkt->name);
					}
				}
			}
		}
	}
	
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
	
	dir = opendir("./");
	file = (FileInfoPacket *) malloc(sizeof(FileInfoPacket));
	directory = (struct dirent*)malloc((int)sizeof(struct dirent) * 100);
	int write_len = 0;

	if (dir != NULL) {
		//  실행 중인 디렉토리의 모든 파일 목록 (파일 이름, 파일 크기)을 클라이언트에게 전송 
		while ((directory = readdir(dir)) != NULL) {
			if (directory->d_type == DT_REG) {
				strcpy(file->name, directory->d_name);
				file->size = filesize(directory->d_name);
				strcpy(file->type, "file");
				printf ("%s %d\n", directory->d_name, file->size);

				write_len = write(clnt_sock, file, sizeof(FileInfoPacket));
				i++;
			} else if (directory->d_type == DT_DIR) {
				strcpy(file->name, directory->d_name);
				file->size = filesize(directory->d_name);
				strcpy(file->type, "dir");
				printf ("%s %d\n", directory->d_name, file->size);

				write_len = write(clnt_sock, file, sizeof(FileInfoPacket));
				i++;
			}
		}
		closedir (dir);
		// 디렉토리 목록 끝났음을 알리기
		strcpy(file->name, "디렉토리 목록을 다 읽었습니다.");
		file->size = -1;
		write(clnt_sock, file, sizeof(FileInfoPacket));
	} else {
		/* could not open directory */
		perror ("Error: could not open directory");
		exit(EXIT_FAILURE);
	}					
}

/* send file content specific filename */
void send_file_content(int clnt_sock, char *filename)
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
	close(clnt_sock);
}

/* read file content from client and save file */
int read_and_save_file(int clnt_sock, char *filename)
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
			printf("%s\n", line);
			
			fwrite(line, sizeof(char), recv_cnt, fp);
		}
	}
	fclose(fp);
	close(clnt_sock);

	return 1;
}

/* error handling */
void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}