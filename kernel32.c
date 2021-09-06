#include "types.h"
#include "screenstuff.h"
#include "io.h"
#include "idt.h"
#include "keyboard.h"

#include "strings.h"

int main() {
  init_idt();
  init_term();

  printf("KERNEL32 OK!\n");

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");

  init_keyboard();
  enable_idt();


  while (1) {
    printf(">>> ");
    char buffer[128];
    for (int i=0; i<128; i++)
      buffer[i] = 0;
    readline(128, buffer);

    char* argv[32];
    int argc = string_split(' ', buffer, argv);


    if (strcmp(argv[0], "help", 4)) {
      printf(
        "XnoeOS 32 Bit Mode Help.\n"
        "------------------------\n"
        "- help\n"
        ": Shows this message\n"
        "- clear\n"
        ": Clears the screen\n"
      );
    } else if (strcmp(argv[0], "clear", 5)) {
      clear_screen();
      set_curpos(0, 0);
    } else if (strcmp(argv[0], "echo", 4)) {
      int index = 0;
      while (++index <= argc)
        printf("%s ", argv[index]);
      printf("\n");
    }
  }
}