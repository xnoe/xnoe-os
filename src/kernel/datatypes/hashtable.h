#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "hash.h"
#include "linkedlist.h"
#include "../memory.h"
#include "maybe.h"

namespace xnoe {
  template <class key, class value>
  class hashtable {
  private:
    xnoe::linkedlist<xnoe::tuple<key, value>>* table;

  public:
    hashtable() {
      this->table = new xnoe::linkedlist<xnoe::tuple<key, value>>[4096];
    }

    void set(key k, value v) {
      xnoe::linkedlist<xnoe::tuple<key, value>> list = table[xnoe::hash<key>(k) % 4096];
      xnoe::linkedlistelem<xnoe::tuple<key, value>>* current = list.start;

      bool exists = false;

      if (current) {
        while (current->next) {
          if (xnoe::get<0>(current->elem) == k) {
            exists = true;
            break;
          }
        }
      }
      
      if (exists)
        current->elem = xnoe::tuple<key, value>(k, v);
      else
        list.append(xnoe::tuple<key, value>(k, v));
    }

    xnoe::Maybe<value> get(key k) {
      xnoe::linkedlist<xnoe::tuple<key, value>> list = table[xnoe::hash<key>(k) % 4096];
      xnoe::linkedlistelem<xnoe::tuple<key, value>>* current = list.start;
      if (current)
        while (current->next)
          if (xnoe::get<0>(current->elem) == k)
            return xnoe::Maybe<value>(xnoe::get<1>(current->elem));
      
      return xnoe::Maybe<value>();
    }
  };
}

#endif