#include "fat16.h"

uint32_t FAT16FileReadWriter::offsetBytesToCluster(uint32_t offset) {
  uint32_t cluster = this->firstCluster;
  uint32_t remaining = offset;
  while (remaining > this->clusterSize) {
    cluster = this->backingFS->FAT1[this->firstCluster];
    remaining -= this->clusterSize;
  }

  return cluster;
}

FAT16FileReadWriter::FAT16FileReadWriter(uint32_t owner, uint32_t firstCluster, uint32_t sizeBytes, FAT16FS* backingFS)
: ReadWriter(owner) {
  this->firstCluster = firstCluster;
  this->sizeBytes = sizeBytes;
  this->currentPosition = 0;
  this->backingFS = backingFS;
  this->clusterSize = 512 * (uint32_t)(*this->backingFS->sectorsPerCluster);
}

uint32_t FAT16FileReadWriter::read(uint32_t count, uint8_t* buffer) {
  uint8_t* clusterBuffer = new uint8_t[this->clusterSize];
  uint32_t clusterToRead = offsetBytesToCluster(this->currentPosition);
  uint32_t sectorToRead = this->backingFS->clusterToSector(clusterToRead);
  this->backingFS->backingDevice->seek(sectorToRead * this->clusterSize);
  this->backingFS->backingDevice->read(this->clusterSize, clusterBuffer);

  uint32_t currentClusterIndex = this->currentPosition % this->clusterSize;

  uint32_t remaining = count;

  uint32_t index = 0;
  while (remaining) {
    if (currentClusterIndex == this->clusterSize) {
      clusterToRead = this->backingFS->FAT1[clusterToRead];
      if (clusterToRead == 0xffff)
        break;
      sectorToRead = this->backingFS->clusterToSector(clusterToRead);
      this->backingFS->backingDevice->seek(sectorToRead * this->clusterSize);
      this->backingFS->backingDevice->read(this->clusterSize, clusterBuffer);
      currentClusterIndex = 0;
    }

    buffer[index++] = clusterBuffer[currentClusterIndex++];
    remaining--;
  }

  delete[] clusterBuffer;
  return index;
}

uint32_t FAT16FileReadWriter::write(uint32_t count, uint8_t* buffer) {}

uint32_t FAT16FileReadWriter::size() {
  return this->sizeBytes;
}

uint32_t FAT16FileReadWriter::seek(uint32_t position) {
  if (position < this->sizeBytes) {
    this->currentPosition = position;
    return position;
  }
  return 0;
}

char safeUppercase(char c) {
  switch (c) {
    case 'a'...'z':
      return c & ~32;
    default:
      return c;
  }
}

bool FAT16FS::pathEntryTo83(PathEntry pe, char* buffer) {
  uint32_t maxSize = pe.length;
  uint8_t* data = pe.path;

  uint32_t readIndex=0;
  uint32_t writeIndex=0;

  while (writeIndex<11 && readIndex < maxSize) {
    char c;
    if ((c=data[readIndex++]) == '.') {
      writeIndex = 8;
      continue;
    }
    buffer[writeIndex++] = safeUppercase(c);
  }
}

uint32_t FAT16FS::clusterToSector(uint32_t cluster) {
  return (cluster * (uint32_t)(*this->sectorsPerCluster)) + (*sectorsPerFAT * *countFATs) + (*countRDEs / 16) + (*countReserved - 1) - 1;
}

void FAT16FS::load_file(uint32_t location, uint8_t* destination) {
  int offset = 0;

  bool loaded = false;
  while (!loaded) {
    uint16_t fromSector = clusterToSector(location);
    this->backingDevice->seek(fromSector * 512 * *this->sectorsPerCluster);
    this->backingDevice->read(512 * *this->sectorsPerCluster, destination+offset);
    offset += 512 * *this->sectorsPerCluster;

    location = FAT1[location++];
    if (location == 0xffff)
      loaded = true;
  }
}

uint32_t FAT16FS::calc_size(uint32_t location) {
  int offset = 0;

  bool loaded = false;
  while (!loaded) {
    uint16_t fromSector = clusterToSector(location);
    offset += 512 * *this->sectorsPerCluster;

    location = FAT1[location++];
    if (location == 0xffff)
      loaded = true;
  }
  return offset;
}

