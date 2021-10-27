#ifndef MEMORY_H
#define MEMORY_H

#include "datatypes/tuple.h"
#include "paging.h"
#include "allocate.h"
#include "screenstuff.h"

void memset(uint8_t* address, uint32_t count, uint8_t value);
void memcpy(uint8_t* src, uint8_t* dst, uint32_t count);

class __attribute__((packed)) PageMap {
private:
  uint8_t* pagemap;

  void set_bit(uint32_t index);
  void unset_bit(uint32_t index);

  bool bit_set(uint32_t index);

public:
  PageMap(uint32_t map);

  void mark_unavailable(uint32_t address);
  void mark_unavailable(uint32_t address, uint32_t count);

  void mark_available(uint32_t address);
  void mark_available(uint32_t address, uint32_t count);

  bool available(uint32_t address);

  uint32_t find_next_available_from(uint32_t address);
  uint32_t find_next_available_from(uint32_t address, uint32_t count);
};

struct PageTable {
  PTE* page_table;

  uint32_t phys_addr;
  uint32_t virt_addr;

  PageTable(uint32_t phys, uint32_t virt);
  PageTable();

  void map_table(uint32_t index, uint32_t addr);
  void unmap_table(uint32_t index);

  uint32_t get_physical_address(uint32_t index);
};

class PageDirectory {
private:
  PDE* page_directory;
  PageTable page_tables[1024];

  uint32_t phys_addr;

public:
  PageDirectory(PDE* page_directories, uint32_t phys_addr, uint32_t offset);

  void map(uint32_t phys, uint32_t virt);
  void unmap(uint32_t virt);

  uint32_t virtual_to_physical(uint32_t virt);

  void select();
};

class Allocator {
protected:
  static PageMap* phys;
  PageMap* virt;

  PageDirectory* PD;

  uint32_t virt_alloc_base;
public:
  Allocator(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);
  virtual void* allocate(uint32_t size);
  virtual void deallocate(uint32_t virt_addr);
};

void* operator new (uint32_t size, Allocator* allocator);
void operator delete (void* ptr, Allocator* allocator);

void* operator new[] (uint32_t size, Allocator* allocator);
void operator delete[] (void* ptr, Allocator* allocator);

#endif