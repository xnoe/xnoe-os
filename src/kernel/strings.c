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

void decode_filename(char* nice_name, char* filenamebuffer) {
  // Clear filenamebuffer
  for (int i=0; i<11; i++)
    filenamebuffer[i] = ' ';
  filenamebuffer[11] = 0;
  
  int fbIndex = 0;
  for (int i=0; i<12; i++) {
    if (nice_name[i] == 0)
      return;
    if (nice_name[i] == '.') {
      fbIndex = 8;
      continue;
    }

    if (nice_name[i] >= 0x61 && nice_name[i] <= 0x7f)
      filenamebuffer[fbIndex++] = nice_name[i] - 32;
    else 
      filenamebuffer[fbIndex++] = nice_name[i];
  }
}