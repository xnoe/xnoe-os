#ifndef TUPLE_H
#define TUPLE_H

#include "../types.h"

namespace xnoe {
  template<int index, typename T>
  class tupleGetHelper;

  template<typename ... T>
  class tuple {};

  template<typename T, typename ... Tail>
  class tuple<T, Tail ...> {
  public:
    T head;
    tuple<Tail...> tail;

    tuple(const T& head, const Tail& ... tail)
      : head(head), tail(tail...) {}

    template<int index>
    auto get() {
      return tupleGetHelper<index, tuple<T, Tail...>>::get(*this);
    }
  };

  template<typename T, typename ... Tail>
  class tupleGetHelper<0, tuple<T, Tail...>> {
  public:
    static T get(tuple<T, Tail...>& t) {
      return t.head;
    }
  };

  template<int index, typename T, typename ... Tail>
  class tupleGetHelper<index, tuple<T, Tail...>> {
  public:
    static auto get(tuple<T, Tail...>& t) {
      return tupleGetHelper<index - 1, tuple<Tail ...>>::get(t.tail);
    }
  };
}

#endif