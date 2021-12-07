#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "screenstuff.h"
#include "global.h"
#include "kernel.h"
#include "gdt.h"

struct interrupt_frame {
    uint32_t eip;
    uint16_t cs;
    uint16_t _ignored0;
    uint32_t eflags;
};
extern void load_idt();
void set_entry(uint8_t interrupt_number, uint16_t code_segment, void* handler, uint8_t type, uint8_t privilege = 0);
void init_idt();
void enable_idt();

struct __attribute__((packed)) GateEntry{
  uint16_t offset_low;
  uint16_t selector;
  uint8_t zero;
  uint8_t type : 4;
  uint8_t zero1 : 1;
  uint8_t privilege : 2;
  uint8_t present : 1;
  uint16_t offset_high;
} ;

struct __attribute__((packed)) idt_desc {
  uint16_t size;
  uint32_t offset;
} ;

#endif