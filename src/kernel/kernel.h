#ifndef KERNEL_H
#define KERNEL_H

#include "process.h"

class Kernel : public Process {
private:
  static Kernel* kernel;

public:

  Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);
};

#endif