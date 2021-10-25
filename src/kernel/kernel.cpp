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

int main() {
  init_gdt();

  PageDirectory kernel_pd = PageDirectory(0xc0100000, 0x120000, 0xbffe0000);
  kernel_pd.select();
  kernel_pd.unmap(0x8000);

  PageMap phys_pm(0xc0600000);
  PageMap virt_pm(0xc0620000);

  Allocator kernel_allocator = Allocator(&kernel_pd, &phys_pm, &virt_pm, 0xd0000000);

  void* alloc = kernel_allocator.allocate(4096);
  void* alloc2 = kernel_allocator.allocate(4096);

  init_idt();
  init_term();

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  printf("KERNEL OK!\n");

  printf("Alloc: %x\nAlloc2: %x\n", alloc, alloc2);

  init_keyboard();
  
  enable_idt();
  init_atapio();

  uint8_t sector[512];

  read_sector(0, sector);

  uint8_t* filebuffer = (uint8_t*)dumb_alloc(0x3000);

  while (1) {
    printf(">>> ");
    char buffer[128];
    for (int i=0; i<128; i++)
      buffer[i] = 0;
    readline(128, buffer);

    char* rest = split_on_first(' ', buffer);

    if (strcmp(buffer, "help", 4)) {
      printf(
        "XnoeOS 32 Bit Mode Help.\n"
        "------------------------\n"
        " - help\n"
        " : Shows this message\n"
        " - clear\n"
        " : Clears the screen\n"
        " - echo\n"
        " : Repeats the text written afterwards\n"
        " - type\n"
        " : Prints the contents of a file\n"
      );
    } else if (strcmp(buffer, "clear", 5)) {
      clear_screen();
      set_curpos(0, 0);
    } else if (strcmp(buffer, "echo", 4)) {
      printf("%s\n", rest);
    } else if (strcmp(buffer, "type", 4)) {
      char filenamebuffer[12];

      decode_filename(rest, filenamebuffer);
      if (!file_exists(filenamebuffer)) {
        printf("File %s not found!\n", filenamebuffer);
        continue;
      }

      for (int i=0; i<4096; i++)
        filebuffer[i] = 0;
      
      load_file(filenamebuffer, filebuffer);
      printf(filebuffer);
    } else if (strcmp(buffer, "pagefault", 9)) {
      uint32_t* bad_addr = 0xdeadbeef;
      uint32_t a = *bad_addr;
    } else {
      char filenamebuffer[12];
      decode_filename(buffer, filenamebuffer);
      if (!file_exists(filenamebuffer)) {
        printf("Bad Command or filename!\n");
        continue;
      }
    }
  }
}