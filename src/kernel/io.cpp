#include "io.h"

extern "C" {
  void outb(uint16_t portnumber, uint8_t data) {
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (portnumber));
  }
  uint8_t inb(uint16_t portnumber) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a" (result) : "Nd" (portnumber));
    return result;
  }

  void outw(uint16_t portnumber, uint16_t data) {
    asm volatile("outw %0, %1" : : "a" (data), "Nd" (portnumber));
  }
  uint16_t inw(uint16_t portnumber) {
    uint16_t result;
    asm volatile("inw %1, %0" : "=a" (result) : "Nd" (portnumber));
    return result;
  }
}

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