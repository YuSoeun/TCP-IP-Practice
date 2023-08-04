#ifndef __FILE_INFO__
#define __FILE_INFO__
#define MAX_BUF 10  // set buffer size 10 to test

typedef struct packet
{
    int seq;                // index
    int data_size;          // data size
    char data[MAX_BUF];     // data
} packet;

#endif