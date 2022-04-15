#include "kernel.h"

Kernel::Kernel(PageDirectory* page_directory, PageMap* phys, PageMap* virt, uint32_t virt_alloc_base, uint32_t stack)
  : Process(0, 0x8a000, page_directory, phys, virt, virt_alloc_base)
{
  this->currentPID = 1;
  Global::allocator = this;
  Global::kernel = this;

  Global::currentProc = 0;

  this->stack = stack;

  this->lastFH = 8;
}

void Kernel::init_kernel() {
  this->pid_map = new xnoe::hashtable<uint32_t, Process*>();
  Global::FH = new xnoe::hashtable<void*, ReadWriter*>();
  this->globalISRStack = (new uint8_t[0x8000]) + 0x8000;
}

Process* Kernel::createProcess(uint32_t fh) {
  Process* p = new Process(currentPID, this->PD, 0xc0000000, fh);
  this->pid_map->set(currentPID, p);
  currentPID++;

  this->processes.append(p);

  return p;
}

Process* Kernel::createProcess(uint32_t fh, ReadWriter* stdout) {
  Process* p = this->createProcess(fh);
  p->stdout = stdout;
  return p;
}

void Kernel::destroyProcess(Process* p) {
  if (Global::currentProc == p)
    Global::currentProcValid = false;
  this->processes.remove(p);
  this->pid_map->remove(p->PID);
  delete p;
}

int Kernel::mapFH(ReadWriter* fh) {
  Global::FH->set(this->lastFH++, fh);
  return this->lastFH - 1;
}

void Kernel::unmapFH(uint32_t fh) {
  Global::FH->remove((void*)fh);
}