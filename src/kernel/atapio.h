#ifndef ATA_PIO
#define ATA_PIO

#include <stdbool.h>

#include "io.h"
#include "types.h"
#include "strings.h"
#include "allocate.h"
#include "global.h"

void init_atapio();
void read_sector(uint32_t address, uint8_t* buffer);
void read_sectors(uint32_t address, int count, uint8_t* buffer);
uint16_t file_exists(char* filename);
void load_file(char* filename, uint8_t* location);

uint32_t file_size(char* filename);

#endif