#include "allocate.h"

uint8_t* bitmap = 0xc0600000;
PDE* kernel_page_directory = 0xc0100000;
PTE** kernel_page_tables = 0xc0502000;

uint32_t kernel_allocate_area = 0xf0000000;

uint32_t last_free_page = 0;

void set_bit(uint32_t offset, uint8_t* buffer) {
  uint32_t index = offset / 8;
  uint32_t bit = offset % 8;

  buffer[index] |= (1<<(7 - bit));
}

uint8_t get_bit(uint32_t offset, uint8_t* buffer) {
  uint32_t index = offset / 8;
  uint32_t bit = offset % 8;

  return buffer[index] & (1<<(7 - bit));
}

void unset_bit(uint32_t offset, uint8_t* buffer) {
  uint32_t index = offset / 8;
  uint32_t bit = offset % 8;

  buffer[index] &= (255 - (1<<(7 - bit)));
}

void mark_unavailble(uint32_t address, uint32_t size) {
  // This function takes a physical address and length and marks the corresponding pages as unavailable.
  address -= address % 4096; 
  if (size % 4096)
    size += 4096 - (size % 4096);

  address /= 4096;
  size /= 4096;

  for (int i=0; i<size; i++) {
    unset_bit(address + i, bitmap);
  }
}

void* dumb_alloc(uint32_t size) {
  if (size % 4096)
    size += 4096 - (size % 4096); // Since this is dumb alloc just make the amount we're allocating be in whole page sizes

  uint32_t ptr = kernel_allocate_area;

  while (size > 0) {
    for (; get_bit(last_free_page, bitmap) == 0; last_free_page++);

    uint32_t phys_addr = last_free_page * 4096;

    mark_unavailble(phys_addr, 4096);
    map_4k_phys_to_virt(phys_addr, kernel_allocate_area, kernel_page_directory, kernel_page_tables);

    kernel_allocate_area += 4096;
    size -= 4096;
  }

  return (void*)ptr;
}

xnoe::tuple<uint32_t, void*> alloc_page_with_phys() {
  uint32_t ptr = kernel_allocate_area;

  for (; get_bit(last_free_page, bitmap) == 0; last_free_page++);
  uint32_t phys_addr = last_free_page * 4096;
  mark_unavailble(phys_addr, 4096);
  map_4k_phys_to_virt(phys_addr, kernel_allocate_area, kernel_page_directory, kernel_page_tables);
  kernel_allocate_area += 4096;

  return xnoe::tuple<uint32_t, void*>(phys_addr, (void*)ptr);
}