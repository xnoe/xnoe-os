#ifndef ATA_PIO
#define ATA_PIO

#include <stdbool.h>

#include "io.h"
#include "types.h"
#include "strings.h"
#include "allocate.h"
#include "global.h"

#include "stdio/readwriter.h"

struct __attribute__((packed)) DirectoryEntry {
  char name[11];
  uint8_t _ignored0 : 3;
  uint8_t archive : 1;
  uint8_t directory : 1;
  uint8_t volumeid : 1;
  uint8_t system : 1;
  uint8_t hidden : 1;
  uint8_t readonly : 1;
  uint8_t _ignored1;
  uint16_t createdHour : 5;
  uint16_t createdMinute : 6;
  uint16_t createdSecond : 5;

  uint16_t createdYear : 7;
  uint16_t createdMonth : 4;
  uint16_t createdDay : 5;

  uint16_t lastAccessYear : 7;
  uint16_t lastAccessMonth : 4;
  uint16_t lastAccessDay : 5;

  uint16_t firstClusterHigh;

  uint16_t modifiedHour : 5;
  uint16_t modifiedMinute : 6;
  uint16_t modifiedSecond : 5;

  uint16_t modifiedYear : 7;
  uint16_t modifiedMonth : 4;
  uint16_t modifiedDay : 5;

  uint16_t firstClusterLow;
  uint32_t size;
};

struct Directory {
  DirectoryEntry* entry;
  uint32_t entries;
};

void init_atapio();
void read_sector(uint32_t address, uint8_t* buffer);
void read_sectors(uint32_t address, int count, uint8_t* buffer);
uint16_t file_exists(char* filename);
void load_file(char* filename, uint8_t* location);

uint32_t file_size(char* filename);

class FATFileReadWriter : public ReadWriter {
private:
  uint32_t sizeBytes;
  uint32_t bytesRead;
  uint32_t currentCluster;
  uint8_t* read_buffer;

public:
  FATFileReadWriter(uint32_t owner, char* filename);
  uint32_t read(uint32_t count, uint8_t* buffer) override;
  uint32_t write(uint32_t count, uint8_t* buffer) override;
  uint32_t size() override;
};

#endif