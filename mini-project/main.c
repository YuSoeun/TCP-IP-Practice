/*
    sender: ./p2p -s -n (최대 receving peer) -f (filename) -g (segment 크기 KB) -p (listening port)
        ex) ./p2p -s -n 2 -f data.txt -g 1 -p 7777
    receiver: ./p2p -r -a 192.168.10.2 9090 -p (listening port)
        ex) ./p2p -r -p 4000 -a 203.252.112.31 7777

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "client_server.h"

#define BUF_SIZE 1024
void help();

int main(int argc, char *argv[])
{
    int opt;
    int opt_client = 0, opt_server = 0;
    int num, seg_size, port, listen_port;
    char *filename, *ip;
    
    printf("argc: %d\n", argc);
    while((opt = getopt(argc, argv, "hsn:f:g:ra:p:")) != -1) {
        switch(opt) {
            case 'h':
                help();
                break;
            case 's':
                opt_server++;
                break;
            case 'n':
                num = atoi(optarg);
                opt_server++;
                break;  
            case 'f':
                filename = optarg;
                opt_server++;
                break;
            case 'g':
                seg_size = atoi(optarg);
                opt_server++;
                break;
            case 'r':
                opt_client++;
                break;
            case 'a':
                ip = optarg;
                opt_client++;
                break;
            case 'p':
                listen_port = atoi(optarg);
                opt_client++;
                opt_server++;
                break;
        }
    }
    
    seg_size = seg_size * 1000;

    if (opt_server >= 5) {
        printf("server - num: %d, filename: %s, seg_size: %d\n", num, filename, seg_size);
        printf("listen port: %d\n",  listen_port);
        server(listen_port, num, filename, seg_size);
    } else if (opt_client >= 3) {
        port = atoi(argv[optind]);
        printf("client - ip: %s, port: %d\n", ip, port);
        printf("listen port: %d\n",  listen_port);
        client(listen_port, ip, port);
    } else {  
        help();
    }

    return 0;
}

void help() 
{ 
    printf("Usage: ./p2p -s -n [NUMBER] -f [FILE] -g [SIZE] -p [LISTEN_PORT]\n"
           "  -h                help\n" 
           "  -s                Sending Peer\n" 
           "  -n [NUMBER]       최대 receving peer\n" 
           "  -f [FILE]         읽을 파일 (ex. video.mp4, video_long.mp4)\n"
           "  -g [SIZE]         Segment Size(KB\n"
           "  -p [LISTEN_PORT]  listening할 port\n"
           "\n");
    printf("Usage: ./p2p -r -a [IP] [PORT] -p [LISTEN_PORT]\n" 
           "  -h                help\n" 
           "  -r                Receiving Peer\n" 
           "  -a [IP] [PORT]    Sending Peer의 IP와 port 정보\n"
           "  -p [LISTEN_PORT]  listening할 port\n"); 
    exit(0); 
}

void error_handling(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}