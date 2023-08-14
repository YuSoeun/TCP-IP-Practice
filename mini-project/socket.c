#include <stdio.h>
#include <unistd.h>
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

void* recvStr(int sock, char* msg, int size)
{
	int str_len, buffer;
    char temp[BUF_SIZE] = {0};
	
	str_len = read(sock, msg, BUF_SIZE);

	if (str_len == -1) 
		return (void*) -1;

	while (str_len < BUF_SIZE) {
		buffer = read(sock, temp, BUF_SIZE - str_len);
		str_len += buffer;
	}
}