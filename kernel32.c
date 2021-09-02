#include "types.h"

uint8_t* VMEM_ADDR = (uint8_t*)0xb8000;
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

uint16_t get_cursor_position() {
  uint16_t cursor_position = 0;
  uint8_t* cursor_position_split = (uint8_t*)&cursor_position;
  outb(0x3D4, 0x0F);
  cursor_position_split[0] = inb(0x3D5);
  outb(0x3D4, 0x0E);
  cursor_position_split[1] = inb(0x3D5);
  return cursor_position;
}

void init_term() {
  uint16_t cursor_position = get_cursor_position();

  cursor_y = cursor_position / TERM_WIDTH;
  cursor_x = cursor_position % TERM_WIDTH;
}

void clear_screen()  {
  for (int i=0; i<TERM_WIDTH*TERM_HEIGHT; i++) {
    VMEM_ADDR[i*2] = 0x20;
    VMEM_ADDR[i*2 + 1] = 0x07;
  }
}

void set_curpos(int x, int y) {
  cursor_x = x;
  cursor_y = y;
}

void print(const char* string) {
  int index = 0;
  char current;
  while (current=string[index++]) {
    if (current == '\n') {
      set_curpos(0, cursor_y+1);
      continue;
    }

    int mem_pos = cursor_y * TERM_WIDTH + cursor_x++;
    VMEM_ADDR[mem_pos*2] = current;
    VMEM_ADDR[mem_pos*2 + 1] = 0x07;

    if (index == TERM_WIDTH) {
      cursor_x = 0;
      cursor_y++;
    }
  }
}


int main() {
  init_term();

  print("KERNEL32 OK!\n\n");

  print("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~");

  while (1) {}
}