xnoe::tuple<DirectoryEntry*, uint32_t, bool> FAT16FS::getDirectoryEntry(Path p) {
  PathElement* current = p.start;

  DirectoryEntry* currentDirectory = new DirectoryEntry[*countRDEs];
  for (int i=0; i < *countRDEs; i++)
    currentDirectory[i] = rootDirEntries[i];

  uint32_t count = *countRDEs;

  if (!current)
    return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);

escape_for:
  while (current != p.end) {
    char name83[12] = "           ";
    pathEntryTo83(current->elem, name83);
    for (int i=0; i < count; i++) {
      if (strcmp(currentDirectory[i].name, name83, 11)) {
        DirectoryEntry found = currentDirectory[i];
        if (!(found.directory))
          return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);

        delete currentDirectory;
        uint32_t sizeBytes = calc_size(found.firstClusterLow);
        currentDirectory = (DirectoryEntry*)(new uint8_t[sizeBytes]);

        load_file(found.firstClusterLow, (uint8_t*)currentDirectory);

        i=0;
        count = sizeBytes / sizeof(DirectoryEntry);

        current = current->next;
        goto escape_for;
      }
    }

    return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, 0, false);
  }

  return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);
}

xnoe::tuple<DirectoryEntry*, uint32_t, bool> FAT16FS::getDirectoryEntryFull(Path p) {
  PathElement* current = p.start;

  DirectoryEntry* currentDirectory = new DirectoryEntry[*countRDEs];
  for (int i=0; i < *countRDEs; i++)
    currentDirectory[i] = rootDirEntries[i];

  uint32_t count = *countRDEs;

  if (!current)
    return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);

escape_for:
  while (current) {
    char name83[12] = "           ";
    pathEntryTo83(current->elem, name83);
    for (int i=0; i < count; i++) {
      if (strcmp(currentDirectory[i].name, name83, 11)) {
        DirectoryEntry found = currentDirectory[i];
        if (!(found.directory))
          return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);

        delete currentDirectory;
        uint32_t sizeBytes = calc_size(found.firstClusterLow);
        currentDirectory = (DirectoryEntry*)(new uint8_t[sizeBytes]);

        load_file(found.firstClusterLow, (uint8_t*)currentDirectory);

        i=0;
        count = sizeBytes / sizeof(DirectoryEntry);

        current = current->next;
        goto escape_for;
      }
    }

    return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, 0, false);
  }

  return xnoe::tuple<DirectoryEntry*, uint32_t, bool>(currentDirectory, count, true);
}

FAT16FS::FAT16FS(ReadWriter* disk) {
  this->backingDevice = disk;

  this->backingDevice->seek(0);
  this->backingDevice->read(512, sectorOne);

  this->rootDirEntries = new DirectoryEntry[*countRDEs];

  this->backingDevice->seek(((*sectorsPerFAT) * (*countFATs) + (*countReserved)) * 512);
  this->backingDevice->read((*countRDEs) * sizeof(DirectoryEntry), (uint8_t*)this->rootDirEntries);

  this->FAT1 = new uint16_t[(*sectorsPerFAT) * 256];

  this->backingDevice->seek((*countReserved) * 512);
  this->backingDevice->read((*sectorsPerFAT) * 512, (uint8_t*)FAT1);
}

bool FAT16FS::exists(Path p) {
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> directory = getDirectoryEntry(p);
  if (!xnoe::get<2>(directory))
    return false;
  
  bool found = false;
  DirectoryEntry* directoryEntries = xnoe::get<0>(directory);
  uint32_t count = xnoe::get<1>(directory);

  PathElement* end = p.end;
  if (!end)
    return false;

  char name83[11] = {' '};
  pathEntryTo83(end->elem, name83);
  for (int i=0; i<count; i++) {
    if (strcmp(directoryEntries[i].name, name83, 11)) {
      found = true;
      break;
    }
  }

  delete directoryEntries;

  return found;
}

