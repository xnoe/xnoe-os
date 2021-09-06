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

char* split_on_first(char delimeter, char* string) {
  int index = 0;
  char current;

  while (current = string[index++]) {
    if (current == delimeter) {
      string[index-1] = 0;
      return string+index;
    }
  }

  return 0;
}

int string_split(char delimeter, char* string, char** pointer_array) {
  int index = 0;
  int last_split_index = 0;
  int split_count = 0;

  char current;

  while (current = string[index]) {
    if (current == delimeter) {
      string[index] = 0;
      pointer_array[split_count++] = (string+last_split_index);
      last_split_index = (index+1);
    }
    index++;
  }

  // Add remaining part of the string to the pointer_array
  pointer_array[split_count] = (string+last_split_index);

  return split_count;
}