#include "idt.h"

GateEntry idt[256];

void set_entry(uint8_t interrupt_number, uint16_t code_segment, void* handler, uint8_t type) {
  uint32_t handler_addr = (uint32_t)handler;
  uint16_t* handler_halves = (uint16_t*)&handler_addr;
  idt[interrupt_number] = (GateEntry){
    .offset_low = handler_halves[0],
    .selector = code_segment,
    .zero = 0,
    .type = type,
    .offset_high = handler_halves[1]
  };
}

__attribute__((interrupt)) void interrupt_20(interrupt_frame* frame) {
//  printf("Interrupt 20 received!!\n");
  outb(0x20, 0x20);
}

__attribute__((interrupt)) void page_fault(interrupt_frame* frame, uint32_t err_code) {
  uint32_t problem_address;
  asm("mov %%cr2, %0" : "=a" (problem_address) :);
  printf("Page Fault at %x\n", problem_address);
}

__attribute__((interrupt)) void ignore_interrupt(interrupt_frame* frame) {
  outb(0x20, 0x20);
}

void init_idt() {
  idt_desc desc = {.size = 256 * sizeof(GateEntry) - 1, .offset = (uint32_t)idt};
  asm volatile("lidt %0" : : "m" (desc));
  for (int i=0; i<256; i++)
    set_entry(i, 0x08, &ignore_interrupt, 0x8E);
  
  set_entry(0x20, 0x08, &interrupt_20, 0x8E);
  set_entry(0xE, 0x08, &page_fault, 0x8E);

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
}

void enable_idt() {
  asm ("sti");
}