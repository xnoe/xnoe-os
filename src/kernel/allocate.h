#ifndef ALLOCATE_H
#define ALLOCATE_H

#include "paging.h"
//void init_allocator();

void set_bit(uint32_t offset, uint8_t* buffer);
void unset_bit(uint32_t offset, uint8_t* buffer);
uint8_t get_bit(uint32_t offset, uint8_t* buffer);

void mark_unavailble(uint32_t address, uint32_t size);

void* dumb_alloc(uint32_t size);

#endif 