#ifndef __CLIENT_SERVER__
#define __CLIENT_SERVER__
int server(int receiver_num, char* filename, int seg_size);
int client(char* ip, int port);
void error_handling(char *msg);

#endif