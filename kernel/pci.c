#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"
#include "pci.h"

uint32 namba; // 0x1000
uint32 nabmba; // 0x1400
int add_node_count = 0;

uint8 data[DMA_BUFFER_NUM * DMA_BUFFER_SIZE];

static struct descriptor descriptor_list[DMA_BUFFER_NUM];
static struct sound_node* sound_queue_head;
static struct spinlock sound_lock;

volatile uint8* Pointer8(uint64 addr) { return (volatile uint8*)(addr); }
volatile uint16* Pointer16(uint64 addr) { return (volatile uint16*)(addr); }
volatile uint32* Pointer32(uint64 addr) { return (volatile uint32*)(addr); }
volatile uint64* Pointer64(uint64 addr) { return (volatile uint64*)(addr); }

volatile uchar* RegByte(uint64 reg) {return (volatile uchar *)(reg);}
uchar ReadRegByte(uint64 reg) {return *RegByte(reg);}

uint8 Read8(uint64 addr) { return *Pointer8(addr); }
uint16 Read16(uint64 addr) { return *Pointer16(addr); }
uint32 Read32(uint64 addr) { return *Pointer32(addr); }
uint64 Read64(uint64 addr) { return *Pointer64(addr); }

void Write8(uint64 addr, uint8 val) { *Pointer8(addr) = val; }
void Write16(uint64 addr, uint16 val) { *Pointer16(addr) = val; }
void Write32(uint64 addr, uint32 val) { *Pointer32(addr) = val; }
void Write64(uint64 addr, uint64 val) { *Pointer64(addr) = val; }

uint32
read_pci_config(uint16 bus, uint16 slot, uint16 func, uint16 offset)
{
  // PCI configuration space is mapped at 0x30000000
  uint32  *ecam = (uint32 *) PCIE_ECAM;
  
  // config address, 23 - 16: bus, 15 - 11: slot, 10 - 8: func, 7 - 0: offset
  uint32 offaddr = (bus << 16) | (slot << 11) | (func << 8) | (offset);
  volatile uint32 *addr = ecam + offaddr;
  return addr[0];
}

void
set_sample_rate(uint32 rate)
{
  // write rate to registers in NAM
  Write16(PCIE_PIO | (namba + 0x2c), rate & 0xffff);
  Write16(PCIE_PIO | (namba + 0x2e), rate & 0xffff);
  Write16(PCIE_PIO | (namba + 0x30), rate & 0xffff);
}

void
set_volume(uint16 volume)
{
  // set main volume to max
  Write16(PCIE_PIO | (namba + 0x02), 0x0);
  // set channel volume
  Write16(PCIE_PIO | (namba + 0x18), volume);
}

uint16
get_volume()
{
  return Read16(PCIE_PIO | (namba + 0x18));
}

void
clear_sound_queue()
{
  Write8(PCIE_PIO | (nabmba + 0x1b), 0); // clear control register
  Write8(PCIE_PIO | (nabmba + 0x1b), 2);

  // wait until registers are cleared
  uint wait = 1000;
  while (wait--) {
    if ((Read8(PCIE_PIO | (nabmba + 0x1b)) & 2) == 0) {
      // printf("success in clearing!\n");
      break;
    }
  }
  sound_queue_head = 0;
}

int
resume()
{
  int cur_status = Read8(PCIE_PIO | (nabmba + 0x1b)); // read control register
  if (cur_status == 0) {
    Write8(PCIE_PIO | (nabmba + 0x1b), 5); // resume
    return 0;
  }
  return -1;
}

int
pause()
{
  int cur_status = Read8(PCIE_PIO | (nabmba + 0x1b)); // read control register
  if (cur_status == 5) { // still playing
    Write8(PCIE_PIO | (nabmba + 0x1b), 0); // pause
    return 0;
  }
  return -1;
}

void
test_play()
{
  printf("in test play\n");

  // create test data
  acquire(&sound_lock);
  int len = DMA_BUFFER_NUM * DMA_BUFFER_SIZE;
  for (int i = 0; i < len; i++) data[i] = i % 256;
  release(&sound_lock);

  // check POCIV
  uint32 cycle = 1;
  uint8 last_POCIV = Read8(PCIE_PIO | (nabmba + 0x14)), new_POCIV;

  // write buffer descriptor info
  uint32 base_addr = (uint64)data;
  for (int i = 0; i < DMA_BUFFER_NUM; i++) {
    descriptor_list[i].buffer_pointer = base_addr + i * DMA_BUFFER_SIZE;
    descriptor_list[i].buffer_ctrl_and_len = 0x80000000 | DMA_SAMPLE_NUM;
  }
  // initialize the DMA engine
  uint32 base = (uint64)descriptor_list;
  Write32(PCIE_PIO | (nabmba + 0x10), base);

  // write the Last Valid Index
  Write8(PCIE_PIO | (nabmba + 0x15), 0x1F);

  // write the run bit
  Write8(PCIE_PIO | (nabmba + 0x1B), 5);

  // print the index of the current playing buffer
  while (1) {
    cycle--;
    if (cycle == 0) {
      new_POCIV = Read8(PCIE_PIO | (nabmba + 0x14));
      if (new_POCIV != last_POCIV) {
        printf("POCIV: %8\n", new_POCIV);
      }
      last_POCIV = new_POCIV;
      cycle = 1;
      if (last_POCIV == 0x1f) break;
    }
  }
}

