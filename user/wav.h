#ifndef USER_WAV_H
#define USER_WAV_H

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

struct WavInfo
{
    char chunk_id[4];
    uint chunk_size;
    char format[4];
    char fmt_chunk_id[4];
    uint fmt_chunk_size;
    ushort audio_fomat;
    ushort num_channels; //对应数字1，表示声道数为1，是个单声道Wav
    uint sample_rate;
    uint byte_rate; //通道数×每秒样本数×每样本的数据位数／8（1*22050*16/8）。播放软件利用此值可以估计缓冲区的大小
    ushort block_align;
    ushort bits_per_sample;//每样本的数据位数，表示每个声道中各个样本的数据位数。如果有多个声道，对每个声道而言，样本大小都一样
    char data_chunk_id[4];
    uint data_chunk_size;
    int num_frame;
    int start_pos;
};

/**
 * @pre fd的偏移量为0
 * @brief 读取wav文件的文件头，将相关信息存储到info.
 * 若检测到编码错误，则立即返回-1
 * @return 0 if 正确执行，-1 if 该wav文件有问题
 */
int readWavHead(int fd, struct WavInfo *info);
// 从fileData中读取数据，解码后写入decodedData
// question: 每次读取多少数据来解码？每次解码会产生多少解码后的数据？此数据用于指导缓冲区大小
// question: 【是否可能直接将解码后的数据写入声卡缓冲区，从而取消“解码后数据”的缓冲区？】
void decodeWav(const char *fileData, char *decodedData);

void getData(char *fname, struct WavInfo *info); 

#endif