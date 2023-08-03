/*
	• 클라이언트가 서버에 접속하여 서버의 디렉토리와 파일정보를 확인할 수 있는 프로그램을 작성하세요.
	• 클라이언트가 서버에 접속하면, 서버는 서버 프로그램이 실행되고 있는 디렉토리 위치 및 파일 정보(파일명 + 파일 크기)를 클라이언트에게 전달합니다.
	• 클라이언트는 서버로부터 수신한 정보들을 출력합니다.
	• 클라이언트는 서버의 디렉토리를 변경해가며 디렉토리 정보와 디렉토리 안에있는 파일 정보(파일명 + 파일크기)를 확인할 수 있습니다.
	• 클라이언트가 원하는 파일을 서버로부터 다운로드 받을 수 있어야 합니다.
	• 클라이언트는 자신이 소유한 파일을 서버에게 업로드할 수 있어야 합니다.
	• 클라이언트가 서버에 접속하면 클라이언트는 서버 프로그램의 권한과 동일한 권한을 갖습니다. 즉, 서버 프로그램에게 허용되지 않은 디렉토리와 파일은 클라이언트도 접근할 수 없습니다.
	• 서버는 여러 클라이언트의 요청을 동시에 지원할 수 있어야 합니다.
		→I/O Multiplexing 사용!
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
#define FLIST_SIZE 100
int open_socket(char *argv[]);
void error_handling(char *message);

int read_file_list(int sock, FileInfoPacket **files);
int read_and_save_file(int sock, char* filename);
void send_file_content(int sock, char *filename);

void print_file_list(FileInfoPacket **files, int file_cnt);
void print_directories(FileInfoPacket **files, int file_cnt);
void print_files(FileInfoPacket **files, int file_cnt);

int main(int argc, char *argv[])
{
	int sock, file_cnt, num, file_num;
	char message[BUF_SIZE];
	char filename[NAME_LEN];
	char dirname[NAME_LEN];
	int str_len, recv_len, recv_cnt;

	FileInfoPacket** files;
	files = (FileInfoPacket**)malloc((int)sizeof(FileInfoPacket*) * FLIST_SIZE);
	for (int i = 0; i < FLIST_SIZE; i++) {
		files[i] = (FileInfoPacket*)malloc((int)sizeof(FileInfoPacket));
	}

	if (argc != 3) {
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	
	sock = open_socket(argv);
	
	while (1) {
		// 디렉토리의 모든 파일 목록 받기
		strcpy(message, "start");
		write(sock, message, BUF_SIZE);

		recv_cnt = read(sock, dirname, NAME_LEN);
		while (recv_cnt < NAME_LEN) {
			recv_cnt += read(sock, &dirname[recv_cnt], NAME_LEN - recv_cnt);
		}
		
		file_cnt = read_file_list(sock, files);
		print_file_list(files, file_cnt);

		fputs("\n----- Menu ----\n", stdout);
		fputs("1. 디렉토리 이동\n", stdout);
		fputs("2. 파일 가져오기\n", stdout);
		fputs("3. 파일 업로드\n", stdout);
		fputs("4. 프로그램 종료\n", stdout);
		fputs("1 ~ 4 중 할 번호를 선택하세요: ", stdout);
		scanf("%d", &num);

		if (num == 1) {					// 1. 디렉토리 이동
			printf("\ncurrent directory: %s\n", dirname);
			print_directories(files, file_cnt);
			fputs("\n이동할 디렉토리 번호를 입력해주세요: ", stdout);
			scanf("%d", &file_num);
			printf("%d을 선택하였습니다.\n", file_num);
			write(sock, files[file_num], sizeof(FileInfoPacket));
		} else if (num == 2) {			// 2. 파일 가져오기
			print_files(files, file_cnt);
			fputs("\n받을 파일의 번호을 적어주세요: ", stdout);
			scanf("%d", &file_num);
			printf("%d을 선택하였습니다.\n", file_num);
			write(sock, files[file_num], sizeof(FileInfoPacket));

			read_and_save_file(sock, files[file_num]->name);
			printf("%s is saved.\n", files[file_num]->name);

			break;
		} else if (num == 3) {			// 3. 파일 업로드
			fputs("\n업로드 할 파일 이름을 적어주세요: ", stdout);
			scanf("%s", filename);
			printf("'%s' 업로드를 시작합니다.\n", filename);
			strcpy(files[0]->name, filename);
			strcpy(files[0]->type, "upload");
			write(sock, files[0], sizeof(FileInfoPacket));

			send_file_content(sock, files[0]->name);
			break;
		} else if (num == 4) {
			break;
		}
	}

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
int read_file_list(int sock, FileInfoPacket **files)
{
	int i = 0;
	int recv_cnt, recv_len = 0, buffer;
	char temp[BUF_SIZE] = {0};
	
	while ((recv_cnt = read(sock, files[i], sizeof(FileInfoPacket))) != 0) {
		// sizeof(FileInfoPacket)만큼 읽어들이지 못했다면 
		while (recv_cnt < sizeof(FileInfoPacket)) {
			buffer = read(sock, temp, sizeof(FileInfoPacket) - recv_cnt);
			recv_cnt += buffer;
		}

		if (files[i]->size < 0) {
			printf("%s\n", files[i]->name);
			break;
		}

		i++;
	}

	if(recv_cnt == -1)
	error_handling("read() error!");

	return i;
}

/* read file list from server */
void print_file_list(FileInfoPacket **files, int file_cnt)
{
	printf("\n---- file list ----\n");
	for (int i = 0; i < file_cnt; i++) {
		printf("[%s] %d. '%s', (%d Byte)\n", files[i]->type, i, files[i]->name, files[i]->size);
	}
	printf("\n");
}

void print_directories(FileInfoPacket **files, int file_cnt)
{
	printf("\n---- directory list ----\n");
	for (int i = 0; i < file_cnt; i++) {
		if (strcmp(files[i]->type, "dir") == 0) {
			printf("[%s] %d. '%s', (%d Byte)\n", files[i]->type, i, files[i]->name, files[i]->size);
		}
	}
	printf("\n");
}

void print_files(FileInfoPacket **files, int file_cnt)
{
	printf("\n---- file list ----\n");
	for (int i = 0; i < file_cnt; i++) {
		if (strcmp(files[i]->type, "file") == 0) {
			printf("[%s] %d. '%s', (%d Byte)\n", files[i]->type, i, files[i]->name, files[i]->size);
		}
	}
	printf("\n");
}

/* read file content from server and save file */
int read_and_save_file(int sock, char* filename)
{
	char line[BUF_SIZE];
	int recv_cnt, recv_len = 0;
	FILE * fp;

	// file open and write received content
	if ((fp = fopen(filename, "wb")) == NULL) {
		printf("Failed to open file.\n");
		return 0;
	} else {
		while ((recv_cnt = read(sock, line, BUF_SIZE)) != 0) {
			printf("%s\n", line);
			if (strcmp(line, "eof") == 0)
				break;
			
			fwrite(line, sizeof(char), recv_cnt, fp);
		}
	}
	fclose(fp);
	printf("fclose(fp);\n");

	return 1;
}

/* send file content specific filename */
void send_file_content(int sock, char *filename)
{
	FILE * fp;
	char * result;
	int line_size;
	char input[BUF_SIZE];

	printf("여기\n");

	// file open and read
	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
		printf("file content is\n");
		
		while (feof(fp) == 0) {
			line_size = fread(input, 1, BUF_SIZE, fp);
		
			printf("%s\n", input);
			strlen(input);
			write(sock, input, line_size);
		}
	}
	fclose(fp);
	// close(sock);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);

}