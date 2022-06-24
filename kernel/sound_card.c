#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pci.h"

#define BUFFER_NODE_NUM 4

static struct sound_node sound_buffer[BUFFER_NODE_NUM];
int buffer_index, used_size;

// void setSampleRate(uint sample_rate);
uint64 sys_setSampleRate()
{
  int rate; argint(0, &rate);
  if (rate < 0) return -1;
  // clear buffer
  buffer_index = 0; used_size = 0;
  for (int i = 0; i < BUFFER_NODE_NUM; i++) {
    memset(&sound_buffer[i], 0, sizeof(struct sound_node));
    sound_buffer[i].flag = 0;
  }
  set_sample_rate(rate);
  return 0;
}

// TODO
// int getVolume(int maxVolume);
uint64 sys_getVolume(void)
{
  uint16 volume = get_volume();
  volume = volume + 1;

  int maxVolume; argint(0, &maxVolume);
  return 0;
}

// void setVolume(int volume, int maxVolume);
uint64 sys_setVolume(void)
{
  return 0;
}

// void setPlay(int play);
uint64 sys_setPlay(void)
{
  return 0;
}

// void writeDecodedAudio(char *decodedData, uint size);
uint64 sys_writeDecodedAudio(void)
{
  printf("in sys write decode\n");
  int bufsize = DMA_BUFFER_NUM * DMA_BUFFER_SIZE, size;

  char buf[2049];
  if (argint(1, &size) < 0 || size > 2048) return -1;
  
  uint64 buf_addr;
  if (argaddr(0, &buf_addr) < 0)
    return -1;
  // copy data in virtual address to physical address
  struct proc *p = myproc();
  if (copyin(p->pagetable, buf, buf_addr, size) < 0)
    return -1;
  
  if (used_size == 0) {
    // this is a new buffer
    memset(&sound_buffer[buffer_index], 0, sizeof(struct sound_node));
  }
  if (bufsize - used_size > size) {
    // data can all put into current buffer.
    memmove(&sound_buffer[buffer_index].data[used_size], buf, size);
    sound_buffer[buffer_index].flag = 1;
    used_size += size;
  } else {
    // data cannot all put into current buffer.
    int remain = bufsize - used_size;
    // then send this buffer to the audio card.
    memmove(&sound_buffer[buffer_index].data[used_size], buf, remain);
    sound_buffer[buffer_index].flag = 3; // has been sent
    add_sound_node(&sound_buffer[buffer_index]);
    // and find a new buffer that is not sent, and make it to be the new buffer.
    while (1) {
      int flag = 0;
      for (int i = 0; i < BUFFER_NODE_NUM; ++i) {
        if (sound_buffer[i].flag != 3) { // has not been sent
          memset(&sound_buffer[i], 0, sizeof(struct sound_node));
          if (bufsize > size - remain) {
            // all remaining data can put into one buffer
            memmove(&sound_buffer[i].data[0], buf + remain, size - remain);
            sound_buffer[i].flag = 1; flag = 1;
            used_size = size - remain; buffer_index = i;
            break;
          } else {
            // send this new buffer
            memmove(&sound_buffer[i].data[0], buf + remain, bufsize);
            remain += bufsize; sound_buffer[i].flag = 3; // has been sent
            add_sound_node(&sound_buffer[i]);
          }
        }
      }
      if (flag) break;
    }
  }
  return 0;
}

// void clearSoundCardBuffer();
uint64 sys_clearSoundCardBuffer(void)
{
  return 0;
}

