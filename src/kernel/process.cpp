#include "process.h"

Process::Process(uint32_t PID, xnoe::hashtable<void*, xnoe::tuple<uint32_t, uint32_t>>* table, PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
: Allocator(page_directory, phys, virt, virt_alloc_base) {
  this->PID = PID;
  this->allocmap = table;
  this->last_page_pointer = 0;
}

void* Process::allocate(uint32_t size) {

}

void Process::deallocate(uint32_t virt_addr) {

}