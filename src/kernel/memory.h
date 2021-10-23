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
  uint8_t pagemap[1024^2 / 8];

  void set_bit(uint32_t index);
  void unset_bit(uint32_t index);

public:
  PageMap(uint8_t* map);

  void mark_unavailble(uint32_t page);
  void mark_available(uint32_t page);
};

struct PageTable {
  PTE* page_table;

  uint32_t phys_addr;
  uint32_t virt_addr;

  PageTable(uint32_t phys, uint32_t virt);
  PageTable();

  void map_table(uint32_t index, uint32_t addr);
  void unmap_table(uint32_t index);
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

  void select();
};

class Allocator {
protected:
  static PageMap phys;
  PageMap virt;

  PageDirectory PD;

  uint32_t current_page_index;
  uint32_t remaining;
public:
  Allocator();
  Allocator(PageDirectory page_directory, PageMap phys, PageMap virt);
  void* allocate();
  void deallocate();
};

class Process : protected Allocator {
private:
  uint32_t PID;

public:
  Process(uint32_t PID);
};

#endif