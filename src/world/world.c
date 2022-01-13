#include "../common/common.h"
#include <stdbool.h>

int main() {
  bindToKeyboard();

  print("Hello from Ring 3!\n");

  while (1) {
    char c;
    if (read(1, 1, &c))
      write(1, 0, &c);
  }
}