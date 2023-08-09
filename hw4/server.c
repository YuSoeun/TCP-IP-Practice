/*
    자동 완성 기능 기반 Search Engine 구현
    1. 서버의 Listen 포트 번호와 검색어 데이터베이스 파일을 command line argument 로 넣어 프로그램을 실행
       - 서버는 Multi-Thread로 구현
     $ ./search_server 9090 data.txt
    2. 파일로부터 검색어와 검색어의 검색 횟수를 읽어 온다. Pohang 30000
    3. Client에서 검색어 입력
       - 연관 검색어 검색 횟수에 대해 정렬하여 실시간으로 표시 (Trie DS 사용, 최대 10개)
       - TCP nodelay option 줘보기
       - 검색어가 중간에 나오거나 끝에 나오는 경우 고려 (kmp algorithim)
    4. 검색어에 해당되는 부분은 임의의 색깔

	split: https://blog.naver.com/PostView.nhn?blogId=sooftware&logNo=221999680764
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <ctype.h>

#include "Console.h"
#include "trie.h"

#define MAX_CLNT 256
#define NAME_LEN 256
#define BUF_SIZE 1024

int clnt_cnt = 0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
Trie *trie;

void * handle_clnt(void * arg);
void send_msg(Result ** result, int count, char msg[BUF_SIZE], int clnt_sock);
void error_handling(char * msg);
char** split(char* str, const char* delimiter, int* count);
void openFileAndSaveTrie(char filename[NAME_LEN], Trie* trie);
void bubbleSort(Result** arr, int count);

int main(int argc, char *argv[])
{
    // Multi-Thread로 서버 구현 (TCP nodelay socket option 주기)
    int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	char ** result;

	if(argc != 3) {
		printf("Usage : %s <port> <file name>\n", argv[0]);
		exit(1);
	}
  
	pthread_mutex_init(&mutx, NULL);
	serv_sock=socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");
	if(listen(serv_sock, 5)==-1)
		error_handling("listen() error");

	trie = getNewTrie();
	openFileAndSaveTrie(argv[2], trie);

	while(1)
	{
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);

    return 0;
}

void * handle_clnt(void * arg)
{
	int clnt_sock = *((int*)arg);
	int str_len=0, i;
	char msg[BUF_SIZE] = {};
	Result ** result;
	
	while ((str_len = read(clnt_sock, msg, sizeof(msg))) != 0) {
		msg[str_len] = 0;
        
		// to lower msg
		for (int i = 0; i < str_len; i++) {
			msg[i] = tolower(msg[i]);
		}

		printf("msg: '%s'\n", msg);
		
		result = getStringsContainChar(trie, msg);

		for (int i = 0; i < trie->rslt_cnt; i++) {
			printf("data: %s, ", result[i]->word);
			printf("cnt: %d\n", result[i]->cnt);
		}

		bubbleSort(result, trie->rslt_cnt);

		// TODO: 상위 10개 보내기
		send_msg(result, trie->rslt_cnt, msg, clnt_sock);
    }
	
	// remove disconnected client
	pthread_mutex_lock(&mutx);
	for (i = 0; i < clnt_cnt; i++) {
		if (clnt_sock == clnt_socks[i]) {
			while(i++ < clnt_cnt-1)
				clnt_socks[i] = clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_msg(Result ** result, int count, char msg[BUF_SIZE], int clnt_sock)   // send to all
{
	int send_len;
	char line[BUF_SIZE];

	send_len = write(clnt_sock, &count, sizeof(int));
	send_len = write(clnt_sock, msg, BUF_SIZE);
	printf("count: %d\n", count);
	if (count > 10) {
		count = 10;
	}
	for (int i = 0; i < count; i++) {
		sprintf(line, "%d. %-30s	(%d)", i+1, result[i]->word, result[i]->cnt);
		printf("line: %s\n", line);
		send_len = write(clnt_sock, line, BUF_SIZE);
		// printf("send_len: %d\n", send_len);
	}
}

void bubbleSort(Result** arr, int count)
{
    Result* temp;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (arr[j]->cnt < arr[j+1]->cnt) {    // swap
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            }
        }
    }
}

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void openFileAndSaveTrie(char filename[NAME_LEN], Trie* trie)
{
	char **split_line;
	FILE * fp;
	char line[BUF_SIZE];
	char data[BUF_SIZE];
	int count;

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
		printf("file content is\n");
		printf("---------------\n");
		
		while (feof(fp) == 0) {
			count = 0;
			fgets(line, BUF_SIZE, fp);
			printf("%s", line);

			split_line = split(line, " ", &count);
			strcpy(data, split_line[0]);
			for (int i = 1; i < count-1; i++) {
				strcat(data, " ");
				strcat(data, split_line[i]);
			}
			strncat(data, "\0", 1);

			for (int i = 0; i < strlen(data); i++) {
				data[i] = tolower(data[i]);
			}
			
			insert(trie, data, atoi(split_line[count-1]));
		}
		printf("\n\n");
	}
}

char** split(char* str, const char* delimiter, int* count) {
    int i, j, len;
    char* token;
    char** result = NULL;

    // 구분자로 문자열을 분리한 후, 문자열 개수(count)를 구합니다.
    token = strtok(str, delimiter);
    while (token != NULL) {
        (*count)++;
        result = (char**)realloc(result, (*count) * sizeof(char*));
        result[(*count) - 1] = token;
        token = strtok(NULL, delimiter);
    }

    // 문자열 개수(count)만큼의 문자열 배열을 동적으로 할당합니다.
    result = (char**)realloc(result, (*count) * sizeof(char*));

    return result;
}