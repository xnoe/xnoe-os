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

int main() {
  init_gdt();
  init_idt();
  init_term();

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved (as well as Virtual Memory / Paging!!!) and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");
  
  printf("KERNEL OK!\n");

  unmap_4k_virt(0x8000, kernel_page_directory, kernel_page_tables);

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