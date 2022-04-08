#include "common.h"

void print(char* string) {
  char* c = string;
  int i=0;
  while (*(c++))
    i++;
  write(i, 0, (uint8_t*)string);
}

void* localalloc(uint32_t size) {
  asm volatile ("mov $4, %%eax; mov %0, %%esi; int $0x7f" : : "m" (size) : "esi");
}

void localdelete(void* ptr) {
  asm volatile ("mov $5, %%eax; mov %0, %%esi; int $0x7f" : : "m" (ptr) : "esi");
}

uint32_t getPID() {
  asm volatile ("mov $8, %%eax; int $0x7f" : : :);
}

int read(uint32_t count, void* filehanlder, uint8_t* buffer) {
  asm volatile ("mov $10, %%eax; mov %0, %%ebx; mov %1, %%esi; mov %2, %%edi; int $0x7f" : : "m" (count), "m" (filehanlder), "m" (buffer): "ebx", "esi", "edi");
}

int write(uint32_t count, void* filehanlder, uint8_t* buffer) {
  asm volatile ("mov $11, %%eax; mov %0, %%ebx; mov %1, %%esi; mov %2, %%edi; int $0x7f" : : "m" (count), "m" (filehanlder), "m" (buffer): "ebx", "esi", "edi");
}

uint32_t fork(uint32_t fh) {
  asm volatile("mov $7, %%eax; mov %0, %%esi; int $0x7f" : : "m" (fh) : "esi");
}

uint32_t bindStdout(uint32_t PID) {
  asm volatile("mov $13, %%eax; mov %0, %%esi; int $0x7f" : : "m" (PID) : "esi");
}

uint32_t bindStdin(uint32_t PID) {
  asm volatile("mov $14, %%eax; mov %0, %%esi; int $0x7f" : : "m" (PID) : "esi");
}

int fopen(char* filename) {
  asm volatile("mov $15, %%eax; mov %0, %%esi; int $0x7f" : : "m" (filename) : "esi");
}

void fclose(uint32_t fh) {
  asm volatile("mov $16, %%eax; mov %0, %%esi; int $0x7f" : : "m" (fh) : "esi");
}

void kill(uint32_t pid) {
  asm volatile("mov $17, %%eax; mov %0, %%esi; int $0x7f" : : "m" (pid) : "esi");
}

void sleep(uint32_t time) {
  asm volatile("mov $18, %%eax; mov %0, %%esi; int $0x7f" : : "m" (time) : "esi");
}

void bindToKeyboard() {
  asm volatile ("mov $12, %%eax; int $0x7f" : : :);
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