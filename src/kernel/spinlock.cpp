#include "spinlock.h"

void Spinlock::lock() {
  asm volatile ("spin_lock: mov $1, %%eax; lock xchg %0, %%eax; test %%eax, %%eax; jnz spin_lock" :: "m"(locked)  : "eax");
}
void Spinlock::unlock() {
  asm volatile ("xor %%eax, %%eax; lock xchg %0, %%eax" :: "m"(locked) : "eax");
}