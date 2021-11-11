#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>

#include "datatypes/tuple.h"
#include "types.h"
#include "strings.h"
#include "io.h"

class Terminal {
private:
  virtual void update();
  virtual void update_cur();
  virtual void putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata=0x07);

  void scroll_up();
  
  void putchar(uint32_t ptr, uint8_t c, uint8_t edata=0x07);

protected:
  uint16_t* buffer;
  uint32_t width;
  uint32_t height;
  uint32_t pages;

  uint32_t cur_x;
  uint32_t cur_y;

  uint16_t* current_page_pointer;
  uint16_t* last_page_pointer;

  bool active;
public:
  Terminal(uint32_t width, uint32_t height, uint32_t pages);

  void printf(const char* string, ...);

  void clear_screen();
  void set_curpos(int x, int y);

  uint32_t get_cur_x();
  uint32_t get_cur_y();

  void activate();
  void deactivate();
};

class TextModeTerminal : public Terminal {
private:
   void update() override;
   void update_cur() override;
   void putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata=0x07) override;

   uint16_t* text_mode_pointer;
public:
  TextModeTerminal(uint16_t* text_mode_pointer);
};

#endif