#ifndef SCREENSTUFF_H
#define SCREENSTUFF_H

#include <stdarg.h>
#include "types.h"
#include "io.h"

uint16_t get_curpos();
void init_term();
void clear_screen();
void clear_line(int line);
void set_curpos_raw(int curpos);
void set_curpos(int x, int y);
int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);
void printf(const char* string, ...);

#endif