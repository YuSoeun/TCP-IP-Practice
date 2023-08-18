#ifndef __PROGRESS_H__
#define __PROGRESS_H__

#define BAR_SIZE 1024

typedef struct sendInfo
{
	int file_size;			// 보내거나 받을 전체 file size
	int cur_size;			// 현재 보내거나 받은 파일 사이즈
	double time_spent;	    // 소요된 시간
} SendInfo;

typedef struct recvInfo
{
	int total_seg;     		// 특정 receiver가 보내거나 받아야 할 총 segment 수
	int seg_num;		    // 보낸 segment 개수
	int cur_size;			// 현재 보내거나 받은 파일 사이즈
	double time_spent;	    // 소요된 시간
} RecvInfo;

void setSendInfo(SendInfo * info, int file_size, int size, float time);
void updateSendInfo(SendInfo * info, int size, double time);
void getSendInfo(SendInfo * info, int* file_size, int* size, double* time);

void setRecvInfo(RecvInfo * info, int total_seg, int seg_num, int size, double time);
void updateRecvInfo(RecvInfo * info, int size, double time);
void getRecvInfo(RecvInfo * info, int* total_seg, int* seg_num, int* size, double* time);

void printBar(int bar_width, double percent);

#endif