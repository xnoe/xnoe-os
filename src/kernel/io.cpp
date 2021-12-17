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