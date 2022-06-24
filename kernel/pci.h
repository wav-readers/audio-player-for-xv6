#include "types.h"

#define DMA_BUFFER_NUM 32
#define DMA_SAMPLE_NUM 4096
#define DMA_BUFFER_SIZE (DMA_SAMPLE_NUM * 2)

#define PCI_CONFIG_SPACE_CMD 0x04
#define PCI_CONFIG_SPACE_INTLN 0x3C
#define PCI_CONFIG_NAMBA 0x10
#define PCI_CONFIG_NABMBA 0x14

struct descriptor {
  uint32 buffer_pointer;
  uint32 buffer_ctrl_and_len; // 31:IOC, 30:BUP, 29-16:Reserved, 15-0:Length
};

struct sound_node { // containing data for filling the whole descriptor table
  volatile uint8 flag; // 0: played, 1: containing data, 2: sent already
  struct sound_node *next;
  uint8 data[DMA_BUFFER_SIZE * DMA_BUFFER_NUM];
};

void set_sample_rate(uint32 rate);
void set_volume(uint16 volume);
uint16 get_volume();
void test_play();
void play();
void add_sound_node(struct sound_node *node);
