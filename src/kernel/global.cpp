#include "global.h"

namespace Global {
  Allocator* allocator = 0;
  Kernel* kernel = 0;
  Process* currentProc = 0;
}

void* operator new (uint32_t size) {
  return Global::allocator->allocate(size);
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