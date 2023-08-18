#include <stdlib.h>
#include "progress.h"

void setSendInfo(SendInfo * info, int file_size, int total_seg, int seg_size, int seg_num, float time)
{
    info->file_size = file_size;
    info->total_seg = total_seg;
    info->seg_size = seg_size;
    info->snd_seg_num = seg_num;
    info->time_spent = time;
}

void updateSendInfo(SendInfo * info, int seg_num, double time)
{
    info->snd_seg_num = seg_num;
    info->time_spent += time;
}

void getSendInfo(SendInfo * info, int* file_size, int* total_seg, int* seg_size, int* seg_num, double* time)
{
    *file_size = info->file_size;
    *total_seg = info->total_seg;
    *seg_size = info->seg_size;
    *seg_num = info->snd_seg_num;
    *time = info->time_spent;
}