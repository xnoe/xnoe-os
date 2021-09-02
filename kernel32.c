#include <stdarg.h>
#include "types.h"

uint16_t* VMEM_ADDR = (uint16_t*)0xb8000;
const int TERM_WIDTH = 80;
const int TERM_HEIGHT = 25;

int cursor_x = 0;
int cursor_y = 0;

void outb(uint16_t portnumber, uint8_t data) {
  asm volatile("outb %0, %1" : : "a" (data), "Nd" (portnumber));
}
uint8_t inb(uint16_t portnumber) {
  uint8_t result;
  asm volatile("inb %1, %0" : "=a" (result) : "Nd" (portnumber));
  return result;
}

uint16_t get_curpos() {
  uint16_t cursor_position = 0;
  uint8_t* cursor_position_split = (uint8_t*)&cursor_position;
  outb(0x3D4, 0x0F);
  cursor_position_split[0] = inb(0x3D5);
  outb(0x3D4, 0x0E);
  cursor_position_split[1] = inb(0x3D5);
  return cursor_position;
}

void init_term() {
  uint16_t cursor_position = get_curpos();

  cursor_y = cursor_position / TERM_WIDTH;
  cursor_x = cursor_position % TERM_WIDTH;
}

void clear_screen()  {
  for (int i=0; i<TERM_WIDTH*TERM_HEIGHT; i++) {
    VMEM_ADDR[i] = 0x0720;
  }
}

void clear_line(int line) {
  for (int x=0; x<TERM_WIDTH; x++) {
    VMEM_ADDR[TERM_WIDTH*line + x] = 0x0720;
  }
}

void set_curpos_raw(int curpos) {
  uint8_t* cursor_position_split = (uint8_t*)&curpos;
  outb(0x3D4, 0x0F);
  outb(0x3D5, cursor_position_split[0]);
  outb(0x3D4, 0x0E);
  outb(0x3D5, cursor_position_split[1]);
}

void set_curpos(int x, int y) {
  set_curpos_raw(y * TERM_WIDTH + x);
  
  cursor_x = x;
  cursor_y = y;
}

int int_to_decimal(unsigned int number, char* string_buffer) {
  for (int i=0; i<11; i++)
    string_buffer[i] = 0;
  
  int index = 9;
  unsigned int acc = number;
  while (acc != 0) {
    string_buffer[index--] = 0x30+(acc%10);
    acc /= 10;
  }
  return (index+1);
}

char dec_to_hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

int int_to_hex(unsigned int number, char* string_buffer) { 
  for (int i=0; i<9; i++)
    string_buffer[i] = 0;
  
  int index = 7;
  unsigned int acc = number;
  while (acc != 0) {
    string_buffer[index--] = dec_to_hex[acc%0x10];
    acc /= 0x10;
  }
  return (index+1);
}

void printf(const char* string, ...) {
  va_list ptr;
  va_start(ptr, string);

  int index = 0;
  int count = 0;
  char current;
  while (current=string[index++]) {
    count++;
    if (current == '\n') {
      cursor_x = 0;
      cursor_y++;
    }

    if (cursor_x == TERM_WIDTH) {
      cursor_x = 0;
      cursor_y++;
    }

    if (cursor_y == TERM_HEIGHT) {
      for (int i=1; i<TERM_HEIGHT; i++) {
        for (int x=0; x<TERM_WIDTH; x++) {
          int from = i * TERM_WIDTH + x;
          int to = (i-1) * TERM_WIDTH + x;
          VMEM_ADDR[to] = VMEM_ADDR[from];
        }
      }
      clear_line(24);
      cursor_y--;
    }

    if (current == '%') {
      int type = string[index++];
      int offset;
      switch (type) {
        case 'd':
          char decimal_buffer[11];
          offset = int_to_decimal(va_arg(ptr, int), decimal_buffer);
          printf(decimal_buffer + offset);
          break;
        case 'x':
          char hex_buffer[8];
          offset = int_to_hex(va_arg(ptr, int), hex_buffer);
          printf(hex_buffer + offset);
          break;
        case 's':
          printf(va_arg(ptr, const char*));
      }
      continue;
    }

    if (current != '\n') {
      int mem_pos = cursor_y * TERM_WIDTH + cursor_x++;
      VMEM_ADDR[mem_pos] = current + (0x07<<8);
    }
  }

  set_curpos(cursor_x, cursor_y);

  va_end(ptr);
}

int main() {
  init_term();

  printf("KERNEL32 OK!\n");

  printf("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~\n");

  while (1) {}
}