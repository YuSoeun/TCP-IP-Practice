#ifndef __FILE_H__
#define __FILE_H__

#define NAME_LEN 100
#include "socket.h"


int filesize(const char *filename);
int SaveFile2Seg(char filename[NAME_LEN], Segment** segment, int seg_size);

#endif