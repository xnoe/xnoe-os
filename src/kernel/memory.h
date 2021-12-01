#ifndef MEMORY_H
#define MEMORY_H

#include "paging.h"
#include "allocate.h"
#include "screenstuff.h"
#include "global.h"

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
  PageMap();

  ~PageMap();

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

  ~PageTable(); // Delete page_table

  void map_table(uint32_t index, uint32_t addr);
  void unmap_table(uint32_t index);

  uint32_t get_physical_address(uint32_t index);
};

class PageDirectory {
private:
  uint8_t __page_tables[sizeof(PageTable) * 1024];
  PageTable* page_tables;

public:
  uint32_t phys_addr;

  PDE* page_directory;
  
  PageDirectory(PDE* page_directories, uint32_t phys_addr, uint32_t offset);
  PageDirectory();

  ~PageDirectory(); // Delete the page tables and page_directory

  void map(uint32_t phys, uint32_t virt);
  void unmap(uint32_t virt);

  uint32_t virtual_to_physical(uint32_t virt);

  void select();
};

class Allocator {
protected:
  static PageMap* phys;
  PageMap* virt;

  uint32_t virt_alloc_base;
public:
  PageDirectory* PD;

  Allocator(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);
  Allocator(PageDirectory* page_directory, PageMap* virt, uint32_t virt_alloc_base);

  ~Allocator(); // Delete virt and PD

  virtual void* allocate(uint32_t size);
  virtual void deallocate(uint32_t virt_addr);

  uint32_t virtual_to_physical(uint32_t virt);
};

#endif