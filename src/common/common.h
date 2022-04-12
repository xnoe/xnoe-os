#ifndef COMMON_H
#define COMMON_H

#include "../kernel/types.h"

typedef enum {
  File,
  Directory,
  CharacterDev,
  BlockDev,
  NoExist
} FSType; 

typedef struct {
  uint16_t length;
  uint8_t* path;
} PathEntry;

typedef struct {
  PathEntry path;
  FSType type;
  uint32_t sizeBytes;
} FSDirectoryEntry;

typedef struct {
  uint32_t count;
  uint32_t stringsLength;
  FSDirectoryEntry entries[];
} FSDirectoryListing;

#define syscall_hdlr_0(a, b, c) \
  a b();
#define syscall_hdlr_1(a, b, c, d, e) \
  a b(d e);
#define syscall_hdlr_2(a, b, c, d, e, f, g) \
  a b(d e, f g);
#define syscall_hdlr_3(a, b, c, d, e, f, g, h, i) \
  a b(d e, f g, h i);

#include "syscalls.h"

void print(char* string);
int int_to_decimal(unsigned int number, char* string_buffer);
int int_to_hex(unsigned int number, char* string_buffer);

#endif