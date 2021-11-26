#ifndef GLOBAL_H
#define GLOBAL_H

#include "memory.h"

class Kernel;
class Allocator;
class Process;

namespace Global {
  extern Allocator* allocator;
  extern Kernel* kernel;
  extern Process* currentProc;
}

void* operator new (uint32_t size);
void operator delete (void* ptr);
void operator delete (void* ptr, unsigned int size);

void* operator new[] (uint32_t size);
void operator delete[] (void* ptr);

#endif