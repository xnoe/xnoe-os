#include "idt.h"

GateEntry idt[256];

void set_entry(uint8_t interrupt_number, uint16_t code_segment, void* handler, uint8_t type) {
  uint32_t handler_addr = (uint16_t)handler;
  uint16_t* handler_halves = (uint16_t*)&handler_addr;
  idt[interrupt_number] = (GateEntry){
    .offset_low = handler_halves[0],
    .selector = code_segment,
    .zero = 0,
    .type = type,
    .offset_high = handler_halves[1]
  };
}

__attribute__((interrupt)) void ignore_interrupt(struct interrupt_frame* frame) {
  // Do nothing
}

void init_idt() {
  idt_desc desc = {.size = 256 * sizeof(GateEntry) - 1, .offset = (uint32_t)idt};
  asm volatile("lidt %0" : : "m" (desc));
  for (int i=0; i<256; i++)
    set_entry(i, 0x08, &ignore_interrupt, 0x8E);
}

void enable_idt() {
  asm ("sti");
}