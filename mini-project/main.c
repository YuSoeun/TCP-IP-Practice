/*
    file sharing을 위한 P2P File Sharing 구현
    - Sending Peer가 가진 File을 나머지 N개의 Receiving Peer들과 공유
    - file은 여러 Segment(동일한 크기)로 나눠 단 하나의 Receving Peer(RR로 선정)에만 전송
    - multi thread로 구현

    sender: ./p2p -s -n (최대 receving peer) -f (filename) -g (segment 크기 KB)
    receiver: ./p2p -r -a 192.168.10.2 9090

    1. Sending peer가 접속한 peer들의 ID, ip, port 정보 분배
    2. Receiver끼리 각각 연결
    3. Peer 간 연결 완료 시 Sender에게 msg 전송
    4. Sender가 모든 Receiver에게 총 seg 수, 파일 이름 보내기
    5. Receiver가 seg를 받으면 다른 peer에게 전송
       + 동시에 다른 peer에게 seg 받기
    6. seg 순서대로 조합
    7. 다 받았으면 알리거나 client 종료
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

void error_handling(char *msg)
{
	printf("%s\n", msg);
	exit(1);
}