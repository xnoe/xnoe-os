#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "screenstuff.h"

struct interrupt_frame {
    uint16_t ip;
    uint16_t cs;
    uint16_t flags;
    uint16_t sp;
    uint16_t ss;
};
extern void load_idt();
void set_entry(uint8_t interrupt_number, uint16_t code_segment, void* handler, uint8_t type);
void init_idt();
void enable_idt();

struct __attribute__((packed)) GateEntry{
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type;
  uint16_t offset_high;
} ;

struct __attribute__((packed)) idt_desc {
  uint16_t size;
  uint32_t offset;
} ;

#endif