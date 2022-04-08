#ifndef COMMON_H
#define COMMON_H

#include "../kernel/types.h"

#define syscall_hdlr_0(a, b, c) \
  a b();
#define syscall_hdlr_1(a, b, c, d, e) \
  a b(d e);
#define syscall_hdlr_2(a, b, c, d, e, f, g) \
  a b(d e, f g);
#define syscall_hdlr_3(a, b, c, d, e, f, g, h, i) \
  a b(d e, f g, h i);

#include "syscalls.h"

void print(char* string);
int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);

#endif