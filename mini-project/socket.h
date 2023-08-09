#ifndef __SOCKET_H__
#define __SOCKET_H__

#define LEN 100

typedef struct socketInfo
{
    int id;
    char ip[LEN];
    int port;
} SocketInfo;

#endif              //__SOCKET_H__