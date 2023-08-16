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
int SaveFile2Seg(char filename[NAME_LEN], Segment** segment, int seg_size);
char** split(char* str, const char* delimiter, int* count);

int writeSegmentInfo(int sock, Segment* socket_info);
int readSegmentInfo(int sock, Segment* socket_info, char * content, int seg_size);

#endif