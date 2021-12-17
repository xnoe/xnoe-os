#include "keyboard.h"

char key_to_char[128] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, 
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
  'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+',
  '1', '2', '3', '0', '.', 0, 0, 0, 0, 0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char key_to_char_caps[128] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0, 
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0,
  0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '\\',
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+',
  '1', '2', '3', '0', '.', 0, 0, 0, 0, 0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

char key_to_char_shift[128] = {
  0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 0, 
  'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
  0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
  'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+',
  '1', '2', '3', '0', '.', 0, 0, 0, 0, 0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

uint8_t current_scancode = 0;
char decoded = 0;

bool caps_on = false;
bool shift_on = false;

void keyboard_interrupt(frame_struct* frame) {
  current_scancode = inb(0x60);
  outb(0x20, 0x21);
  if ((current_scancode&0x7f) == 0x2a)
    shift_on = !(current_scancode&0x80);
  
  if (current_scancode == 0x3a)
    caps_on ^= 1;

  if (shift_on)
    decoded = key_to_char_shift[current_scancode&0x7f];
  else if (caps_on)
    decoded = key_to_char_caps[current_scancode&0x7f];
  else
    decoded = key_to_char[current_scancode&0x7f];
}

void init_keyboard() {
  //set_entry(0x21, 0x08, &keyboard_interrupt, 0x8E);
  gates[0x21] = &keyboard_interrupt;

  while (inb(0x64) & 1) {
    inb(0x60);
  }
  
  outb(0x64, 0xAE);
  outb(0x64, 0x20);
  uint8_t keyboard_status = (inb(0x60) | 1) & ~0x10;
  outb(0x64, 0x60);
  outb(0x60, keyboard_status);
  outb(0x60, 0xF4);
}