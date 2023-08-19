#include <stdio.h>
#include <stdlib.h>
#include "progress.h"

void setSendInfo(SendInfo * info, int file_size, int size, float time)
{
    info->file_size = file_size;
    info->cur_size = size;
    info->time_spent = time;
}

void updateSendInfo(SendInfo * info, int size, double time)
{
    info->cur_size += size;
    info->time_spent = time;
}

void getSendInfo(SendInfo * info, int* file_size, int* size, double* time)
{
    *file_size = info->file_size;
    *size = info->cur_size;
    *time = info->time_spent;
}

void setRecvInfo(RecvInfo * info, int total_seg, int seg_num, int size, double time)
{
    info->total_seg = total_seg;
    info->seg_num = seg_num;
    info->cur_size = size;
    info->time_spent = time;
}

void updateRecvInfo(RecvInfo * info, int size, double time)
{
    info->seg_num++;
    info->cur_size += size;
    info->time_spent = time;
}

void getRecvInfo(RecvInfo * info, int* total_seg, int* seg_num, int* size, double* time)
{
    *total_seg = info->total_seg;
    *seg_num = info->seg_num;
    *size = info->cur_size;
    *time = info->time_spent;
}

void printBar(int bar_width, double percent)
{
    int char_num = bar_width * percent;
    
		for (int i = 0; i < bar_width; i++) {
			if (i < char_num)	printf("#");
			else			printf(" ");
		}
}