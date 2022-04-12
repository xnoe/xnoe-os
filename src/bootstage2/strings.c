#include "strings.h"

bool strcmp(char* a, char* b, int max) {
  int index = 0;
  while (index < max) {
    if (a[index] != b[index])
      return false;
    index++;
  }
  return true;
}