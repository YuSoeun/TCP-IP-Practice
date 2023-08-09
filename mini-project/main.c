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
    int num, seg_size, port;
    char *filename, *ip;
    
    printf("argc: %d\n", argc);
    while((opt = getopt(argc, argv, "hsn:f:g:ra:")) != -1) {
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
        }
    }

    if (opt_server >= 4) {
        printf("server - num: %d, filename: %s, seg_size: %d\n", num, filename, seg_size);
        server(num, filename, seg_size);
    } else if (opt_client >= 2) {
        port = atoi(argv[optind]);
        printf("client - ip: %s, port: %d\n", ip, port);
        client(ip, port);
    } else {  
        help();
    }

    return 0;
}

void help() 
{ 
    printf("Usage: ./p2p -s -n [NUMBER] -f [FILE] -g [SIZE]\n"
           "  -h                help\n" 
           "  -s                Sending Peer\n" 
           "  -n [NUMBER]       최대 receving peer\n" 
           "  -f [FILE]         읽을 파일\n"
           "  -g [SIZE]         Segment Size(KB)\n"
           "\n");
    printf("Usage: ./p2p -r -a [IP] [PORT]]\n" 
           "  -h                help\n" 
           "  -r                Receiving Peer\n" 
           "  -a [IP] [PORT]    Sending Peer의 IP와 port 정보\n"); 
    exit(0); 
}