#ifndef USER_WAV_H
#define USER_WAV_H

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#include "user/audio.h"

struct WavInfo
{
    char chunk_id[4];
    uint chunk_size;
    char format[4];
    char fmt_chunk_id[4];
    uint fmt_chunk_size;
    ushort audio_fomat;    
    ushort block_align;
    char data_chunk_id[4];
    uint data_chunk_size;
    int start_pos;
};

/**
 * @pre fd的偏移量为0
 * @brief 读取wav文件的文件头，将音频信息存储到ainfo，编码细节信息存储到info.
 * 若检测到编码错误，则立即返回-1
 * @return 0 if 正确执行，-1 if 该wav文件有问题
 */
int readWavHead(int fd, struct AudioInfo *ainfo, struct WavInfo *info);

#endif