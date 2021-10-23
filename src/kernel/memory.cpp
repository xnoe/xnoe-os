#include "memory.h"

void memset(uint8_t* address, uint32_t count, uint8_t value) {
  for (int i=0; i<count; i++)
    address[i] = value;
}

void memcpy(uint8_t* src, uint8_t* dst, uint32_t count) {
  for (int i = 0; i<count; i++)
    dst[i] = src[i];
}

PageTable::PageTable(uint32_t phys, uint32_t virt) {
  phys_addr = phys;
  virt_addr = virt;

  page_table = (PTE*)virt;
}

PageTable::PageTable(){}

void PageTable::map_table(uint32_t index, uint32_t addr) {
  page_table[index] = (PTE){
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
    .address = addr
  };
}

void PageTable::unmap_table(uint32_t index) {
  page_table[index] = (PTE){
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

PageDirectory::PageDirectory(PDE* page_directory, uint32_t phys_addr, uint32_t offset) {
  this->page_directory = page_directory;
  this->phys_addr = phys_addr;

  for (int i=0; i<1024; i++) {
    uint32_t table_phys_addr = page_directory[i].getPhysicalPTAddress();
    page_tables[i] = PageTable(table_phys_addr >> 12, table_phys_addr + offset);
  }
}

void PageDirectory::map(uint32_t phys, uint32_t virt) {
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
    .address = page_tables[split->pd_index].phys_addr
  };

  page_tables[split->pd_index].map_table(split->pt_index, phys >> 12);
}

void PageDirectory::unmap(uint32_t virt) {
  split_addr* split = (split_addr*)&virt;
  page_tables[split->pd_index].unmap_table(split->pt_index);
}

void PageDirectory::select() {
  asm volatile("mov %0, %%eax; mov %%eax, %%cr3" : : "m" (phys_addr));
}