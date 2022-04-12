#ifndef STRINGS_H
#define STRINGS_H

bool strcmp(char* a, char* b, int max);
char* split_on_first(char delimeter, char* string);
int string_split(char delimeter, char* string, char** pointer_array);
void decode_filename(char* nice_name, char* filenamebuffer);

int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);

#endif