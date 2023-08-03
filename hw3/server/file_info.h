#ifndef __FILE_INFO__
#define __FILE_INFO__
#define NAME_LEN 100

typedef struct FileInfoPacket
{
    char name[NAME_LEN];
    int size;
    char type[10];
} FileInfoPacket;

// struct FileInfoPacket* search_file_in_directory();
int filesize(const char *filename);
#endif