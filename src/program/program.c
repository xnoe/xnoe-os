#include "../kernel/types.h"

void print(char* string) {
  asm volatile ("mov $0, %%eax; mov %0, %%esi; int $0x7f" : : "m" (string) : "eax", "esi");
}

char getch() {
  asm volatile ("mov $1, %%eax; int $0x7f" : : :);
}

uint8_t getchPS2() {
  asm volatile ("mov $2, %%eax; int $0x7f" : : :);
}

void readfile(char* filename, uint8_t* buffer) {
  asm volatile ("mov $3, %%eax; mov %0, %%esi; mov %1, %%edi; int $0x7f" : : "m" (filename), "m" (buffer) : "eax", "esi", "edi");
}

void* localalloc(uint32_t size) {
  asm volatile ("mov $4, %%eax; mov %0, %%esi; int $0x7f" : : "m" (size) : "esi");
}

void localdelete(void* ptr) {
  asm volatile ("mov $5, %%eax; mov %0, %%esi; int $0x7f" : : "m" (ptr) : "esi");
}

uint32_t filesize(char* filename) {
  asm volatile ("mov $6, %%eax; mov %0, %%esi; int $0x7f" : : "m" (filename) : "esi");
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

int main() {
  print("Testing C code program\n");
  print("My strings are messed up for some reason...\n");

  uint32_t alpha_size = filesize("HELLO   TXT");
  char sizebuf[32];
  uint32_t index = int_to_decimal(alpha_size, sizebuf);
  print(sizebuf+index);
  print("\n");
  uint8_t* alpha_buffer = (uint8_t*)localalloc(alpha_size + 32);
  print("alpha_buffer: ");
  index = int_to_hex(alpha_buffer, sizebuf);
  print(sizebuf+index);
  print("\n");
  readfile("HELLO   TXT", alpha_buffer);
  print(alpha_buffer);
  while (1);
}