#include "memory.h"

void memset(uint8_t* address, uint32_t count, uint8_t value) {
  for (int i=0; i<count; i++)
    address[i] = value;
}

void memcpy(uint8_t* src, uint8_t* dst, uint32_t count) {
  for (int i = 0; i<count; i++)
    dst[i] = src[i];
}

PageMap::PageMap(uint32_t map) {
  this->pagemap = (uint8_t*)map;
}

PageMap::PageMap() {
  this->pagemap = new uint8_t[131072];
  memset(this->pagemap, 131072, 0xff);
}

PageMap::~PageMap() {
  delete pagemap;
}

void PageMap::set_bit(uint32_t index) {
  uint32_t offset = index % 8;
  uint32_t i = index / 8;
  
  pagemap[i] |= 1<<(7-offset);
}

void PageMap::unset_bit(uint32_t index) {
  uint32_t offset = index % 8;
  uint32_t i = index / 8;
  
  pagemap[i] &= 255 - (1<<(7-offset));
}

bool PageMap::bit_set(uint32_t index) {
  uint32_t offset = index % 8;
  uint32_t i = index / 8;

  return pagemap[i] & 1<<(7-offset);
}

void PageMap::mark_unavailable(uint32_t address) {
  unset_bit(address >> 12);
}

void PageMap::mark_unavailable(uint32_t address, uint32_t count) {
  for (int i=0; i<count; i++)
    unset_bit((address >> 12) + i);
}

void PageMap::mark_available(uint32_t address) {
  set_bit(address >> 12);
}

void PageMap::mark_available(uint32_t address, uint32_t count) {
  for (int i=0; i<count; i++)
    set_bit((address >> 12) + i);
}

bool PageMap::available(uint32_t address) {
  return bit_set(address >> 12);
}

uint32_t PageMap::find_next_available_from(uint32_t address) {
  while (!available(address)) address += 4096;

  return address;
}

uint32_t PageMap::find_next_available_from(uint32_t address, uint32_t count) {
  while (1) {
    while (!available(address)) address += 4096;

    for (int a=address, i=0; i<count; i++, a+=4096)
      if (!available(a))
        continue;
    
    return address;
  }
}

PageTable::PageTable(uint32_t phys, uint32_t virt) {
  phys_addr = phys;
  virt_addr = virt;

  page_table = (PTE*)virt;
}

PageTable::PageTable(){
  virt_addr = new PTE[2048];
  while ((uint32_t)this->virt_addr & 0xfff || (uint32_t)this->virt_addr % 0x4)
    this->virt_addr++;
  phys_addr = (Global::allocator->virtual_to_physical(virt_addr)) >> 12;

  page_table = (PTE*)virt_addr;
}

PageTable::~PageTable() {
  delete page_table;
}

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

uint32_t PageTable::get_physical_address(uint32_t index) {
  return page_table[index].address << 12;
}

PageDirectory::PageDirectory(PDE* page_directory, uint32_t phys_addr, uint32_t offset) {
  this->page_tables = (PageTable*)this->__page_tables;
  this->page_directory = page_directory;
  this->phys_addr = phys_addr;

  for (int i=0; i<1024; i++) {
    uint32_t table_phys_addr = page_directory[i].getPhysicalPTAddress();
    new (page_tables + i) PageTable(table_phys_addr >> 12, table_phys_addr + offset);
  }
}

PageDirectory::PageDirectory() {
  this->page_tables = (PageTable*)this->__page_tables;
  this->page_directory = new PDE[2048];
  while ((uint32_t)this->page_directory & 0xfff || (uint32_t)this->page_directory % 0x4)
    this->page_directory++;
  memset((uint8_t*)this->page_tables, sizeof(PageTable) * 1024, 0);
  memset((uint8_t*)this->page_directory, sizeof(PDE) * 1024, 0);
  this->phys_addr = Global::allocator->virtual_to_physical(this->page_directory);
}

PageDirectory::~PageDirectory() {
  for (int i=0; i<1024; i++)
    if (page_tables[i].virt_addr)
      page_tables[i].~PageTable();
  delete page_directory;
}

void PageDirectory::map(uint32_t phys, uint32_t virt) {
  split_addr* split = (split_addr*)&virt;

  if (!page_tables[split->pd_index].virt_addr)
    new (page_tables + split->pd_index) PageTable();

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
  asm volatile ("invlpg (%0)" : : "r" (virt) : "memory");
}

void PageDirectory::unmap(uint32_t virt) {
  split_addr* split = (split_addr*)&virt;
  page_tables[split->pd_index].unmap_table(split->pt_index);
  asm volatile ("invlpg (%0)" : : "r" (virt) : "memory");
}

uint32_t PageDirectory::virtual_to_physical(uint32_t virt) {
  split_addr* split = (split_addr*)&virt;
  return page_tables[split->pd_index].get_physical_address(split->pt_index) + split->page_offset;
}

void PageDirectory::select() {
  asm volatile("mov %0, %%cr3" : : "r" (phys_addr));
}

PageMap* Allocator::phys;

Allocator::Allocator(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base) {
  this->PD = page_directory;
  this->phys = phys;
  this->virt = virt;

  this->virt_alloc_base = virt_alloc_base;
}

Allocator::Allocator(PageDirectory* page_directory, PageMap* virt, uint32_t virt_alloc_base) {
  this->PD = page_directory;
  this->virt = virt;

  this->virt_alloc_base = virt_alloc_base;
}

Allocator::~Allocator() {
  delete virt;
  delete PD;
}

void* Allocator::allocate(uint32_t size) {
  uint32_t count = (size + (4096 - size % 4096)) / 4096;

  uint32_t virt_addr = virt->find_next_available_from(this->virt_alloc_base, count);
  this->virt->mark_unavailable(virt_addr, count);

  for (int i=0; i<count; i++) {
    uint32_t phys_addr = this->phys->find_next_available_from(0);
    this->phys->mark_unavailable(phys_addr);
    this->PD->map(phys_addr, virt_addr + 4096 * i);
  }

  return virt_addr;
}

void Allocator::deallocate(uint32_t virt_addr) {
  uint32_t phys_addr = PD->virtual_to_physical(virt_addr);

  PD->unmap(virt_addr);

  phys->mark_available(phys_addr);
  virt->mark_available(virt_addr);
}

uint32_t Allocator::virtual_to_physical(uint32_t virt) {
  return PD->virtual_to_physical(virt);
}