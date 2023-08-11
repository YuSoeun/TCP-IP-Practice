#ifndef __CLIENT_SERVER__
#define __CLIENT_SERVER__
int server(int listen_port, int receiver_num, char* filename, int seg_size);
int client(int listen_port, char* ip, int port);
void error_handling(char *msg);

#endif