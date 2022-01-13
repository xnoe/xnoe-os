#ifndef CIRCULAR_RW_BUFFER_H
#define CIRCULAR_RW_BUFFER_H

#include "readwriter.h"

class CircularRWBuffer : public ReadWriter {
private:
  uint8_t* buffer;
  uint32_t readPtr;
  uint32_t writePtr;
  uint32_t bufferSize;
public:
  CircularRWBuffer(uint32_t reader, uint32_t writer);

  int read(uint32_t count, uint8_t* buffer) override;
  int write(uint32_t count, uint8_t* buffer) override;
};

#endif