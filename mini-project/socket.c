#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "socket.h"

int writeSocketInfo(int sock, SocketInfo* socket_info)
{
    int str_len = write(sock, socket_info, sizeof(SocketInfo));

    return str_len;
}

int readSocketInfo(int sock, SocketInfo* socket_info)
{
    int buffer;
    char temp[sizeof(SocketInfo)] = {0};

    int str_len = read(sock, socket_info, sizeof(SocketInfo));
    while (str_len < sizeof(SocketInfo)) {
        buffer = read(sock, temp, sizeof(SocketInfo) - str_len);
        str_len += buffer;
    }

    return str_len;
}

int recvStr(int sock, char* msg, int size)
{
	int str_len, buffer;
    char * temp = malloc(size);
	
	str_len = read(sock, msg, size);

	if (str_len == -1) 
		return -1;

	while (str_len < size) {
		str_len += read(sock, &msg[str_len], size - str_len);
	}

    return str_len;
}

/* write segment info to socket */
int writeSegmentInfo(int sock, Segment* seg_info)
{
    int str_len = write(sock, seg_info, sizeof(Segment));
    str_len += write(sock, seg_info->content, seg_info->size);

    return str_len;
}

/* read segment info from socket */
int readSegmentInfo(int sock, Segment* seg_info, char * content, int seg_size)
{
    int buffer;
    char* temp = malloc(seg_size);

    int str_len = read(sock, seg_info, sizeof(Segment));
    while (str_len < sizeof(Segment)) {
        buffer = read(sock, temp, sizeof(Segment) - str_len);
        str_len += buffer;
    }

    int content_len = recvStr(sock, content, seg_info->size);
    content[content_len] = 0;

    return str_len;
}