#include "hash.h"

namespace xnoe {
  template<class T>
  uint32_t hash(T t){}

  template<>
  uint32_t hash<void*>(void* t) {
    return (uint32_t) t;
  }

  template<>
  uint32_t hash<uint32_t>(uint32_t t) {
    return t;
  }
}