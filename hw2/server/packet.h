#ifndef __FILE_INFO__
#define __FILE_INFO__
#define MAX_BUF 10  // set buffer size 10 to test

typedef struct packet
{
    int seq;                // index
    char data[MAX_BUF];     // data
} packet;

#endif