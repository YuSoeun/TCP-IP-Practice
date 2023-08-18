#ifndef __PROGRESS_H__
#define __PROGRESS_H__

typedef struct sendInfo
{
	int file_size;			// 보내는 file size
	int total_seg;		    // 한 전체 segment 개수
	int seg_size;		    // 한 segment의 size
	int snd_seg_num;		// 보낸 segment 개수
	double time_spent;	    // 소요된 시간
} SendInfo;

void setSendInfo(SendInfo * info, int file_size, int total_seg, int seg_size, int seg_num, float time);
void updateSendInfo(SendInfo * info, int seg_num, double time);
void getSendInfo(SendInfo * info, int* file_size, int* total_seg, int* seg_size, int* seg_num, double* time);

#endif