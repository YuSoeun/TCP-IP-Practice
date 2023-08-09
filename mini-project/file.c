#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "file.h"

/* open file and save in trie structure */
// void openFileAndSaveTrie(char filename[WORD_SIZE], Trie* trie)
// {
// 	char **split_line;
// 	FILE * fp;
// 	char line[BUF_SIZE];
// 	char data[BUF_SIZE];
// 	int count;

// 	if ((fp = fopen(filename, "rb")) == NULL) {
// 		printf("Failed to open file.\n");
// 	} else {
// 		printf("file content is\n");
// 		printf("---------------\n");
		
// 		while (feof(fp) == 0) {
// 			count = 0;
// 			fgets(line, BUF_SIZE, fp);
// 			printf("%s", line);

// 			split_line = split(line, " ", &count);
// 			strcpy(data, split_line[0]);
// 			for (int i = 1; i < count-1; i++) {
// 				strcat(data, " ");
// 				strcat(data, split_line[i]);
// 			}
// 			strncat(data, "\0", 1);

// 			for (int i = 0; i < strlen(data); i++) {
// 				data[i] = tolower(data[i]);
// 			}
			
// 			insert(trie, data, atoi(split_line[count-1]));
// 		}
// 		printf("\n\n");
// 	}
// }

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