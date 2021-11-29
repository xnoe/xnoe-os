#include "../common/common.h"
#include <stdbool.h>

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

bool caps_on = false;
bool shift_on = false;

void readline(int max, char* buffer) {
  int index = 0;
  uint8_t current_scancode = getchPS2();

  uint8_t char_print[2];
  char_print[0] = 0;
  char_print[1] = 0;

  uint8_t scancode = 0;
  while (scancode != 0x1c && index < max) {
    scancode = current_scancode;

    char decoded = 0;

    if ((scancode&0x7f) == 0x2a)
      shift_on = !(scancode&0x80);
    
    if (scancode == 0x3a)
      caps_on ^= 1;

    /*if (scancode == 0x0e && index > 0) {
      set_curpos_raw(get_curpos()-1);
      non_moving_put(' ');
      buffer[--index] = 0;
    }*/

    if (shift_on)
      decoded = key_to_char_shift[scancode&0x7f];
    else if (caps_on)
      decoded = key_to_char_caps[scancode&0x7f];
    else
      decoded = key_to_char[scancode&0x7f];

    if (decoded && scancode < 0x80) {
      buffer[index++] = decoded;
      char_print[0] = decoded;
      print(char_print);
    }

    while (scancode == current_scancode)
      current_scancode = getchPS2();
  }
  print("\n");
}

int main() {
  char buffer[128];
  while (1) {
    for (int i=0; i<128; i++)
      buffer[i] = 0;
    print(">>> ");
    readline(128, buffer);
    print("\nYou said: ");
    print(buffer);
    print("\n");
  }
}