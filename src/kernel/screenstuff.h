#ifndef SCREENSTUFF_H
#define SCREENSTUFF_H

#include <stdarg.h>
#include "types.h"
#include "io.h"
#include "strings.h"

uint16_t get_curpos();
void init_term();
void clear_screen();
void clear_line(int line);
void set_curpos_raw(int curpos);
void set_curpos(int x, int y);
void printf(const char* string, ...);
void non_moving_put(char chr);

#endif