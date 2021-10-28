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
#include "datatypes/hashtable.h"

int main() {
  init_gdt();
  init_term();

  PageDirectory kernel_pd = PageDirectory(0xc0100000, 0x120000, 0xbffe0000);
  kernel_pd.select();
  kernel_pd.unmap(0x8000);

  PageMap phys_pm(0xc0600000);
  PageMap virt_pm(0xc0620000);

  Allocator kernel_allocator = Allocator(&kernel_pd, &phys_pm, &virt_pm, 0xd0000000);

  xnoe::hashtable<void*, uint32_t>* kproc_hashtable = new (&kernel_allocator) xnoe::hashtable<void*, uint32_t>(&kernel_allocator);

  Process kernel_process = Process(0, kproc_hashtable, &kernel_pd, &phys_pm, &virt_pm, 0xd0000000);

  uint32_t* test = new(&kernel_process)uint32_t;
  uint32_t* test2 = new(&kernel_process)uint32_t;
  uint32_t* test3 = new(&kernel_process)uint32_t[1024];

  *test = 0xdead;
  *test2 = 0xbeef;

  init_idt();

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  printf("KERNEL OK!\n");

  printf("Test  :%x\nTest 2:%x\nTest 3:%x\n", test, test2, test3);
  printf("Test value  :%x\nTest 2 value:%x\n", *test, *test2);

  init_keyboard();
  
  enable_idt();
  init_atapio();

  uint8_t sector[512];

  read_sector(0, sector);

  uint8_t* filebuffer = new(&kernel_process)uint8_t[0x3000];

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