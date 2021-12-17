#ifndef KERNEL_H
#define KERNEL_H

#include "process.h"
#include "datatypes/hashtable.h"
#include "global.h"
#include "terminal.h"

class Kernel : public Process {
public:
  uint32_t currentPID;
  uint32_t stack;
  Terminal* terminal;

  xnoe::hashtable<uint32_t, Process*>* pid_map; // Map of PIDs -> Process*s

  xnoe::linkedlist<Process*> processes;

  Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base, uint32_t stack);

  void init_kernel();

  Process* createProcess(char* filename);
  void destroyProcess(Process* p);
  //void loadPrimaryStack();
};

#endif