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

  Kernel kernel = Kernel(&kernel_pd, &phys_pm, &virt_pm, 0xc0000000);
  Global::allocator = &kernel;

  Terminal* current_term;

  TextModeTerminal* term = new TextModeTerminal(0xc0501000);
  current_term = term;

  TextModeTerminal* term2 = new TextModeTerminal(0xc0501000);
  term2->printf("Balls");

  init_idt();

  term->activate();
  term->clear_screen();

  term->printf("Hello, World!\n\nWe are running XnoeOS Code in C++ now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  term->printf("KERNEL OK!\n");
  
  Process* process = new Process(1);

  term->deactivate();
  term2->activate();

  while (1);

  init_keyboard();
  
  enable_idt();
  init_atapio();

  uint8_t* filebuffer = new uint8_t[0x3000];

  char* buffer = 0;

  while (1) {
    if (buffer)
      delete[] buffer;
    buffer = new char[128];
    printf(">>> ");
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