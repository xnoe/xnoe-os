#include "global.h"

namespace Global {
  Allocator* allocator = 0;
  Kernel* kernel = 0;
  Process* currentProc = 0;
  tss_struct* tss = 0;
  bool currentProcValid = false;
  xnoe::hashtable<void*, ReadWriter*>* FH; // Map of File Handlers -> Read Writer
}

void* operator new (uint32_t size) {
  return Global::allocator->allocate(size);
}

void* operator new (uint32_t size, void* ptr) {
  return ptr;
}

void operator delete (void* ptr) {
  Global::allocator->deallocate((uint32_t)ptr);
}

void operator delete (void* ptr, unsigned int size) {
  Global::allocator->deallocate((uint32_t)ptr);
}

void* operator new[] (uint32_t size) {
  return Global::allocator->allocate(size);
}

void operator delete[] (void* ptr) {
  Global::allocator->deallocate((uint32_t)ptr);
}