#ifndef KERNEL_H
#define KERNEL_H

#include "process.h"
#include "datatypes/hashtable.h"
#include "global.h"

class Kernel : public Process {
public:
  uint32_t currentPID;

  xnoe::hashtable<uint32_t, Process*>* pid_map; // Map of PIDs -> Process*s

  xnoe::linkedlist<Process*> processes;

  Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base);

  void init_kernel();

  Process* createProcess();
};

#endif