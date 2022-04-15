#include "types.h"
#include "io.h"
#include "idt.h"
#include "keyboard.h"
#include "strings.h"
#include "ata.h"
#include "gdt.h"
#include "paging.h"
#include "allocate.h"
#include "memory.h"
#include "process.h"
#include "global.h"
#include "datatypes/hashtable.h"
#include "terminal.h"
#include "kernel.h"
#include "filesystem/fstree.h"
#include "filesystem/fat16.h"
#include "filesystem/devfs.h"

struct KernelInformationStruct {
  PDE* pde;
  uint32_t page_directory_phys_addr;
  uint32_t page_directory_phys_offset;
  uint32_t page_bitmap_phys;
  uint32_t page_bitmap_virt;
  uint32_t stack_ptr;
  uint32_t vga_addr;
  uint32_t remainingPages;
};

int main(KernelInformationStruct* kstruct) {
  init_gdt();
  
  PageDirectory kernel_pd = PageDirectory(kstruct->pde, kstruct->page_directory_phys_addr, kstruct->page_directory_phys_offset);

  kernel_pd.select();
  kernel_pd.unmap(0x8000);

  PageMap phys_pm(kstruct->page_bitmap_phys, kstruct->remainingPages);
  PageMap virt_pm(kstruct->page_bitmap_virt);

  Kernel kernel = Kernel(&kernel_pd, &phys_pm, &virt_pm, 0xc0000000, kstruct->stack_ptr);
  kernel.init_kernel();

  VGAModeTerminal* term = new VGAModeTerminal(kstruct->vga_addr);

  kernel.terminal = term;

  init_idt();

  term->activate();
  term->clear_screen();

  term->printf("Hello, World!\n\nWe are running XnoeOS Code in C++ now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  term->printf("KERNEL OK!\n");

  ReadWriter* atareadwriter = new ATAReadWriter(0, 0);

  uint8_t* buffer = new uint8_t[512];
  for (int i=0;i<512;i++)
    buffer[i]=0;
  uint32_t size = atareadwriter->size();
  atareadwriter->seek(268*512);
  atareadwriter->read(512, buffer);

  kernel.rootfs = new RootFSTree();
  kernel.rootfs->mount(createPathFromString("/dev"), new DevFS());
  kernel.rootfs->mount(createPathFromString("/"), new FAT16FS(kernel.rootfs->open(createPathFromString("/dev/ata"))));
  ReadWriter* worldbin = kernel.rootfs->open(createPathFromString("/world.bin"));
  uint32_t fh = kernel.mapFH(worldbin);

  Process* p1 = kernel.createProcess(fh, term);

  init_keyboard();
  if (worldbin) {
    worldbin->seek(0);
    worldbin->read(512, buffer);
    worldbin->seek(0);
    enable_idt();
  }

  while (1) asm ("hlt");
}