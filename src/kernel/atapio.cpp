#include "atapio.h"

// Disk code

// Primary ATA Bus: 0x1f0 to 0x1f7
// Device Control Register: 0x3f6
// Secondary ATA Bus: 0x170 to 0x177
// Device Control Register: 0x376


uint16_t identify_result[256];
uint32_t total_28_lbas = 0;

DirectoryEntry* rootDirEntries;
uint16_t* FAT1;

uint16_t countReserved;
uint8_t countFATs;
uint16_t countRDEs;
uint16_t sectorsPerFAT;


void init_atapio() {
  rootDirEntries = (DirectoryEntry*)new uint8_t[8192];
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
  read_sectors(sectorsPerFAT * countFATs + countReserved, countRDEs / 16, (uint8_t*)rootDirEntries);
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

uint32_t clusterToSector(uint32_t cluster) {
  return cluster + (sectorsPerFAT * countFATs) + (countRDEs / 16) + (countReserved - 1) - 1;
}

int split_on_char(char c, char* str, char** split) {
  char* cstr = str;
  char* last = str;
  uint32_t count = 0;
  while (*cstr) {
    if (*cstr == c) {
      *cstr = 0;
      split[count++] = last;
      last = cstr+1;
    }
    cstr++;
  }
  split[count++] = last;
  return count;
}

void to83(char* filename, char* buf83) {
  char* c = filename;
  for (int i=0;i<11;i++)
    buf83[i] = ' ';
  uint32_t bufpos = 0;
  while (*c && bufpos != 11) {
    if (*c == '.')
      bufpos = 8;
    else
      buf83[bufpos++] = *c & 223;
    c++;
  }
}

void load_file(uint32_t location, uint8_t* destination) {
  int offset = 0;

  bool loaded = false;
  while (!loaded) {
    uint16_t fromSector = clusterToSector(location);
    read_sector(fromSector, destination+offset);
    offset += 512;

    location = FAT1[location++];
    if (location == 0xffff)
      loaded = true;
  }
}

int calc_size(uint32_t location) {
  int offset = 0;

  bool loaded = false;
  while (!loaded) {
    uint16_t fromSector = clusterToSector(location);
    offset += 512;

    location = FAT1[location++];
    if (location == 0xffff)
      loaded = true;
  }
  return offset;
}

DirectoryEntry* get_DE(char* filename) {
  DirectoryEntry* dirbuf;
  Directory dir = {
    .entry = rootDirEntries,
    .entries = countRDEs
  };

  char* levels[8];
  int count = split_on_char('/', filename, levels);

  for (int i=0; i<(count-1); i++) {
    char normalname[11] = {' '};
    to83(levels[i], normalname);
    for (int e=0; e<dir.entries; e++) {
      bool found=strcmp(dir.entry[e].name, normalname, 11);
      if (found) {
        DirectoryEntry* de = &dir.entry[e];
        uint32_t size = calc_size(de->firstClusterLow);
        dirbuf = (DirectoryEntry*)(new uint8_t[size]);
        uint32_t cluster = de->firstClusterLow;
        load_file(cluster, (uint8_t*)dirbuf);
        dir.entry = dirbuf;
        dir.entries = size / 32;
        break;
      }
    }
  }

  char normalname[11];
  to83(levels[count-1], normalname);

  for (int i=0; i<dir.entries; i++) {
    bool found = strcmp(dir.entry[i].name, normalname, 11);
    if (found) {
      return &dir.entry[i];
    }
  }
  return 0;
}

uint16_t file_exists(char* filename) {
  DirectoryEntry* de = get_DE(filename);
  if (de)
    return de->firstClusterLow;
  return 0;
}

uint32_t file_size(char* filename) {
  for (int i=0; i<countRDEs; i++) {
    bool found = strcmp(rootDirEntries[i].name, filename, 11);
    if (found) {
      if (rootDirEntries[i].size % 512)
        return ((rootDirEntries[i].size / 512) + 1) * 512;
      else
        return rootDirEntries[i].size;
    }
  }
  return 0;
}

void load_file(char* filename, uint8_t* location) {
  DirectoryEntry* de = get_DE(filename);
  load_file(de->firstClusterLow, location);
}

FATFileReadWriter::FATFileReadWriter(uint32_t owner, char* filename)
: ReadWriter(owner) {
  this->bytesRead = 0;
  DirectoryEntry* de = get_DE(filename);
  this->sizeBytes = de->size;
  this->currentCluster = de->firstClusterLow;
  this->read_buffer = new uint8_t[512];
  read_sector(clusterToSector(this->currentCluster), this->read_buffer);
}

uint32_t FATFileReadWriter::read(uint32_t count, uint8_t* buffer) {
  int index = 0;
  while (count) {
    buffer[index] = this->read_buffer[this->bytesRead++];
    if (this->bytesRead == 512) {
      this->currentCluster = FAT1[this->currentCluster++];
      read_sector(clusterToSector(this->currentCluster), this->read_buffer);
      this->bytesRead = 0;
    }
    count--;
    index++;
  }
}
uint32_t FATFileReadWriter::write(uint32_t count, uint8_t* buffer) {}
uint32_t FATFileReadWriter::size() {
  return this->sizeBytes;
}
