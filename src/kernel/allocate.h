#ifndef ALLOCATE_H
#define ALLOCATE_H

#include "paging.h"
#include "datatypes/tuple.h"

extern uint8_t* bitmap;
extern PDE* kernel_page_directory;
extern PTE** kernel_page_tables;

void set_bit(uint32_t offset, uint8_t* buffer);
void unset_bit(uint32_t offset, uint8_t* buffer);
uint8_t get_bit(uint32_t offset, uint8_t* buffer);

void mark_unavailble(uint32_t address, uint32_t size);

void* dumb_alloc(uint32_t size);

xnoe::tuple<uint32_t, void*> alloc_page_with_phys();

#endif 