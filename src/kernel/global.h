#ifndef GLOBAL_H
#define GLOBAL_H

#include "memory.h"

class Kernel;
class Allocator;
class Process;
struct tss_struct;
class ReadWriter;
namespace xnoe {
  template<class, class>
  class hashtable;
}

namespace Global {
  extern Allocator* allocator;
  extern Kernel* kernel;
  extern Process* currentProc;
  extern tss_struct* tss;
  extern bool currentProcValid;
  extern xnoe::hashtable<void*, ReadWriter*>* FH;
}

void* operator new (uint32_t size);
void* operator new (uint32_t size, void* ptr);
void operator delete (void* ptr);
void operator delete (void* ptr, unsigned int size);

void* operator new[] (uint32_t size);
void operator delete[] (void* ptr);

#endif