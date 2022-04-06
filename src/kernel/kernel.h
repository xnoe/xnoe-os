#ifndef KERNEL_H
#define KERNEL_H

#include "process.h"
#include "datatypes/hashtable.h"
#include "global.h"
#include "terminal.h"
#include "filesystem/fstree.h"

class Kernel : public Process {
private:
  int lastFH;
public:
  uint32_t currentPID;
  uint32_t stack;
  uint32_t globalISRStack;
  Terminal* terminal;

  xnoe::hashtable<uint32_t, Process*>* pid_map; // Map of PIDs -> Process*s

  xnoe::linkedlist<Process*> processes;
  xnoe::linkedlist<Process*> KBListeners;

  RootFSTree* rootfs;

  Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base, uint32_t stack);

  void init_kernel();

  Process* createProcess(uint32_t fh);
  Process* createProcess(uint32_t fh, ReadWriter* stdout);
  void destroyProcess(Process* p);

  int mapFH(ReadWriter* fh);
  void unmapFH(uint32_t fh);
  //void loadPrimaryStack();
};

#endif