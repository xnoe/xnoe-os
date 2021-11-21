#include "kernel.h"

Kernel::Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
  : Process(0, 0x8a000, page_directory, phys, virt, virt_alloc_base)
{
  this->currentPID = 1;
  Global::allocator = this;
  Global::kernel = this;
}

void Kernel::init_kernel() {
  this->pid_map = new xnoe::hashtable<uint32_t, Process*>();
}

Process* Kernel::createProcess() {
  Process* p = new Process(currentPID);
  this->pid_map->set(currentPID, p);
  currentPID++;
  return p;
}