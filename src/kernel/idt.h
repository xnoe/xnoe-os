#ifndef IDT_H
#define IDT_H

#include "types.h"
#include "screenstuff.h"
#include "global.h"
#include "kernel.h"
#include "gdt.h"
#include "stdio/circularrwbuffer.h"

struct __attribute__((packed)) frame_struct {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t oesp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;

  uint32_t gate;
  uint32_t __ignored2;
  uint32_t errcode;

  uint32_t eip;
  uint16_t cs;
  uint16_t _ignored0;
  uint32_t eflags;
  uint16_t ss;
  uint16_t _ignored1;
  uint32_t esp;
};

extern void(*gates[256])(frame_struct*);

extern void load_idt();
void set_entry(uint8_t interrupt_number, uint16_t code_segment, void(*handler)(), uint8_t type, uint8_t privilege = 0);
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
};

struct __attribute__((packed)) idt_desc {
  uint16_t size;
  uint32_t offset;
};

#endif