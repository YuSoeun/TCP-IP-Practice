#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <error.h>
#include <string.h>
#include "file_info.h"

/* return file size */
int filesize(const char *filename)
{
    struct stat file_info;
    int sz_file;

    if (0 > stat(filename, &file_info)){
	    return -1; // file이 없거나 에러
    }
    return file_info.st_size;
}

/* search file in directory */
FileInfoPacket* search_file_in_directory()
{
    DIR *dir;
    FileInfoPacket* files;
    FileInfoPacket file;
    struct dirent* directory;
    int dirSize, i = 0;
    void * tmp;
    dir = opendir ("./");

    directory = (struct dirent*)malloc((int)sizeof(struct dirent) * 100);
    files = (FileInfoPacket*)malloc((int)sizeof(FileInfoPacket) * 100);

    if (dir != NULL) {
        /* print all the files and directories within directory */
        while ((directory = readdir(dir)) != NULL) {
            printf ("%s\n", directory->d_name);
            strcpy(files[i].name, directory->d_name);
            files[i].size = filesize(directory->d_name);

            i++;
        }
        closedir (dir);
    } else {
         /* could not open directory */
         perror ("Error: could not open directory");
         exit(EXIT_FAILURE);
    }

    return 0;
}