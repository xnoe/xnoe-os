#include "../common/common.h"

int main() {
  char buffer[2];
  buffer[0] = 0;
  buffer[1] = 0;

  while (1) {
    buffer[0] = getch();
    print(buffer);
  }
}