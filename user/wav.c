#include "wav.h"

#define HEAD_LENGTH 44
#define BUF_LENGTH 400
#define GET_DATA_BATCH_SIZE 320
// 256KB

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

/// @todo
void decodeWav(const char *fileData, char *decodedData) {
  return;
}

void getData(char *fname, struct WavInfo *info)
{
    //记录文件读取位置
    int pos = info->start_pos;

    //为加快处理速度，根据ChunkSize将文件一次读入内存
    int fd = open(fname, O_RDONLY);
    if (fd < 0)
    {
        fprintf(2, "getData: cannot open %s\n", fname);
        exit(1);
    }

    fprintf(2, "info->chunk_size %d\n",info->chunk_size);

    short getdata_batch = GET_DATA_BATCH_SIZE;

    char file_data[getdata_batch + 8];

    int size = read(fd, file_data, getdata_batch + 8);

    close(fd);

    if (size != getdata_batch + 8)
    {
        fprintf(2, "getData: read error\n");
        exit(1);
    }

    fprintf(2, "getData: pos at %d,start pos at %d\n", pos,info->start_pos);
    fprintf(2, "getData: read %d bytes, data chunk size %d\n", size,info->data_chunk_size);
    printf("getData:bits_per_sample %d\n",info->bits_per_sample);
    
    //TODO 这里要改，这一轮有多少要读。最后一轮不需要读这么多
    short read_size = getdata_batch;
    //以每帧2字节为例

    if (info->num_channels == 1){
        while (pos < (info->start_pos + read_size))
        {   
            printf("\nboth channels:");
            ushort i = 0;           
            while(i < info->bits_per_sample){
                printf("%d ",*(short *)&file_data[pos]);
                pos++;
                i++;
            }
        }
    }
    else if (info->num_channels == 2){
        while (pos < info->start_pos + read_size)
        {
            printf("\nleft channel:");           
            ushort i = 0;           
            while(i < info->bits_per_sample){
                printf("%d ",*(short *)&file_data[pos]);
                pos++;
                i++;
            }
            
            printf("\nright channel:");   
            i = 0;           
            while(i < info->bits_per_sample){
                printf("%d ",*(short *)&file_data[pos]);
                pos++;
                i++;
            }
        }
    }
    else{
        fprintf(2, "error in num_channels\n");        
    }

    printf("\n ");
}