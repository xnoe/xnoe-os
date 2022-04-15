#ifndef SPINLOCK_H
#define SPINLOCK_H

#include "types.h"

class Spinlock {
private:
  uint32_t locked = 0;
public:
  void lock();
  void unlock();
};

#endif