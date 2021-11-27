#include "atapio.h"

// Disk code

// Primary ATA Bus: 0x1f0 to 0x1f7
// Device Control Register: 0x3f6
// Secondary ATA Bus: 0x170 to 0x177
// Device Control Register: 0x376


uint16_t identify_result[256];
uint32_t total_28_lbas = 0;

uint8_t* rootDirEntries;
uint16_t* FAT1;

uint16_t countReserved;
uint8_t countFATs;
uint16_t countRDEs;
uint16_t sectorsPerFAT;


void init_atapio() {
  rootDirEntries = new uint8_t[8192];
  FAT1 = (uint16_t*)(new uint8_t[512 * 34]);

  uint32_t boot_sector = new uint32_t[1024];
  read_sectors(0, 1, (uint8_t*)boot_sector);

  countReserved = *((uint16_t*)(boot_sector + 0x0e));
  countFATs = *((uint8_t*)(boot_sector + 0x10));
  countRDEs = *((uint16_t*)(boot_sector + 0x11));
  sectorsPerFAT = *((uint16_t*)(boot_sector + 0x16));

  // Select Drive 0 on the Primary Bus
  outb(0x1f6, 0xa0);

  // Set sector count to 0 for IDENTIFY
  outb(0x1f2, 0);
  // Set LBAlo to 0
  outb(0x1f3, 0);
  // Set LBAmid to 0
  outb(0x1f4, 0);
  // Set LBAhi to 0
  outb(0x1f5, 0);

  // Send IDENTIFY command
  outb(0x1f7, 0xec);

  uint8_t status = inb(0x1f7);
  if (status) {
    while ((status = inb(0x1f7)) & 0x80);

    if ( !(inb(0x1f4) || inb(0x1f5)) ) {
      while ( !(status & 8 || status & 1) )
        status = inb(0x1f7);
      
      if (!(status & 1)) {
        for (int index=0; index<256; index++)
          identify_result[index] = inw(0x1f0);
      }
    }
  }

  total_28_lbas = *(uint32_t*)(identify_result+60);

  // We've initialised now, let's load the FAT and RootDirEntries.
  read_sectors(sectorsPerFAT * countFATs + countReserved, countRDEs / 16, rootDirEntries);
  read_sectors(countReserved, sectorsPerFAT, (uint8_t*)FAT1);
}

void read_sector(uint32_t address, uint8_t* buffer) {
  outb(0x1f6, 0xe0 | ((address>>24)&0x0f));
  // Read a single sector
  outb(0x1f2, 1);
  // Set LBAlo, LBAmid and LBAhi
  outb(0x1f3, address);
  outb(0x1f4, address>>8);
  outb(0x1f5, address>>16);

  // Send read command 
  outb(0x1f7, 0x20);
  // Poll
  uint8_t status = inb(0x1f7);
  while ( (status & 0x80) && !(status & 8) )
    status = inb(0x1f7);

  for (int index=0; index<256; index++)
    ((uint16_t*)buffer)[index] = inw(0x1f0);
}

void read_sectors(uint32_t address, int count, uint8_t* buffer) {
  for (int i=0; i<count; i++) {
    read_sector(address+i, buffer+512*i);
    for (int i=0; i<15; i++)
      inb(0x1f7);
  } 
}

uint16_t file_exists(char* filename) {
  for (int i=0; i<countRDEs; i++) {
    bool found = strcmp(rootDirEntries+(i*32), filename, 11);
    if (found) {
      uint16_t* correctEntry = (uint16_t*)(rootDirEntries+(i*32));
      return correctEntry[13];
    }
  }
  return 0;
}

void load_file(char* filename, uint8_t* destination) {
  uint16_t location = file_exists(filename);
  if (!location)
    return;

  int offset = 0;

  bool loaded = false;
  while (!loaded) {
    uint16_t fromSector = location + (sectorsPerFAT * countFATs) + (countRDEs / 16) + (countReserved - 1) - 1;
    read_sector(fromSector, destination+offset);
    offset += 512;

    location = FAT1[location++];
    if (location == 0xffff)
      loaded = true;
  }
}

uint32_t file_size(char* filename) {
  for (int i=0; i<countRDEs; i++) {
    bool found = strcmp(rootDirEntries+(i*32), filename, 11);
    if (found) {
      uint32_t* correctEntry = (uint32_t*)(rootDirEntries+(i*32));
      return correctEntry[7];
    }
  }
  return 0;
}