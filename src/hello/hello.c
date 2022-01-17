#include "../common/common.h"

void readline(int count, char* buffer) {
  int index = 0;
  char c;
  while (index < count) {
    if (read(1, 1, &c)) {
      if (c == '\n')
        break;
      if (c == '\b') {
        if (index == 0)
          continue;
        else {
          index--;
          buffer[index] = 0;
          write(1, 0, &c);
          continue;
        }
      }

      buffer[index++] = c;
      write(1, 0, &c);
    }
  }
  print("\n");
}

int main() {
  print("Hello, World!\n");
  char buffer[32];
  while (1) {
    for (int i=0; i<32; i++)
      buffer[i] = 0;
    print(">>> ");
    readline(32, buffer);
    print("You said: ");
    print(buffer);
    print("\n\n");
  }
}