#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <sys/stat.h>
#include "file.h"

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
void Savefile2Seg(char filename[NAME_LEN], Segment** segment, int seg_size)
{
	char **split_line;
	FILE * fp;
	char * content;
	int fsize, i = 0;

    content = (char *)malloc(seg_size);

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("Failed to open file.\n");
	} else {
		printf("file content is\n");
		printf("---------------\n");
		
        // segment 다 읽고 저장
		while (feof(fp) == 0) {
			fsize = fread(content, 1, seg_size, fp);
            content[fsize] = 0;
			printf("[%d] %s\n", i, content);

            segment[i]->content = (char *)malloc(seg_size);

            segment[i]->seq = i;
            memcpy(segment[i]->content, content, seg_size);
            segment[i]->size = fsize;

			// printf("[%d] %s(%d)\n", segment[i]->seq, segment[i]->content), segment[i]->size;
            i++;
		}
	}
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