void
play()
{
  // write buffer descriptor info
  uint32 base_addr = (uint64)(sound_queue_head -> data);
  for (int i = 0; i < DMA_BUFFER_NUM; i++) {
    descriptor_list[i].buffer_pointer = base_addr + i * DMA_BUFFER_SIZE;
    descriptor_list[i].buffer_ctrl_and_len = 0x80000000 | DMA_SAMPLE_NUM;
  }
  
  if (sound_queue_head->flag != 0) { // the first one isn't played yet
    // initialize the DMA engine
    Write32(PCIE_PIO | (nabmba + 0x10), (uint64)descriptor_list);

    // write the Last Valid Index
    Write8(PCIE_PIO | (nabmba + 0x15), 0x1F);

    // write the run bit
    Write8(PCIE_PIO | (nabmba + 0x1B), 5);
  }
  
}

// process interrupt: soundQueue go front 1.
void
soundintr(void) {
  acquire(&sound_lock);
  if (sound_queue_head == 0) { // no sound playing yet receiving interrupt
    panic("empty sound queue, yet interrupt received");
    return;
  }
  // clear the interrupt register
  Write16(PCIE_PIO | (nabmba + 0x16), 0x1c);
  // remove the first node
  sound_queue_head->flag = 0;
  sound_queue_head = sound_queue_head -> next;
  // end of the queue
  if (sound_queue_head == 0) {
    release(&sound_lock);
    return;
  }
  // write data to BDL
  uint32 base_addr = (uint64)(sound_queue_head -> data);
  for (int i = 0; i < DMA_BUFFER_NUM; ++i) {
    descriptor_list[i].buffer_pointer = base_addr + i * DMA_BUFFER_SIZE;
    descriptor_list[i].buffer_ctrl_and_len = 0x80000000 | DMA_SAMPLE_NUM;
  }
  // set playing register
  Write8(PCIE_PIO | (nabmba + 0x1b), 5);
  release(&sound_lock);
}

void
add_sound_node(struct sound_node *node)
{
  acquire(&sound_lock);
  node->next = 0;
  struct sound_node **tail;
  for (tail = &sound_queue_head; *tail; tail = &(*tail)->next) ;
  *tail = node;
  // the queue was empty
  if (sound_queue_head == node) play();
  release(&sound_lock);
}

void
pci_init()
{
  // enumerate all possible 32 slots for devices on bus 0
  for (int slot = 0; slot < 32; slot++) {
    for (int func = 0; func < 8; func++) {
      uint16 bus = 0, offset = 0;
      
      // read device id & vendor id
      uint32 id = read_pci_config(bus, slot, func, offset);
      // 2415:8086 is Intel's AC97-ICH0( device:vendor )
      if (id == 0x24158086) {
        printf("Found AC97 at slot %d, func %d\n", slot, func);

        sound_card_init(bus, slot, func, offset);
      }
    }
  }
  printf("sound card initialized\n");
}

void
sound_card_init(uint16 bus, uint16 slot, uint16 func, uint16 offset)
{
  initlock(&sound_lock, "sound");
  uint32 wait_time = 0;
  
  // PCI configuration space is mapped at 0x30000000
  uint32  *ecam = (uint32 *) PCIE_ECAM;
  
  // config address, 23 - 16: bus, 15 - 11: slot, 10 - 8: func, 7 - 0: offset
  uint32 offaddr = (bus << 16) | (slot << 11) | (func << 8) | (offset);
  uint64 addr = (uint64)(ecam + offaddr);

  // locate command register, enable bus master, I/O space
  Write8(addr + PCI_CONFIG_SPACE_CMD, 0x5);

  // controller address
  Write32(addr + PCI_CONFIG_NAMBA, 0x1001);
  namba = Read32(addr + PCI_CONFIG_NAMBA) & (~0x1);
  Write32(addr + PCI_CONFIG_NABMBA, 0x1401);
  nabmba = Read32(addr + PCI_CONFIG_NABMBA) & (~0x1);

  // removing AC_RESET# bit
  Write8(PCIE_PIO | (nabmba + 0x2c), 0x3);

  // wait for codec to be ready
  wait_time = 1000;
  while (!(Read16(PCIE_PIO | (nabmba + 0x30)) & (0x100)) && wait_time > 0) {
    --wait_time;
  }
  if (!wait_time) {
    panic("AC_RESET# bit not cleared for the first time\n");
    return;
  }

  //Write32(PCIE_PIO | (nabmba + 0x2c), 0x1);

  // determine the audio codec
  uint16 codec_master_volume = Read16(PCIE_PIO | (nabmba + 0x02));
  Write16(PCIE_PIO | (namba + 0x02), 0x8000);
  if (Read16(PCIE_PIO | (namba + 0x02)) != 0x8000) {
    panic("Audio codec is not detected\n");
    Write16(PCIE_PIO | (namba + 0x02), codec_master_volume);
    return;
  }

  // program subsystem ID
  uint32 codec_vendor_id1 = Read16(PCIE_PIO | (namba + 0x7c));
  uint32 codec_vendor_id2 = Read16(PCIE_PIO | (namba + 0x7e));

  uint32 vendor_id = (codec_vendor_id2 << 16) + codec_vendor_id1;
  Write32(addr + 0x2c, vendor_id);

  // initialize the DMA engine
  uint32 base = (uint64)descriptor_list;
  Write32(PCIE_PIO | (nabmba + 0x10), base);

  // initialize sample rate and volume
  set_sample_rate(44100);
  set_volume(0x808);
  
  //test_play();
}
