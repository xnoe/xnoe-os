#include "../common/common.h"

int main() {
  uint32_t counter = 0;
  uint32_t PID = getPID();
  char intbuffer[32];
  uint32_t index = int_to_decimal(PID, intbuffer);
  while (1) {
    counter++;
    if (counter == 312500) {
      print(intbuffer+index);
      print(" ");
      counter = 0;
    }
  }
}