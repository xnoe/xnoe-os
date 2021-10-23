#include "paging.h"

void map_4k_phys_to_virt(uint32_t physical, uint32_t virt, PDE* page_directory, PTE** page_tables) {
  split_addr* split = (split_addr*)&virt;

  page_directory[split->pd_index] = (PDE){
    .present = 1,
    .read_write = 1,
    .privilege = 0,
    .write_through_cache = 0,
    .disable_cache = 0,
    .accessed = 0,
    .ignored2 = 0,
    .page_4mb = 0,
    .ignored = 0,
    .available = 0,
    .address = (uint32_t)(page_tables[split->pd_index]) >> 12
  };

  ((PTE*)((uint32_t)page_tables[split->pd_index] + 0xbffe0000))[split->pt_index] = (PTE){
    .present = 1,
    .read_write = 1,
    .privilege = 0,
    .write_through_cache = 0,
    .disable_cache = 0,
    .accessed = 0,
    .dirty = 0,
    .ignored = 0,
    .global = 0,
    .available = 0,
    .address = physical >> 12
  };
}

void map_many_4k_phys_to_virt(uint32_t physical, uint32_t virt, PDE* page_directory, PTE** page_tables, uint32_t count) {
  for (int i=0; i<count; i++)
    map_4k_phys_to_virt(physical + 4096*i, virt + 4096*i, page_directory, page_tables);
}

void unmap_4k_virt(uint32_t virt, PDE* page_directory, PTE** page_tables) {
  split_addr* split = (split_addr*)&virt;

  ((PTE*)((uint32_t)page_tables[split->pd_index] + 0xbffe0000))[split->pt_index] = (PTE){
    .present = 0,
    .read_write = 0,
    .privilege = 0,
    .write_through_cache = 0,
    .disable_cache = 0,
    .accessed = 0,
    .dirty = 0,
    .ignored = 0,
    .global = 0,
    .available = 0,
    .address = 0
  };
}

uint32_t PDE::getPhysicalPTAddress() {
  return this->address << 12;
}