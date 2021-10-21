#ifndef PAGING_H
#define PAGING_H

#include <stdbool.h>
#include "types.h"

struct __attribute__((packed)) PDE {
  uint32_t present : 1;
  uint32_t read_write : 1;
  uint32_t privilege : 1;
  uint32_t write_through_cache : 1;
  uint32_t disable_cache : 1;
  uint32_t accessed : 1;
  uint32_t ignored2 : 1;
  uint32_t page_4mb : 1;
  uint32_t ignored : 1;
  uint32_t available : 3;
  uint32_t address : 20;
};

struct __attribute__((packed)) PTE {
  uint32_t present : 1;
  uint32_t read_write : 1;
  uint32_t privilege : 1;
  uint32_t write_through_cache : 1;
  uint32_t disable_cache : 1;
  uint32_t accessed : 1;
  uint32_t dirty : 1;
  uint32_t ignored : 1;
  uint32_t global : 1;
  uint32_t available : 3;
  uint32_t address : 20;
};

void map_4k_phys_to_virt(uint32_t physical, uint32_t virt, PDE* page_directory, PTE** page_tables);
void map_many_4k_phys_to_virt(uint32_t physical, uint32_t virt, PDE* page_directory, PTE** page_tables, uint32_t count);

void unmap_4k_virt(uint32_t virt, PDE* page_directory, PTE** page_tables);
#endif