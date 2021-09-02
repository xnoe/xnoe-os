#include "types.h"
#include "screenstuff.h"
#include "io.h"

int main() {
  init_term();

  printf("KERNEL32 OK!\n");

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");

  while (1);
}