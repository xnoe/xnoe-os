#include "common.h"

#define syscall_hdlr_0(a, b, c) \
  a b() { \
    asm volatile("mov $" c ", %%eax; int $0x80" : : :); \
  }
#define syscall_hdlr_1(a, b, c, d, e) \
  a b(d e) { \
    asm volatile("mov $" c ", %%eax; mov %0, %%ebx; int $0x80" : : "m" (e) : "ebx"); \
  }
#define syscall_hdlr_2(a, b, c, d, e, f, g) \
  a b(d e, f g) { \
    asm volatile("mov $" c ", %%eax; mov %0, %%ebx; mov %1, %%ecx; int $0x80" : : "m" (e), "m" (g) : "ebx", "ecx"); \
  }
#define syscall_hdlr_3(a, b, c, d, e, f, g, h, i) \
  a b(d e, f g, h i) { \
    asm volatile("mov $" c ", %%eax; mov %0, %%ebx; mov %1, %%ecx; mov %2, %%edx; int $0x80" : : "m" (e), "m" (g), "m" (i) : "ebx", "ecx", "edx"); \
  }

#include "syscalls.h"

void print(char* string) {
  char* c = string;
  int i=0;
  while (*(c++))
    i++;
  write(i, 0, (uint8_t*)string);
}

int int_to_decimal(unsigned int number, char* string_buffer) {
  for (int i=0; i<11; i++)
    string_buffer[i] = 0;
  
  int index = 9;
  unsigned int acc = number;
  if (acc == 0)
    string_buffer[index--] = '0';
  while (acc != 0) {
    string_buffer[index--] = 0x30+(acc%10);
    acc /= 10;
  }
  return (index+1);
}

char dec_to_hex[16] = "0123456789abcdef";
int int_to_hex(unsigned int number, char* string_buffer) { 
  for (int i=0; i<8; i++)
    string_buffer[i] = '0';
  string_buffer[8] = 0;
  
  int index = 7;
  unsigned int acc = number;
  if (acc == 0)
    string_buffer[index--] = '0';
  while (acc != 0) {
    string_buffer[index--] = dec_to_hex[acc%0x10];
    acc /= 0x10;
  }
  return (index+1);
}