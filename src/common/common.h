#ifndef COMMON_H
#define COMMON_H

#include "../kernel/types.h"

void print(char* string);
void readfile(char* filename, uint8_t* buffer);
void* localalloc(uint32_t size);
void localdelete(void* ptr);
uint32_t filesize(char* filename);

uint32_t fork(uint32_t fh);
uint32_t bindStdout(uint32_t PID);
uint32_t bindStdin(uint32_t PID);

uint32_t getPID();

int read(uint32_t count, void* filehandler, uint8_t* buffer);
int write(uint32_t count, void* filehandler, uint8_t* buffer);
void bindToKeyboard();

int fopen(char* filename);
void fclose(uint32_t fh);

int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);

#endif