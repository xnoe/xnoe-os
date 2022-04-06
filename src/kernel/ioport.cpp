#include "ioport.h"

Port::Port(uint16_t a) {
  this->addr = a;
}

uint8_t Port::readb() {
  return inb(this->addr);
}
uint16_t Port::readw() {
  return inw(this->addr);
}

void Port::writeb(uint8_t d) {
  outb(this->addr, d);
}
void Port::writew(uint16_t d) {
  outw(this->addr, d);
}