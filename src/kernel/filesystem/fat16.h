#ifndef FAT16_H
#define FAT16_H

#include "fstree.h"
#include "strings.h"
#include "../memory.h"
#include "../stdio/readwriter.h"
#include "../datatypes/tuple.h"

struct __attribute__((packed)) DirectoryEntry {
  char name[11];


  uint8_t readonly : 1;
  uint8_t hidden : 1;
  uint8_t system : 1;
  uint8_t volumeid : 1;
  uint8_t directory : 1;
  uint8_t archive : 1;
  uint8_t device : 1;
  uint8_t _ignored0 : 1;

  uint8_t f1 : 1;
  uint8_t f2 : 1;
  uint8_t f3 : 1;
  uint8_t f4 : 1;
  uint8_t _ignored1 : 1;
  uint8_t deleteRequiresPassword : 1;
  uint8_t writeRequiresPassword : 1;
  uint8_t readRequiresPassword : 1;

  uint8_t createTime10ms;

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

class FAT16FS;
class FAT16FileReadWriter: public ReadWriter {
private:
  uint32_t firstCluster;
  uint32_t sizeBytes;

  uint32_t currentPosition;

  uint32_t offsetBytesToCluster(uint32_t offset);

  FAT16FS* backingFS;
public:
  FAT16FileReadWriter(uint32_t owner, uint32_t firstCluster, uint32_t sizeBytes, FAT16FS* backingFS);

  uint32_t read(uint32_t count, uint8_t* buffer) override;
  uint32_t write(uint32_t count, uint8_t* buffer) override;
  uint32_t size() override;
  uint32_t seek(uint32_t position) override;
};

class FAT16FS: public FSTree {
public:
  DirectoryEntry* rootDirEntries;
  uint16_t* FAT1;
  uint8_t sectorOne[512];

  uint16_t* countReserved = ((uint16_t*)(sectorOne + 0x0e));
  uint8_t* countFATs = ((uint8_t*)(sectorOne + 0x10));
  uint16_t* countRDEs = ((uint16_t*)(sectorOne + 0x11));
  uint16_t* sectorsPerFAT = ((uint16_t*)(sectorOne + 0x16));

  ReadWriter* backingDevice;

  bool pathEntryTo83(PathEntry pe, char* buffer);

  uint32_t clusterToSector(uint32_t cluster);
  void load_file(uint32_t location, uint8_t* destination);
  uint32_t calc_size(uint32_t location);

  xnoe::tuple<DirectoryEntry*, uint32_t, bool> getDirectoryEntry(Path p);
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> getDirectoryEntryFull(Path p);

  FAT16FS(ReadWriter* disk);

  bool exists(Path p) override;
  FSType type(Path p) override;

  ReadWriter* open(Path p) override;

  uint32_t getDentsSize(Path p) override;
  void getDents(Path p, FSDirectoryListing* buffer) override;
};

#endif