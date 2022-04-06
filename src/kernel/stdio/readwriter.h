#ifndef READWRITER_H
#define READWRITER_H

#include "../datatypes/linkedlist.h"
#include "../types.h"

class ReadWriter {
private:
  uint32_t owner;
  xnoe::linkedlist<uint32_t> allowedRead;
  xnoe::linkedlist<uint32_t> allowedWrite;

public:
  ReadWriter(uint32_t owner);
  void giveReadPerm(uint32_t PID);
  void giveWritePerm(uint32_t PID);

  virtual uint32_t read(uint32_t count, uint8_t* buffer);
  virtual uint32_t write(uint32_t count, uint8_t* buffer);
  virtual uint32_t size();
  virtual uint32_t seek(uint32_t position);
  uint32_t getOwner();
  bool canRead(uint32_t PID);
  bool canWrite(uint32_t PID);
};

#endif