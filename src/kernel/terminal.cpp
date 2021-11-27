#include "terminal.h"

void Terminal::scroll_up() {
  // Scroll the entire buffer up.
  for (int y = 0; y < (height * pages); y++) {
    uint16_t* cline = buffer + y * width;
    uint16_t* nline = buffer + (y+1) * width;
    for (int x = 0; x < width; x++) {
      cline[x] = nline[x];
    }
  }
  // Clear the last line
  uint16_t* last_line = buffer + (height * pages - 1) * width;
  for (int x = 0; x < width; x++) {
    last_line[x] = 0x0720;
  }
  this->cur_y--;

  this->update();
}

void Terminal::putchar(uint32_t ptr, uint8_t c, uint8_t edata) {
  // All modifications to the screen are done to the last page.
  last_page_pointer[ptr] = c | (edata<<8);

  if (active)
    putchar_internal(ptr, c, edata);
}

void Terminal::update(){}
void Terminal::update_cur(){}
void Terminal::putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata) {}

Terminal::Terminal(uint32_t width, uint32_t height, uint32_t pages) {
  this->width = width;
  this->height = height;
  this->pages = pages;
  this->buffer = new uint16_t[width * height * pages];
  this->last_page_pointer = buffer + (width * height * pages) - (width * height);
  this->current_page_pointer = last_page_pointer;

  this->cur_x = 0;
  this->cur_y = 0;

  this->active = false;
}

void Terminal::printf(const char* string, ...) {
  va_list ptr;
  va_start(ptr, string);

  int index = 0;
  char current;

  while (current=string[index++]) {
    if (current == '\n') {
      this->cur_x = 0;
      this->cur_y++;
    }

    if (this->cur_x == this->width) {
      this->cur_x = 0;
      this->cur_y++;
    }

    if (this->cur_y == this->height)
      this->scroll_up();

    if (current == '%') {
      int type = string[index++];
      int offset;
      switch (type) {
        case 'd': {
          char decimal_buffer[11];
          offset = int_to_decimal(va_arg(ptr, int), decimal_buffer);
          printf(decimal_buffer + offset);
          break;
        }
        case 'x': {
          char hex_buffer[8];
          offset = int_to_hex(va_arg(ptr, int), hex_buffer);
          printf(hex_buffer);
          break;
        }
        case 's': {
          printf(va_arg(ptr, const char*));
          break;
        }
        case 'c': {
          int mem_pos = this->cur_y * this->width + this->cur_x++;
          int promoted = va_arg(ptr, int);
          char charred = promoted;
          
          this->putchar(mem_pos, charred);
          break;
        }
      }
      continue;
    }

    if (current != '\n') {
      int mem_pos = this->cur_y * this->width + this->cur_x++;
      
      this->putchar(mem_pos, current);
    }
  }

  this->set_curpos(this->cur_x, this->cur_y);

  va_end(ptr);
}

void Terminal::clear_screen() {
  for (int i=0; i < width * height * pages; i++) {
    buffer[i] = 0;
  }
  this->cur_x = 0;
  this->cur_y = 0;

  if (active)
    this->update();
}

void Terminal::set_curpos(int x, int y) {
  this->cur_x = x;
  this->cur_y = y;

  if (active)
    this->update_cur();
}

uint32_t Terminal::get_cur_x() {
  return this->cur_x;
}

uint32_t Terminal::get_cur_y() {
  return this->cur_y;
}

void Terminal::activate() {
  update();
  this->active = true;
}

void Terminal::deactivate() {
  this->active = false;
}

void TextModeTerminal::update() {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      text_mode_pointer[y * width + x] = current_page_pointer[y * width + x];
    }
  }

  this->update_cur();
}

void TextModeTerminal::update_cur() {
  int cursor_position = this->cur_y * this->width + this->cur_x;
  uint8_t* cursor_position_split = (uint8_t*)&cursor_position;
  outb(0x3D4, 0x0F);
  outb(0x3D5, cursor_position_split[0]);
  outb(0x3D4, 0x0E);
  outb(0x3D5, cursor_position_split[1]);
}

void TextModeTerminal::putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata) {
  text_mode_pointer[ptr] = c | (edata << 8);
}

TextModeTerminal::TextModeTerminal(uint16_t* text_mode_pointer): Terminal(80, 25, 1) {
  this->text_mode_pointer = text_mode_pointer;
}