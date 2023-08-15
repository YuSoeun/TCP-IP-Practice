#ifndef __FILE_H__
#define __FILE_H__

#define NAME_LEN 100

typedef struct segment
{
    int seq;        // segment sequence
    char * content;
    int size;
} Segment;

int filesize(const char *filename);
void Savefile2Seg(char filename[NAME_LEN], Segment** segment, int seg_size);
char** split(char* str, const char* delimiter, int* count);

#endif