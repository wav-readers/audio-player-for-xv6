#ifndef USER_AUDIO_H
#define USER_AUDIO_H

#include "kernel/types.h"

struct AudioInfo {
  ushort num_channels;
  uint sample_rate;
  uint byte_rate;  //通道数×每秒样本数×每样本的数据位数／8。可据此估计缓冲区大小
  ushort bits_per_sample;  //每样本的数据位数。各声道中的样本大小相等
  uint data_chunk_size;
  int num_frame;
};

#endif