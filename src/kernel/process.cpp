#include "process.h"

AllocTracker::AllocTracker(void* base, uint32_t size, uint32_t count) : page_base(base), page_size(size), alloc_count(count) {}

xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> Process::get_alloc_tracker(uint32_t address) {
  xnoe::linkedlistelem<AllocTracker>* current = this->allocations.start;
  while (current) {
    if (current->elem.page_base < address && (current->elem.page_base + 4096 * current->elem.page_size) > address) {
      return xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*>(current);
    }
    current = current->next;
  }
  
  return xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*>();
}

Process::Process(uint32_t PID, void* stack, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
: Allocator(page_directory, phys, virt, virt_alloc_base) {
  this->PID = PID;
  this->page_remaining = 0;
  this->last_page_pointer = virt_alloc_base;
  this->stack = stack;
}

Process::Process(uint32_t PID)
: Allocator(new PageDirectory, new PageMap, 0) {
  this->PID = PID;
  this->page_remaining = 0;
  this->last_page_pointer = virt_alloc_base;
  this->stack = new uint8_t[0x8000];
}

void* Process::allocate(uint32_t size) {
  void* ptr;
  // Determine if there's enough space to just allocate what's been requested
  if (size < this->page_remaining) {
    xnoe::linkedlistelem<AllocTracker>* alloctracker = this->allocations.end;
    alloctracker->elem.alloc_count += 1;
    ptr = this->last_page_pointer + (4096 - this->page_remaining);
    this->page_remaining -= size;
  } else {
    uint32_t elem_size = sizeof(xnoe::linkedlistelem<AllocTracker>);
    size += elem_size;

    // Determine how many pages we'll allocate, and the remainder;
    uint32_t pages = size / 4096;
    uint32_t remainder = 4096 - (size % 4096);

    ptr = Allocator::allocate(size);

    // Update local values
    this->last_page_pointer = ptr + pages * 4096;
    this->page_remaining = remainder;

    // Create allocations entry
    xnoe::linkedlistelem<AllocTracker>* elem = (xnoe::linkedlistelem<AllocTracker>*)ptr;
    elem->next = 0;
    elem->prev = 0;
    elem->elem = AllocTracker(ptr, pages + 1, 1);
    this->allocations.append(elem);

    ptr += elem_size;
  }
  return ptr;
}

void Process::deallocate(uint32_t virt_addr) {
  xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> alloc_tracker = this->get_alloc_tracker(virt_addr);
  if (alloc_tracker.is_ok()) {
    AllocTracker* ac = &alloc_tracker.get()->elem;
    ac->alloc_count -= 1;
    if (ac->alloc_count == 0) {
      void* base = ac->page_base;
      uint32_t count = ac->page_size;

      this->allocations.remove(alloc_tracker.get());

      for (int i=0; i<count; i++)
        Allocator::deallocate(base + (4096 * i));
      
      this->page_remaining = 0;
    }
  }
}

uint32_t Process::count_allocations(uint32_t address) {
  xnoe::Maybe<xnoe::linkedlistelem<AllocTracker>*> alloc_tracker = this->get_alloc_tracker(address);

  if (alloc_tracker.is_ok())
    return alloc_tracker.get()->elem.alloc_count;
  else
    return 0;
}

