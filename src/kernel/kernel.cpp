#include "kernel.h"

Kernel* Kernel::kernel;

Kernel::Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base)
  : Process(0, 0x8a000, page_directory, phys, virt, virt_alloc_base)
{
  this->kernel = this;
}