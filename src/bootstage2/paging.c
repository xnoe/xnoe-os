#include "paging.h"

typedef struct {
  uint32_t page_offset : 12;
  uint32_t pt_index : 10;
  uint32_t pd_index : 10;
}__attribute__((packed)) split_addr;

void map_4k_phys_to_virt(uint32_t physical, uint32_t virtual, PDE* page_directory, PTE** page_tables) {
  split_addr* split = (split_addr*)&virtual;

  page_directory[split->pd_index] = (PDE){
    .address = (uint32_t)(page_tables[split->pd_index]) >> 12,
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

  page_tables[split->pd_index][split->pt_index] = (PTE){
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