#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "memory.h"
#include "datatypes/linkedlist.h"
#include "datatypes/hashtable.h"

class Process : protected Allocator {
private:
  uint32_t PID;

  uint32_t last_page_pointer;
  uint32_t page_usage;

  xnoe::linkedlist<xnoe::tuple<uint32_t, uint32_t>> allocations;
  xnoe::hashtable<void*, xnoe::tuple<uint32_t, uint32_t>>* allocmap;

public:
  Process(uint32_t PID, xnoe::hashtable<void*, xnoe::tuple<uint32_t, uint32_t>>* table, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);

  void* allocate(uint32_t size) override;
  void deallocate(uint32_t virt_addr) override;
};

class Kernel : private Process {

};

#endif