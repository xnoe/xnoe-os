#ifndef HASH_H
#define HASH_H

#include "../types.h"

namespace xnoe {
  template<class T>
  uint32_t hash(T t);

  template<>
  uint32_t hash<void*>(void* t);
  template<>
  uint32_t hash<uint32_t>(uint32_t t);
}

#endif