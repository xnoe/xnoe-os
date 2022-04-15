#ifndef TERMINAL_H
#define TERMINAL_H

#include <stdarg.h>

#include "datatypes/tuple.h"
#include "types.h"
#include "strings.h"
#include "io.h"
#include "font.h"

#include "stdio/readwriter.h"

struct frame_struct;

namespace Timer {
  void register_event(uint32_t milliseconds, void(*function)(frame_struct*, void*), void* auxiliary, bool oneshot=false);
}

enum TerminalState {
  None,
  EscapeCode,
  CSI,
  ParameterBytes,
  IntermediaryBytes,
  FinalByte,
  SGR
};

class Terminal: public ReadWriter {
private:
  virtual void update();
  virtual void update_cur();
  virtual void putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata);

  void scroll_up(uint32_t count=1);
  void scroll_down(uint32_t count=1);
  
  void putchar(uint8_t c);

  TerminalState state = None;
  uint8_t parameterBytes[128];
  uint8_t intermediaryBytes[128];
  uint32_t parameterIndex = 0;
  uint32_t intermediaryIndex = 0;

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

  uint8_t edata=0x07;
public:
  Terminal(uint32_t width, uint32_t height, uint32_t pages);

  void printf(const char* string, ...);

  uint32_t write(uint32_t count, uint8_t* buffer) override;
  uint32_t read(uint32_t count, uint8_t* buffer) override;

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
  void putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata) override;

  uint16_t* text_mode_pointer;
public:
  TextModeTerminal(uint16_t* text_mode_pointer);
};


class VGAModeTerminal : public Terminal {
private:
  void update() override;
  void update_cur() override;
  void putchar_internal(uint32_t ptr, uint8_t c,uint8_t edata) override;

  void put_pixel(uint32_t x, uint32_t y, uint8_t color);
  void put_pixels_byte(uint32_t x, uint32_t y, uint8_t color, uint8_t pixel_byte);

  static void bufferToVRAM(frame_struct* frame, VGAModeTerminal* terminal);
   
public:
  uint8_t* vga_pointer;
  uint8_t* planes[4];

  VGAModeTerminal(uint8_t* vga_pointer);
};
#endif