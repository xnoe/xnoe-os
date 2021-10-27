#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "memory.h"

namespace xnoe {
  template<typename T>
  struct linkedlistelem {
    T elem;
    linkedlistelem<T>* previous;
    linkedlistelem<T>* next;

    linkedlistelem(T t, linkedlistelem<T>* p, linkedlistelem<T>* n) {
      this->elem = t;
      this->previous = p;
      this->next = n;
    }
  };

  template<typename T>
  struct linkedlist {
    linkedlistelem<T>* start;
    linkedlistelem<T>* end;

    void append(linkedlistelem<T>* t) {
      this->end->next = t;
      t->prev = this->end;
      t->next = 0;
      this->end = t;
    }

    void prepend(linkedlistelem<T>* t) {
      this->start->previous = t;
      this->start = t;
    }

    void insert(linkedlist<T>* ll, uint32_t index) {
      linkedlistelem<T>* current = this->start;
      for (int i=0; i<index; i++, current = current->next);

      current->next->prev = ll->end;
      current->next = ll->start;
    }

    void remove(Allocator* allocator, uint32_t index) {
      linkedlistelem<T>* current = this->start;
      for (int i=0; i<index; i++, current = current->next);

      current->prev->next = current->next;
      current->next->prev = current->prev;

      delete(allocator, current);
    }
  };
}

#endif