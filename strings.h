#ifndef STRINGS_H
#define STRINGS_H

#include <stdbool.h>

bool strcmp(char* a, char* b, int max);
int string_split(char delimeter, char* string, char** pointer_array);

#endif