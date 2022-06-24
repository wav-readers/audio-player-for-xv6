#include "wav.h"

#define HEAD_LENGTH 44

int readWavHead(int fd, struct WavInfo *info) {
    char buf[HEAD_LENGTH];
    int n;
    if (fd < 0)
    {
        return -1;
    }
    if ((n = read(fd, buf, HEAD_LENGTH)) > 0)
    {
        int pos = 0;
        // 寻找“RIFF”标记
        while (pos < HEAD_LENGTH)
        {
            if (buf[pos] == 'R' && buf[pos + 1] == 'I' && buf[pos + 2] == 'F' && buf[pos + 3] == 'F')
            {
                info->chunk_id[0] = 'R';
                info->chunk_id[1] = 'I';
                info->chunk_id[2] = 'F';
                info->chunk_id[3] = 'F';
                pos += 4;
                break;
            }
            ++pos;
        }
        info->chunk_size = *(int *)&buf[pos];
        pos += 4;
        info->format[0] = buf[pos];
        info->format[1] = buf[pos + 1];
        info->format[2] = buf[pos + 2];
        info->format[3] = buf[pos + 3];
        pos += 4;

        //寻找“fmt”标记
        while (pos < HEAD_LENGTH)
        {
            if (buf[pos] == 'f' && buf[pos + 1] == 'm' && buf[pos + 2] == 't')
            {
                info->fmt_chunk_id[0] = 'f';
                info->fmt_chunk_id[1] = 'm';
                info->fmt_chunk_id[2] = 't';
                pos += 4;
                break;
            }
            ++pos;
        }

        //读取Format Chunk部分
        info->fmt_chunk_size = *(int *)&buf[pos];
        pos += 4;
        info->audio_fomat = *(short *)&buf[pos];
        pos += 2;
        info->num_channels = *(short *)&buf[pos];
        pos += 2;
        info->sample_rate = *(int *)&buf[pos];
        pos += 4;
        info->byte_rate = *(int *)&buf[pos];
        pos += 4;
        info->block_align = *(short *)&buf[pos];
        pos += 2;
        info->bits_per_sample = *(short *)&buf[pos];
        pos += 2;

        //寻找“data”标记
        while (pos < HEAD_LENGTH)
        {
            if (buf[pos] == 'd' && buf[pos + 1] == 'a' && buf[pos + 2] == 't' && buf[pos + 3] == 'a')
            {
                info->data_chunk_id[0] = 'd';
                info->data_chunk_id[1] = 'a';
                info->data_chunk_id[2] = 't';
                info->data_chunk_id[3] = 'a';
                pos += 4;
                break;
            }
            ++pos;
        }

        //读取Data Chunk的非data部分
        info->data_chunk_size = *(int *)&buf[pos];
        pos += 4;

        //记录真正音频数据的开始位置
        info->start_pos = pos;

        //计算文件总帧数
        info->num_frame = info->data_chunk_size / (info->num_channels * (info->bits_per_sample / 8));
    }
    else
    {
        return -1;
    }
    if (info->start_pos != HEAD_LENGTH)
    {
        return -1;
    }
    return 0;
}