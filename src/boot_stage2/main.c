#include "atapio.h"
#include "screenstuff.h"
#include "paging.h"

typedef struct {
  uint32_t base_low;
  uint32_t base_high;
  uint32_t length_low;
  uint32_t length_high;
  uint32_t type;
  uint32_t acpi3_extended;
}__attribute__((packed)) e820entry;

uint8_t* bitmap = 0x100000;

void set_bit(uint32_t offset, uint8_t* buffer) {
  uint32_t index = offset / 8;
  uint32_t bit = offset % 8;

  buffer[index] |= (1<<(7 - bit));
}

void unset_bit(uint32_t offset, uint8_t* buffer) {
  uint32_t index = offset / 8;
  uint32_t bit = offset % 8;

  buffer[index] &= (255 - (1<<(7 - bit)));
}

void memset(uint8_t* base, uint32_t count, uint8_t to) {
  for (int i=0; i<count; i++) {
    base[i] = to;
  }
}

void mark_unavailble(uint32_t address, uint32_t size) {
  // This function takes an address and length and marks the corresponding pages as unavailable.
  address -= address % 4096; 
  if (size % 4096)
    size += 4096 - (size % 4096);

  address /= 4096;
  size /= 4096;

  for (int i=0; i<size; i++) {
    unset_bit(address + i, bitmap);
  }
}

char* stringify_type(uint32_t type) {
  switch (type) {
    case 1:
      return "Usable";
    case 3:
      return "ACPI Reclaimable";
    case 4:
      return "ACPI NVS";
    case 5:
      return "Bad memory";
    default:
      return "Reserved";
  }
}

void main() {
  init_term();
  init_atapio();

  // e820 memory map exists at 0x20000
  e820entry* e820_entries = 0x20000;

  // Zero out the bitmap.
  memset(bitmap, 0x20000, 0);
  // Ensure the bitmap data is clear

  for (int i=0; i<0x20000; i++)
    if (bitmap[i])
      printf("Found data in bitmap at %x!\n", (bitmap+i));

  for (int i=0; e820_entries[i].length_low != 0 || e820_entries[i].length_high != 0; i++) {
    e820entry entry = e820_entries[i];
    printf("BIOS-e820: Starting %x%x, length %x%x is %s\n", entry.base_high, entry.base_low, entry.length_high, entry.length_low, stringify_type(entry.type));

    if (entry.type != 1)
      continue;
    
    uint32_t base = entry.base_low;
    uint32_t length = entry.length_low;

    if (base % 4096) { 
      // Memory isn't page aligned, we need to fix that.
      uint32_t add_offset = 4096 - (base % 4096);
      if (length > add_offset) {
        base += add_offset;
        length -= add_offset;
      }
    }
    uint32_t page_index = base / 4096;

    printf("Page Index: %d\nLength (Pages): %d\n", page_index, length / 4096);

    for (int j=0; length > 4096; length -= 4096, j++) {
      set_bit(page_index + j, bitmap);
    }
  }

  mark_unavailble(bitmap, 0x20000);

  // Page Directory
  PDE* kernel_page_directory = bitmap + 0x20000;
  // Clear the PD 
  memset((uint8_t*)kernel_page_directory, 4096, 0);

  // Clear 4MB of RAM from 0x121000 to 0x521000 for the 1024 page tables
  memset((uint8_t*)0x121000, 0x400000, 0);

  // Construct a 1024 long PTE** at 0x521000
  for (int i=0; i<1024; i++) {
    ((uint32_t*)0x521000)[i] = 0x121000 + 0x1000*i;
  }

  PTE** kernel_page_tables = 0x521000;

  // Mark unavailable bitmap to 0x522000
  mark_unavailble(bitmap, 0x4000000);

  // Now we want to map some stuff.
  // But first, we should load the kernel somewhere

  uint8_t* kernel_location = 0x522000; // Just load it at 0x522000 for now
  mark_unavailble(0x522000, 32768); // Just treat the kernel as not growing beyong 32k for now.

  map_many_4k_phys_to_virt(0x522000, 0xc0000000, kernel_page_directory, kernel_page_tables, 8); // Map 8 pages from 0x522000 to 0xc0000000;
  map_4k_phys_to_virt((uint32_t)kernel_page_directory, 0xc0100000, kernel_page_directory, kernel_page_tables); // Map the page directory to 0xc0100000
  map_many_4k_phys_to_virt(0x121000, 0xc0101000, kernel_page_directory, kernel_page_tables, 1024); // Map 1024 pages from 0x121000 to 0xc0101000
  map_4k_phys_to_virt(0xb8000, 0xc0501000, kernel_page_directory, kernel_page_tables); // Map 0xb8000 (video) to 0xc0501000
  map_4k_phys_to_virt(0x521000, 0xc0502000, kernel_page_directory, kernel_page_tables); // Map the PTE** data, we'll need to convert the pointers to point at kernel space at some point.

  // Map the bitmap 
  map_many_4k_phys_to_virt(0x100000, 0xc0600000, kernel_page_directory, kernel_page_tables, 32);

  map_4k_phys_to_virt(0x8000, 0x8000, kernel_page_directory, kernel_page_tables);
  map_many_4k_phys_to_virt(0x8a000, 0x8a000, kernel_page_directory, kernel_page_tables, 6);

  load_file("KERNEL  BIN", kernel_location);

  printf("Stage2 success!\n");

  //while (1);

  asm volatile("mov %0, %%eax;" 
       "mov %%eax, %%cr3;"
       "mov %%cr0, %%eax;"
       "or $0x80000000, %%eax;"
       "mov %%eax, %%cr0" : : "m" (kernel_page_directory));

  ((void(*)(void))0xc0000000)();
}