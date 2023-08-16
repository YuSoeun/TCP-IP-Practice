#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include "file.h"
#include "socket.h"

/* return file size*/
int filesize(const char *filename)
{
    struct stat file_info;
    int sz_file;

    if (0 > stat(filename, &file_info)){
	    return -1; // file이 없거나 에러
    }
    return file_info.st_size;
}

/* open file and save in trie structure */
int SaveFile2Seg(char filename[NAME_LEN], Segment** segment, int seg_size)
{
	FILE * fp;
	char * content;
	int fsize, i = 0;

    content = (char *)malloc(seg_size);

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
        // segment_size보다 적게 읽으면 더 읽기
		while (feof(fp) == 0) {
			fsize = fread(content, 1, seg_size, fp);
            while (fsize < seg_size && feof(fp) == 0) {
                fsize += fread(&content[fsize], 1, seg_size, fp);
            }
            content[fsize] = 0;

            segment[i]->content = (char *)malloc(seg_size);
            segment[i]->seq = i;
            memcpy(segment[i]->content, content, seg_size);
            segment[i]->size = fsize;

            i++;
		}
	}

    return i;
}

/* split string with delimiter */
char** split(char* str, const char* delimiter, int* count) {
    int i, j, len;
    char* token;
    char** result = NULL;

    // 구분자로 문자열을 분리 후, 문자열 개수(count)를 구하기.
    token = strtok(str, delimiter);
    while (token != NULL) {
        (*count)++;
        result = (char**)realloc(result, (*count) * sizeof(char*));
        result[(*count) - 1] = token;
        token = strtok(NULL, delimiter);
    }

    return result;
}

/* write segment info to socket */
int writeSegmentInfo(int sock, Segment* seg_info)
{
    int str_len = write(sock, seg_info, sizeof(Segment));
    str_len += write(sock, seg_info->content, seg_info->size);

    return str_len;
}

/* read segment info from socket */
int readSegmentInfo(int sock, Segment* seg_info, char * content, int seg_size)
{
    int buffer;
    char temp[seg_size];

    int str_len = read(sock, seg_info, sizeof(Segment));
    while (str_len < sizeof(Segment)) {
        buffer = read(sock, temp, sizeof(Segment) - str_len);
        str_len += buffer;
        // printf("temp: %s\n", temp);
    }

    int content_len = recvStr(sock, content, seg_info->size);
    content[content_len] = 0;

    return str_len;
}