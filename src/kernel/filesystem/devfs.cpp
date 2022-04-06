#include "devfs.h"

bool DevFS::exists(Path p) {
  if (p.start->elem == PathEntry{3, "ata"})
    return true;
  return false;
}

FSType DevFS::type(Path p) {
  if (p.start->elem == PathEntry{3, "ata"})
    return BlockDev;
}


ReadWriter* DevFS::open(Path p) {
  if (p.start->elem == PathEntry{3, "ata"}) {
    return new ATAReadWriter(0, 0);
  }
  return 0;
}


uint32_t DevFS::getDentsSize(Path p) {
  return 0;
}

void DevFS::getDents(Path p, FSDirectoryListing* buffer) {}