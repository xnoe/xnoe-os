#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "memory.h"
#include "datatypes/linkedlist.h"
#include "datatypes/hashtable.h"
#include "datatypes/maybe.h"
#include "screenstuff.h"
#include "global.h"

struct AllocTracker {
  void* page_base;
  uint32_t page_size;
  uint32_t alloc_count;

  AllocTracker(void* base, uint32_t size, uint32_t count);
};

class Process : public Allocator {
private:
  uint32_t PID;

  uint32_t last_page_pointer;
  uint32_t page_remaining;

  void* stack;

  // List of pages this process has allocated
  xnoe::linkedlist<AllocTracker> allocations;

  xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> get_alloc_tracker(uint32_t address);

public:
  Process(uint32_t PID, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);

  void* allocate(uint32_t size) override;
  void deallocate(uint32_t virt_addr) override;

  uint32_t count_allocations(uint32_t address);
};

class Kernel : public Process {
public:
  static Allocator* global_allocator;

  Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);
};

#endif