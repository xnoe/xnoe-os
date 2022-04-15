#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "../types.h"
#include "../memory.h"
#include "../spinlock.h"

namespace xnoe {
  template<typename T>
  struct linkedlistelem {
    T elem;
    linkedlistelem<T>* prev;
    linkedlistelem<T>* next;

    linkedlistelem(T t) {
      this->elem = t;
      this->prev = 0;
      this->next = 0;
    }
  };

  template<typename T>
  struct linkedlist {
    xnoe::linkedlistelem<T>* start=0;
    xnoe::linkedlistelem<T>* end=0;
    Spinlock lock = Spinlock();

    bool has(T t) {
      xnoe::linkedlistelem<T>* current = this->start;
      while (start) {
        if (start->elem == t)
          return true;
        //current = current->next;
      }

      return false;
    }

    void destroy() {
      xnoe::linkedlistelem<T>* current = this->start;
      while (current) {
        xnoe::linkedlistelem<T>* c = current;
        current = current->next;
        delete c;
      }
    }

    void append(T t) {
      xnoe::linkedlistelem<T>* llelem = new xnoe::linkedlistelem<T>(t);
      append(llelem);
    }

    void append(xnoe::linkedlistelem<T>* llelem) {
      lock.lock();
      if (this->start && this->end) {
        this->end->next = llelem;
        llelem->prev = this->end;
        llelem->next = 0;
        this->end = llelem;
      } else {
        this->start = llelem;
        this->end = llelem;
      }
      lock.unlock();
    }

    void prepend(T t) {
      xnoe::linkedlistelem<T>* llelem = new xnoe::linkedlistelem<T>(t);
      prepend(llelem);
    }

    void prepend(xnoe::linkedlistelem<T>* llelem) {
      lock.lock();
      if (this->start && this->end) {
        this->start->prev = llelem;
        llelem->next = this->start;
        llelem->prev = 0;
        this->end = llelem;
      } else {
        this->start = llelem;
        this->end = llelem;
      }
      lock.unlock();
    }

    void insert(linkedlist<T>* ll, uint32_t index) {
      lock.lock();
      linkedlistelem<T>* current = this->start;
      for (int i=0; i<index; i++, current = current->next);

      current->next->prev = ll->end;
      current->next = ll->start;
      lock.unlock();
    }

    /*void remove(uint32_t index) {
      linkedlistelem<T>* current = this->start;
      for (int i=0; i<index; i++, current = current->next);

      current->prev->next = current->next;
      current->next->prev = current->prev;

      delete current;
    }*/

    void remove(linkedlistelem<T>* elem) {
      lock.lock();
      linkedlistelem<T>* current = start;
      while (current) {
        if (current == elem) {
          if (current->prev)
            current->prev->next = current->next;
          
          if (current->next)
            current->next->prev = current->prev;

          if (current == start)
            start = current->next;

          if (current = end)
            end = current->prev;

          lock.unlock();
          return;
        }
        current = current->next;
      }
      lock.unlock();
    }

    void remove(T elem) {
      lock.lock();
      linkedlistelem<T>* current = start;
      while (current) {
        if (current->elem == elem) {
          if (current->prev)
            current->prev->next = current->next;
          
          if (current->next)
            current->next->prev = current->prev;

          if (current == start)
            start = current->next;

          if (current == end)
            end = current->prev;
          
          delete current;

          lock.unlock();
          return;
        }
        current = current->next;
      }
      lock.unlock();
    }
  };
}

#endif