#include "gdt.h"

tss_struct tss = (tss_struct) {
  .link = 0,
  ._reserved0 = 0,
  .esp0 = 0xc1006000,
  .ss0 = 0x10,
  ._reserved1 = 0,
  .esp1 = 0,
  .ss1 = 0,
  ._reserved2 = 0,
  .esp2 = 0,
  .ss2 = 0,
  ._reserved3 = 0,
  .cr3 = 0,
  .eip = 0,
  .eflags = 0,
  .eax = 0,
  .ebx = 0,
  .ecx = 0,
  .edx = 0,
  .esp = 0,
  .ebp = 0,
  .esi = 0,
  .edi = 0,
  .es = 0,
  ._reserved4 = 0,
  .cs = 0,
  ._reserved5 = 0,
  .ss = 0,
  ._reserved6 = 0,
  .ds = 0,
  ._reserved7 = 0,
  .fs = 0,
  ._reserved8 = 0,
  .gs = 0,
  ._reserved9 = 0,
  .ldtr = 0,
  ._reserved10 = 0,
  ._reserved11 = 0,
  .iopb = 104
};

constexpr gdt_entry::gdt_entry(uint32_t limit, uint32_t base, bool rw, bool exec, bool system, uint8_t ring) :
  limit_lo(limit & 0xffff),
  limit_hi((limit & 0xf0000) >> 16),
  base_lo(base & 0xffff),
  base_mid((base & 0xff0000) >> 16),
  base_hi((base & 0xff000000) >> 24),
  __ignored__(0),
  accessed(0),
  granularity(1),
  size(1),
  present(1),
  read_write(rw),
  executable(exec),
  system_segment(system),
  privilege(ring),
  direction(0)
{}

constexpr gdt_entry::gdt_entry() :
  limit_lo(0),
  base_lo(0),
  base_mid(0),
  accessed(0),
  read_write(0),
  direction(0),
  executable(0),
  system_segment(0),
  privilege(0),
  present(0),
  limit_hi(0),
  __ignored__(0),
  size(0),
  granularity(0),
  base_hi(0)
{}

gdt_entry gdt[] = {
  gdt_entry(), // Null Segment
  gdt_entry(0xfffff, 0, 1, 1, 1, 0), // Kernel Code Segment
  gdt_entry(0xfffff, 0, 1, 0, 1, 0), // Kernel Data Segment

  gdt_entry(0xfffff, 0, 1, 1, 1, 3), // User Code Segment
  gdt_entry(0xfffff, 0, 1, 0, 1, 3), // User Data Segment

  gdt_entry() // Empty Task State Segment
};

gdt_descr descr = (gdt_descr){
  .size = sizeof(gdt) - 1,
  .offset = gdt,
};

void init_gdt() {
  gdt[5] = gdt_entry(sizeof(tss), &tss, 0, 1, 0, 0); // Initialise the TSS.
  gdt[5].accessed = 1;
  asm volatile("lgdt %0;"
               "mov $0x10, %%eax;"
               "mov %%eax, %%ss;"
               "mov %%eax, %%ds;"
               "mov $0x28, %%ax;"
               "ltr %%ax" : : "m" (descr));
}