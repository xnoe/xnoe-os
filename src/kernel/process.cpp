#include "process.h"

Process::Process(uint32_t PID, xnoe::hashtable<void*, uint32_t>* table, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
: Allocator(page_directory, phys, virt, virt_alloc_base) {
  this->PID = PID;
  this->allocmap = table;
  this->page_remaining = 0;
  this->last_page_pointer = virt_alloc_base;
}

void* Process::allocate(uint32_t size) {
  uint32_t elem_size = sizeof(xnoe::linkedlistelem<xnoe::tuple<uint32_t, uint32_t>>);
  printf("Elem size: %d\n", elem_size);
  uint32_t actualsize = size + elem_size;

  void* ptr = 0;

  if (this->page_remaining > actualsize) {
    ptr = last_page_pointer + (4096 - page_remaining);
    page_remaining -= actualsize;
    printf("Non-allocating return\nptr: %x\nremaining: %d\n", ptr, page_remaining);
  } else if (actualsize % 4096 == 0) {
    ptr = Allocator::allocate(actualsize);
    page_remaining = 0;
    printf("Whole allocating return\nptr: %x\n", ptr);
  } else {
    uint32_t remaining = 4096 - (actualsize % 4096);
    uint32_t fpindex = actualsize >> 12;

    void* alloc_base = Allocator::allocate(actualsize);
    ptr = last_page_pointer + (4096 - page_remaining);
    last_page_pointer = alloc_base + (fpindex<<12);
    page_remaining = remaining;
    printf("Partially allocating return\nptr: %x\nremaining: %d\n", ptr, page_remaining);
  }
  
  *(xnoe::linkedlistelem<xnoe::tuple<uint32_t, uint32_t>>*)ptr = xnoe::linkedlistelem<xnoe::tuple<uint32_t, uint32_t>>(xnoe::tuple<uint32_t, uint32_t>((uint32_t)ptr, actualsize));

  return ptr + elem_size;
}

void Process::deallocate(uint32_t virt_addr) {

}