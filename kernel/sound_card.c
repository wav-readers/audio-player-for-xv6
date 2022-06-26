#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pci.h"

#define BUFFER_NODE_NUM 3
#define MAX_VOLUME 0b00000
#define MIN_VOLUME 0b11111
#define MUTE_VOLUME 0x8000

static struct sound_node sound_buffer[BUFFER_NODE_NUM];
int buffer_index = 0, used_size = 0;

// void setSampleRate(uint sample_rate);
uint64 sys_setSampleRate()
{
  int rate; argint(0, &rate);
  if (rate < 0) return -1;
  set_sample_rate(rate);
  return 0;
}

// int getVolume(int maxVolume);
uint64 sys_getVolume(void)
{
  uint16 volume = get_volume();

  int maxVolume; argint(0, &maxVolume);
  if (volume >= MUTE_VOLUME) return 0;
  else {
    volume = volume & 0b11111;
    volume = (MIN_VOLUME - volume) * maxVolume / (MIN_VOLUME - MAX_VOLUME);
    return volume;
  }
}

// void setVolume(int volume, int maxVolume);
uint64 sys_setVolume(void)
{
  int volume, maxVolume;
  argint(0, &volume); argint(1, &maxVolume);
  if (volume == 0) set_volume(MUTE_VOLUME);
  else {
    volume = MIN_VOLUME - volume * (MIN_VOLUME - MAX_VOLUME) / maxVolume;
    volume = volume + (volume << 8);
    set_volume(volume);
  }
  
  return 0;
}

// void setPlay(int play);
uint64 sys_setPlay(void)
{
  int play; argint(0, &play);
  if (play) return resume();
  else return pause();
}

// void writeDecodedAudio(char *decodedData, uint size);
uint64 sys_writeDecodedAudio(void)
{
  int bufsize = DMA_BUFFER_NUM * DMA_BUFFER_SIZE, size;

  char buf[2049];
  if (argint(1, &size) < 0 || size > 2048) return -1;

  uint64 buf_addr;
  if (argaddr(0, &buf_addr) < 0) return -1;
  // copy data in user space to kernel space
  struct proc *p = myproc();
  if (copyin(p->pagetable, buf, buf_addr, size) < 0) return -1;  
  
  // clear the new buffer
  if (used_size == 0) memset(&sound_buffer[buffer_index], 0, sizeof(struct sound_node));

  // if data can all fit in the current buffer
  if (bufsize - used_size > size) {
    memmove(&sound_buffer[buffer_index].data[used_size], buf, size);
    sound_buffer[buffer_index].flag = 1;
    used_size += size;
  } 
  else { 
    // fill the current buffer...
    int read_already = bufsize - used_size;
    memmove(&sound_buffer[buffer_index].data[used_size], buf, read_already);
    sound_buffer[buffer_index].flag = 3; // has been sent
    add_sound_node(&sound_buffer[buffer_index]);
    //  and repeatedly look for new buffer
    while (1) {
      int flag = 0;
      for (int i = 0; i < BUFFER_NODE_NUM; ++i) {
        if (sound_buffer[i].flag == 0) { // buffer can be overwritten
          memset(&sound_buffer[i], 0, sizeof(struct sound_node));
          // all left data can fit into the new buffer
          if (bufsize > size - read_already) {
            memmove(&sound_buffer[i].data[0], buf + read_already, size - read_already);
            sound_buffer[i].flag = 1;
            used_size = size - read_already;
            flag = 1; buffer_index = i;
            break;
          }
          else { // fill this new buffer and send it
            memmove(&sound_buffer[i].data[0], buf + read_already, bufsize);
            sound_buffer[i].flag = 3;
            add_sound_node(&sound_buffer[i]);
            read_already += bufsize;
          }
        }
      }
      if (flag) break;
    }
  }
  return 0;
}

uint64 sys_finishWriteAudio(void) {
  // send the last buffer
  if (used_size > 0) { 
    sound_buffer[buffer_index].flag = 3; add_sound_node(&sound_buffer[buffer_index]); 
  }
  return 0;
}

// void clearSoundCardBuffer();
uint64 sys_clearSoundCardBuffer(void)
{
  buffer_index = 0; used_size = 0;
  for (int i = 0; i < BUFFER_NODE_NUM; i++) {
    memset(&sound_buffer[i], 0, sizeof(struct sound_node));
    sound_buffer[i].flag = 0;
  }
  clear_sound_queue();
  return 0;
}

