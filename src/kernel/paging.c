#include "paging.h"

void map_4k_phys_to_virt(uint32_t physical, uint32_t virtual, PDE* page_directory, PTE** page_tables) {
  uint32_t pd_index = (virtual & (1023 << 20)) >> 20;
  uint32_t pt_index = (virtual & (1023 << 10)) >> 10;

  page_directory[pd_index] = (PDE){
    .address = (uint32_t)(page_tables[pd_index]) >> 12,
    .available = 0,
    .page_4mb = 0,
    .accessed = 0,
    .disable_cache = 0,
    .write_through_cache = 0,
    .privilege = 0,
    .present = 1,
    .read_write = 1,

    .ignored = 0,
    .ignored2 = 0
  };

  page_tables[pd_index][pt_index] = (PTE){
    .address = physical >> 12,
    .available = 0,
    .global = 0,
    .accessed = 0,
    .disable_cache = 0,
    .dirty = 0,
    .write_through_cache = 0,
    .privilege = 0,
    .present = 1,
    .read_write = 1,

    .ignored = 0
  };
}

void map_many_4k_phys_to_virt(uint32_t physical, uint32_t virtual, PDE* page_directory, PTE** page_tables, uint32_t count) {
  for (int i=0; i<count; i++)
    map_4k_phys_to_virt(physical + 4096*i, virtual + 4096*i, page_directory, page_tables);
}