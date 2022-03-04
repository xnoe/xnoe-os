#ifndef IO_H
#define IO_H

#include "types.h"

void outb(uint16_t portnumber, uint8_t data);
uint8_t inb(uint16_t portnumber);

void outw(uint16_t portnumber, uint16_t data);
uint16_t inw(uint16_t portnumber);

#endif