#ifndef IOPORT_H
#define IOPORT_H

#include "io.h"

class Port {
private:
  uint16_t addr;
public:
  Port(uint16_t a);

  uint8_t readb();
  uint16_t readw();

  void writeb(uint8_t d);
  void writew(uint16_t d);
};

#endif