#include "fstree.h"

bool operator==(const PathEntry& lhs, const PathEntry& rhs) {
  if (lhs.length == rhs.length)
    if (lhs.length == 0)
      return true;
    else
      return strcmp(lhs.path, rhs.path, lhs.length);
  return false;
}

// FS Tree Skeleton
bool FSTree::exists(Path p){}
FSType FSTree::type(Path p){}
ReadWriter* FSTree::open(Path p){}
uint32_t FSTree::getDentsSize(Path p){}
void FSTree::getDents(Path p, FSDirectoryListing* buffer){}

// RootFSTree

RootFSTree::RootFSTree() {
  this->node = new FSTreeNode{
    PathEntry{0,0},
    xnoe::linkedlist<FSTreeNode*>(),
    0
  };
}

bool pathEntryInFSTreeNode(PathEntry p, FSTreeNode* n) {
  xnoe::linkedlistelem<FSTreeNode*>* current = n->children.start;
  while (current) {
    if (current->elem->self == p)
      return true;
    current = current->next;
  }
  return false;
}

FSTreeNode* getNodeFromPathEntry(PathEntry p, FSTreeNode* n) {
  if (!n)
    return 0;
  xnoe::linkedlistelem<FSTreeNode*>* current = n->children.start;
  while (current) {
    if (current->elem->self == p)
      return current->elem;
    current = current->next;
  }
  return 0;
}

FSTreeNode* RootFSTree::makeNodeIfNotExist(Path p) {
  PathElement* currentPathElement = p.start;

  FSTreeNode* currentNode = this->node;

  if (!currentPathElement || currentPathElement == p.end)
    return currentNode;

nextPE:
  while (currentPathElement) {
    xnoe::linkedlistelem<FSTreeNode*>* currentChild = currentNode->children.start;
    while (currentChild) {
      if (currentChild->elem->self == currentPathElement->elem) {
        currentNode = currentChild->elem;
        currentPathElement = currentPathElement->next;
        goto nextPE;
      }
      currentChild = currentChild->next;
    }
    currentNode->children.append(new FSTreeNode{currentPathElement->elem, xnoe::linkedlist<FSTreeNode*>(), 0});
    currentNode = currentNode->children.end->elem;
    currentPathElement = currentPathElement->next;
  }

  return currentNode;
}

FSTree* RootFSTree::getLongestMatchingUnder(Path p) {
  PathElement* currentPath = p.start;
  FSTreeNode* currentNode = this->node;
  FSTree* lastMountpoint = 0;
  while (currentPath && currentNode) {
    if ((currentPath->elem == currentNode->self) && currentNode->mountpoint)
      lastMountpoint = currentNode->mountpoint;
    currentNode = getNodeFromPathEntry(currentPath->elem, currentNode);
    if (currentNode && currentNode->mountpoint)
      lastMountpoint = currentNode->mountpoint;
    currentPath = currentPath->next;
  }
  return lastMountpoint;
}

Path* RootFSTree::getRemainingPath(Path p) {
  PathElement* currentPath = p.start;
  FSTreeNode* currentNode = this->node;
  PathElement* lastMountpoint = 0;
  while (currentPath && currentNode) {
    if (currentPath->elem == currentNode->self && currentNode->mountpoint)
      lastMountpoint = currentPath;
    currentNode = getNodeFromPathEntry(currentPath->elem, currentNode);
    if (currentNode && currentNode->mountpoint)
      lastMountpoint = currentPath;
    currentPath = currentPath->next;
  }
  lastMountpoint = lastMountpoint->next;
  
  if (lastMountpoint) {
    Path* np = new Path;
    PathElement* current = lastMountpoint;
    while (current) {
      np->append(current->elem);
      current = current->next;
    }
    return np;
  }
  return 0;
}

template<typename T>
T RootFSTree::attempt(T(FSTree::*fn)(Path), Path p, T fallback) {
  FSTree* mp = getLongestMatchingUnder(p);
  if (mp) {
    Path* rp = getRemainingPath(p);
    T r;
    if (rp) {
      r = (mp->*fn)(*rp);
      rp->destroy();
    } else {
      r = (mp->*fn)(Path());
    }
    return r;
  }
  return fallback;
}

void RootFSTree::attempt(void(FSTree::*fn)(Path, FSDirectoryListing*), Path p, FSDirectoryListing* b) {
  FSTree* mp = getLongestMatchingUnder(p);
  if (mp) {
    Path* rp = getRemainingPath(p);
    if (rp) {
      (mp->*fn)(*rp, b);
      rp->destroy();
    } else {
      (mp->*fn)(Path(), b);
    }
  }
}

bool RootFSTree::exists(Path p) {
  return attempt<bool>(&FSTree::exists, p, false);
}
FSType RootFSTree::type(Path p){
  return attempt<FSType>(&FSTree::type, p, Directory);
}
ReadWriter* RootFSTree::open(Path p){
  return attempt<ReadWriter*>(&FSTree::open, p, 0);
}
uint32_t RootFSTree::getDentsSize(Path p){
  return attempt<uint32_t>(&FSTree::getDentsSize, p, 0);
}
void RootFSTree::getDents(Path p, FSDirectoryListing* buffer){
  attempt(&FSTree::getDents, p, buffer);
}

bool RootFSTree::isMountpoint(Path p) {
  Path* mp = getRemainingPath(p);
  if (mp->start->next)
    return false;
  else
    return true;
}
void RootFSTree::mount(Path p, FSTree* f) {
  FSTreeNode* fstn = makeNodeIfNotExist(p);
  fstn->mountpoint = f;
}
void RootFSTree::unmount(Path p) {

}

Path createPathFromString(char* s) {
  Path p;
  p.start = 0;
  p.end = 0;

  char* lastPtr = s;
  uint32_t length = 0;

  char c;
  while (c=*(s++)) {
    if (c == '/') {
      if (length == 0)
        p.append(PathEntry{length, 0});
      else
        p.append(PathEntry{length, lastPtr});
      lastPtr = s;
      length = 0;
      continue;
    }
    length += 1;
  }
  if (length)
    p.append(PathEntry{length, lastPtr});
  return p;
}