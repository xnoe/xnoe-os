#include "readwriter.h"

ReadWriter::ReadWriter(uint32_t owner) {
  this->owner = owner;
}

void ReadWriter::giveReadPerm(uint32_t PID) {
  this->allowedRead.append(PID);
}

void ReadWriter::giveWritePerm(uint32_t PID) {
  this->allowedWrite.append(PID);
}

int ReadWriter::read(uint32_t count, uint8_t* buffer){}
int ReadWriter::write(uint32_t count, uint8_t* buffer){}

bool ReadWriter::canRead(uint32_t PID) {
  if (this->owner == PID)
    return true;

  if (this->allowedRead.has(PID))
    return true;
  
  return false;
}

bool ReadWriter::canWrite(uint32_t PID) {
  if (this->owner == PID)
    return true;

  if (this->allowedWrite.has(PID))
    return true;
  
  return false;
}