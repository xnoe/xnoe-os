#include "types.h"
#include "screenstuff.h"
#include "io.h"
#include "idt.h"

__attribute__((interrupt)) void interrupt_21(struct interrupt_frame* frame) {
  printf("Interrupt 20 received!!\n");
  outb(0x20, 0x20);
}

int main() {
  init_idt();
  set_entry(0x20, 0x08, &interrupt_21, 0x8E);
  init_term();

  printf("KERNEL32 OK!\n");

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");

  outb(0x20, 0x11);
  outb(0xA0, 0x11);
  outb(0x21, 0x20);
  outb(0xA1, 0x28);
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x00);
  outb(0xA1, 0x00);

  enable_idt();

  while (1);
}