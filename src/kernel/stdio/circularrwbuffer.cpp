#include "circularrwbuffer.h"

CircularRWBuffer::CircularRWBuffer(uint32_t reader, uint32_t writer)
: ReadWriter(0) {
  this->giveReadPerm(reader);
  this->giveWritePerm(writer);

  this->bufferSize = 3072;
  this->buffer = new uint8_t[this->bufferSize];
  this->readPtr = 0;
  this->writePtr = 0;
}

uint32_t CircularRWBuffer::write(uint32_t count, uint8_t* buffer) {
  int i=0;
  while (i < count) {
    this->buffer[this->writePtr] = buffer[i];

    this->writePtr++;
    if (this->writePtr == this->bufferSize)
      this->writePtr = 0;
    i++;
  }
  return i;
}

uint32_t CircularRWBuffer::read(uint32_t count, uint8_t* buffer) {
  int i=0;
  while (i < count) {
    if (this->readPtr == this->writePtr)
      return i;
    buffer[i] = this->buffer[this->readPtr];

    this->readPtr++;
    if (this->readPtr == this->bufferSize)
      this->readPtr = 0;
    i++;
  }
  return i;
}

uint32_t CircularRWBuffer::size() {
  return 0;
}