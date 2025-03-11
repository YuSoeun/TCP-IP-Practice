#ifndef __SOCKET_H__
#define __SOCKET_H__

#define BUF_SIZE 1024

typedef struct socketInfo
{
    int id;
    char ip[BUF_SIZE];
    int listen_port;
} SocketInfo;

typedef struct segment
{
    int seq;            // segment sequence
    char * content;
    int size;
} Segment;

int writeSocketInfo(int sock, SocketInfo* socket_info);
int readSocketInfo(int sock, SocketInfo* socket_info);
int recvStr(int sock, char* msg, int size);

int writeSegmentInfo(int sock, Segment* socket_info);
int readSegmentInfo(int sock, Segment* socket_info, char * content, int seg_size);

#endif              //__SOCKET_H__