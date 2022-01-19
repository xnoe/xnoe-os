#include "types.h"
#include "screenstuff.h"
#include "io.h"
#include "idt.h"
#include "keyboard.h"
#include "strings.h"
#include "atapio.h"
#include "gdt.h"
#include "paging.h"
#include "allocate.h"
#include "memory.h"
#include "process.h"
#include "global.h"
#include "datatypes/hashtable.h"
#include "terminal.h"
#include "kernel.h"

int main() {
  init_gdt();
  init_term();
  
  PageDirectory kernel_pd = PageDirectory(0xc0100000, 0x120000, 0xbffe0000);

  kernel_pd.select();
  kernel_pd.unmap(0x8000);

  PageMap phys_pm(0xc0600000);
  PageMap virt_pm(0xc0620000);

  Kernel kernel = Kernel(&kernel_pd, &phys_pm, &virt_pm, 0xc0000000, 0xc1006000);
  kernel.init_kernel();
  init_atapio();

  TextModeTerminal* term = new TextModeTerminal(0xc0501000);

  kernel.terminal = term;

  init_idt();

  term->activate();
  term->clear_screen();

  term->printf("Hello, World!\n\nWe are running XnoeOS Code in C++ now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  term->printf("KERNEL OK!\n");

  ReadWriter* worldbin = new FATFileReadWriter(0, "etc/world.bin");
  uint32_t fh = kernel.mapFH(worldbin);

  Process* p1 = kernel.createProcess(fh, term);

  init_keyboard();
  
  enable_idt();

  while (1) asm ("hlt");
}