#ifndef GDT_H
#define GDT_H

#include "types.h"

typedef struct {
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
}__attribute__((packed)) gdt_entry;

typedef struct {
  uint16_t size;
  uint32_t offset;
}__attribute__((packed)) gdt_descr;

#endif