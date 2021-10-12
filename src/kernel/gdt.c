#include "gdt.h"

gdt_entry gdt[] = {
  (gdt_entry){ // Null Segment
    .limit_lo = 0,
    .limit_hi = 0,
    .base_lo = 0,
    .base_mid = 0,
    .base_hi = 0,
    .accessed = 0,
    .read_write = 0,
    .direction = 0,
    .executable = 0,
    .system_segment = 0,
    .privilege = 0,
    .present = 0,
    .size = 0,
    .granularity = 0,

    .__ignored__ = 0,
  },
  (gdt_entry){ // Code Segment
    .limit_lo = 0xffff,
    .limit_hi = 0xf,
    .base_lo = 0,
    .base_mid = 0,
    .base_hi = 0,
    .accessed = 0,
    .read_write = 1,
    .direction = 0,
    .executable = 1,
    .system_segment = 1,
    .privilege = 0,
    .present = 1,
    .size = 1,
    .granularity = 1,

    .__ignored__ = 0,
  },
  (gdt_entry){ // Data Segment
    .limit_lo = 0xffff,
    .limit_hi = 0xf,
    .base_lo = 0,
    .base_mid = 0,
    .base_hi = 0,
    .accessed = 0,
    .read_write = 1,
    .direction = 0,
    .executable = 0,
    .system_segment = 1,
    .privilege = 0,
    .present = 1,

    .size = 1,
    .granularity = 1,

    .__ignored__ = 0,
  }
};

gdt_descr descr = (gdt_descr){
  .size = sizeof(gdt) - 1,
  .offset = gdt,
};

__attribute__((far)) void far_call() {}

void init_gdt() {
  asm volatile("lgdt %0;"
               "mov $0x10, %%eax;"
               "mov %%eax, %%ss;"
               "mov $0x10, %%eax;"
               "mov %%eax, %%ds" : : "m" (descr));
  
  far_call();
}