FSType FAT16FS::type(Path p) {
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> directory = getDirectoryEntry(p);
  if (!xnoe::get<2>(directory))
    return NoExist;
  
  FSType found = NoExist;
  DirectoryEntry* directoryEntries = xnoe::get<0>(directory);
  uint32_t count = xnoe::get<1>(directory);

  PathElement* end = p.end;
  if (!end)
    return NoExist;

  char name83[11] = {' '};
  pathEntryTo83(end->elem, name83);
  for (int i=0; i<count; i++) {
    if (strcmp(directoryEntries[i].name, name83, 11)) {
      if (directoryEntries[i].directory)
        found = Directory;
      else
        found = File;
    }
  }

  delete directoryEntries;

  return found;
}

ReadWriter* FAT16FS::open(Path p) {
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> directory = getDirectoryEntry(p);
  if (!xnoe::get<2>(directory))
    return 0;
  
  DirectoryEntry* directoryEntries = xnoe::get<0>(directory);
  uint32_t count = xnoe::get<1>(directory);

  PathElement* end = p.end;
  if (!end)
    return 0;
  
  uint32_t written=0;

  char name83[12] = "           ";
  name83[11] = 0;
  pathEntryTo83(end->elem, name83);
  for (int i=0; i<count; i++) {
    if (strcmp(directoryEntries[i].name, name83, 11)) {
      if (!directoryEntries[i].directory)
        return new FAT16FileReadWriter(0, ((uint32_t)directoryEntries[i].firstClusterHigh << 16) | directoryEntries[i].firstClusterLow, directoryEntries[i].size, this);
    }
  }
  delete directoryEntries;
  return 0;
}

PathEntry name83ToPathEntry(char* name83, char* text) {
  uint32_t mainLength = 8;
  uint32_t index = 7;
  while (name83[index] == ' ' && index--)
    mainLength--;
  
  uint32_t extLength = 3;
  index = 10;
  while (name83[index] == ' ' && index-- > 7)
    extLength--;

  memcpy(name83, text, mainLength);
  if (name83[8] != ' ') {
    text[mainLength] = '.';
    memcpy(name83+8, text+mainLength+1, extLength);
  }
  text[mainLength+extLength+1] = 0;
  return PathEntry{mainLength+extLength+1, text};
}

uint32_t getRealCount(DirectoryEntry* directoryEntries, uint32_t c) {
  uint32_t r = 0;
  for (int i = 0; i < c; i++) {
    if (directoryEntries[i].name[0] != 0 && directoryEntries[i].name[0] != 0xE5 && !directoryEntries[i].volumeid)
      r++;
  }
  return r;
}

uint32_t FAT16FS::getDentsSize(Path p) {
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> directory = getDirectoryEntryFull(p);
  DirectoryEntry* directoryEntries = xnoe::get<0>(directory);
  if (!xnoe::get<2>(directory)) {
    delete directoryEntries;
    return 0;
  }
  
  uint32_t found = 0;
  uint32_t count = xnoe::get<1>(directory);

  found += sizeof(FSDirectoryListing);
  for (int i=0; i<count; i++) {
    if (directoryEntries[i].name[0] != 0 && directoryEntries[i].name[0] != 0xE5 && !directoryEntries[i].volumeid) {
      found += sizeof(FSDirectoryEntry);
      found += 13;
    }
  }

  delete directoryEntries;

  return found;
}

void FAT16FS::getDents(Path p, FSDirectoryListing* buffer) {
  xnoe::tuple<DirectoryEntry*, uint32_t, bool> directory = getDirectoryEntryFull(p);
  DirectoryEntry* directoryEntries = xnoe::get<0>(directory);
  if (!xnoe::get<2>(directory)) {
    delete directoryEntries;
    return 0;
  }

  uint32_t count = xnoe::get<1>(directory);
  
  uint32_t written=0;

  buffer->count = getRealCount(directoryEntries, count);
  buffer->stringsLength = 0;

  char* nameBuffer = ((char*)buffer);
  nameBuffer += sizeof(FSDirectoryEntry)*buffer->count + sizeof(FSDirectoryListing);
  
  for (int i=0; i<count; i++) {
    if (directoryEntries[i].name[0] != 0 && directoryEntries[i].name[0] != 0xE5 && !directoryEntries[i].volumeid) {
      buffer->entries[written] = FSDirectoryEntry {
        name83ToPathEntry(directoryEntries[i].name, nameBuffer + 13*written),
        directoryEntries[i].directory ? Directory : File,
        directoryEntries[i].size
      };
      written++;
      buffer->stringsLength += 13;
    }
  }

  delete directoryEntries;
}