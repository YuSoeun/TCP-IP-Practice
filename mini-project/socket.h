#ifndef __SOCKET_H__
#define __SOCKET_H__

#define BUF_SIZE 1024

typedef struct socketInfo
{
    int id;
    char ip[BUF_SIZE];
    int port;
} SocketInfo;

int writeSocketInfo(int sock, SocketInfo* socket_info);
int readSocketInfo(int sock, SocketInfo* socket_info);
void* recvStr(int sock, char* msg, int size);

#endif              //__SOCKET_H__