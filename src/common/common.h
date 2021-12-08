#ifndef COMMON_H
#define COMMON_H

#include "../kernel/types.h"

void print(char* string);
char getch();
uint8_t getchPS2();
void readfile(char* filename, uint8_t* buffer);
void* localalloc(uint32_t size);
void localdelete(void* ptr);
uint32_t filesize(char* filename);

uint32_t getPID();

int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);

#endif