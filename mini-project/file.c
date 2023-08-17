#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

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
            memcpy(segment[i]->content, content, fsize);
            segment[i]->size = fsize;

            i++;
		}
	}

    return i;
}