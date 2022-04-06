#ifndef FSTREE_H
#define FSTREE_H

#include "../datatypes/linkedlist.h"
#include "strings.h"
#include "../stdio/readwriter.h"

struct PathEntry {
  uint16_t length;
  uint8_t* path;
};

bool operator==(const PathEntry& lhs, const PathEntry& rhs);

using Path = xnoe::linkedlist<PathEntry>;
using PathElement = xnoe::linkedlistelem<PathEntry>;

enum FSType {
  File,
  Directory,
  CharacterDev,
  BlockDev,
  NoExist
};

struct FSDirectoryEntry {
  PathEntry path;
  FSType type;
  uint32_t sizeBytes;
};

struct FSDirectoryListing {
  uint32_t count;
  FSDirectoryEntry entries[];
};

class FSTreeNode;
class FSTree {
public:
  virtual bool exists(Path p);
  virtual FSType type(Path p);

  virtual ReadWriter* open(Path p);

  virtual uint32_t getDentsSize(Path p);
  virtual void getDents(Path p, FSDirectoryListing* buffer);
};

struct FSTreeNode {
  PathEntry self;
  xnoe::linkedlist<FSTreeNode*> children;
  FSTree* mountpoint;
};

class RootFSTree: public FSTree {
private:
  FSTree* getLongestMatchingUnder(Path p);
  Path* getRemainingPath(Path p);

  FSTreeNode* makeNodeIfNotExist(Path p);

  template<typename T>
  T attempt(T(FSTree::*fn)(Path), Path p, T fallback);
  void attempt(void(FSTree::*fn)(Path, FSDirectoryListing*), Path p, FSDirectoryListing* b);
public:
  RootFSTree();

  FSTreeNode* node;

  bool isMountpoint(Path p);
  void mount(Path p, FSTree* f);
  void unmount(Path p);

  bool exists(Path p) override;
  FSType type(Path p) override;

  ReadWriter* open(Path p) override;

  uint32_t getDentsSize(Path p) override;
  void getDents(Path p, FSDirectoryListing* buffer) override;
};

Path createPathFromString(char* s);

#endif