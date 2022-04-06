#ifndef DEVFS_H
#define DEVFS_H

#include "fstree.h"
#include "../ata.h"
#include "../kernel.h"

class DevFS: public FSTree {
  bool exists(Path p) override;
  FSType type(Path p) override;

  ReadWriter* open(Path p) override;

  uint32_t getDentsSize(Path p) override;
  void getDents(Path p, FSDirectoryListing* buffer) override;
};

#endif