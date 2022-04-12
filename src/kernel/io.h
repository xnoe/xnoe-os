#ifndef IO_H
#define IO_H

#include "types.h"

extern "C" {
  void outb(uint16_t portnumber, uint8_t data);
  uint8_t inb(uint16_t portnumber);

  void outw(uint16_t portnumber, uint16_t data);
  uint16_t inw(uint16_t portnumber);
}

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