#ifndef GDT_H
#define GDT_H

#include "types.h"

struct __attribute__((packed)) tss_struct {
  uint16_t link;
  uint16_t _reserved0;
  uint32_t esp0;
  uint16_t ss0;
  uint16_t _reserved1;
  uint32_t esp1;
  uint16_t ss1;
  uint16_t _reserved2;
  uint32_t esp2;
  uint16_t ss2;
  uint16_t _reserved3;
  uint32_t cr3;
  uint32_t eip;
  uint32_t eflags;
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
  uint32_t esp;
  uint32_t ebp;
  uint32_t esi;
  uint32_t edi;
  uint16_t es;
  uint16_t _reserved4;
  uint16_t cs;
  uint16_t _reserved5;
  uint16_t ss;
  uint16_t _reserved6;
  uint16_t ds;
  uint16_t _reserved7;
  uint16_t fs;
  uint16_t _reserved8;
  uint16_t gs;
  uint16_t _reserved9;
  uint16_t ldtr;
  uint16_t _reserved10;
  uint16_t _reserved11;
  uint16_t iopb;
  constexpr tss_struct();
};

struct __attribute__((packed)) gdt_entry {
  uint32_t limit_lo : 16;
  uint32_t base_lo : 16;

  uint32_t base_mid : 8;
  uint32_t accessed : 1;
  uint32_t read_write : 1;
  uint32_t direction : 1;
  uint32_t executable : 1;
  uint32_t system_segment : 1;
  uint32_t privilege : 2;
  uint32_t present : 1;

  uint32_t limit_hi : 4;
  uint32_t __ignored__ : 2;
  uint32_t size : 1;
  uint32_t granularity : 1;
  
  uint32_t base_hi : 8;

  constexpr gdt_entry(uint32_t limit, uint32_t base, bool rw, bool exec, bool system, uint8_t ring);
  constexpr gdt_entry();
};

struct __attribute__((packed)) gdt_descr {
  uint16_t size;
  uint32_t offset;
};

void init_gdt();

#endif