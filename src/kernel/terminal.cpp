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

Terminal::Terminal(uint32_t width, uint32_t height, uint32_t pages)
: ReadWriter(0) {
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

int strToInt(char* str) {
  int r=0;
  while (*str >= 0x30 && *str <= 0x39) {
    r *= 10;
    r += *(str++) - 0x30;
  }
  return r;
}

int clamp(int a, int b, int c) {
  if (a < b)
    return b;
  if (a > c)
    return c;
  return a;
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

    if (current == 0x1b && string[index] == '[') {
      index++;
      char* parameterStart = (string+index);
      while (string[index] >= 0x30 && string[index] <= 0x3F)
        index++;
      char* parameterEnd = (string+index);

      char* intermediateStart = (string+index);
      while (string[index] >= 0x20 && string[index] <= 0x2F)
        index++;

      char final = *(string+(index++));

      switch (final) {
        case 'A':
          this->cur_y -= clamp(strToInt(parameterStart), 0, this->cur_y);
          break;
        case 'B':
          this->cur_y += clamp(strToInt(parameterStart), 0, this->height - this->cur_y);
          break;
        case 'C':
          this->cur_x += clamp(strToInt(parameterStart), 0, this->width - this->cur_x);
          break;
        case 'D':
          this->cur_x -= clamp(strToInt(parameterStart), 0, this->cur_x);
          break;
        case 'H': {
          char* s=parameterStart;
          while (*s != ';' && s < parameterEnd)
            s++;
          s++;
          this->cur_y = clamp(strToInt(parameterStart), 1, this->height) - 1;
          this->cur_x = clamp(strToInt(s), 1, this->width) - 1;
          break;
        }
      }

      continue;
    }

    if (current == '\b') {
      if (this->cur_x > 0) {
        this->cur_x--;
      } else if (this->cur_y > 0) {
        this->cur_y--;
        this->cur_x = this->width-1;
      }

      int mem_pos = this->cur_y * this->width + this->cur_x;
      
      this->putchar(mem_pos, ' ');
      continue;
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

uint32_t Terminal::write(uint32_t count, uint8_t* string) {
  /*char* buf = new char[count+1];
  for (int i=0;i<count;i++) {
    buf[i] = buffer[i];
  }
  buf[count] = 0x00;
  printf(buf);
  delete buf;*/
  int index = 0;
  char current;

  while (index < count) {
    current=string[index++];
    if (current == '\n') {
      this->cur_x = 0;
      this->cur_y++;
    }

    if (current == 0x1b && string[index] == '[') {
      index++;
      char* parameterStart = (string+index);
      while (string[index] >= 0x30 && string[index] <= 0x3F)
        index++;
      char* parameterEnd = (string+index);

      char* intermediateStart = (string+index);
      while (string[index] >= 0x20 && string[index] <= 0x2F)
        index++;

      char final = *(string+(index++));

      switch (final) {
        case 'A':
          this->cur_y -= clamp(strToInt(parameterStart), 0, this->cur_y);
          break;
        case 'B':
          this->cur_y += clamp(strToInt(parameterStart), 0, this->height - this->cur_y);
          break;
        case 'C':
          this->cur_x += clamp(strToInt(parameterStart), 0, this->width - this->cur_x);
          break;
        case 'D':
          this->cur_x -= clamp(strToInt(parameterStart), 0, this->cur_x);
          break;
        case 'H': {
          char* s=parameterStart;
          while (*s != ';' && s < parameterEnd)
            s++;
          s++;
          this->cur_y = clamp(strToInt(parameterStart), 1, this->height) - 1;
          this->cur_x = clamp(strToInt(s), 1, this->width) - 1;
          break;
        }
      }

      continue;
    }

    if (current == '\b') {
      if (this->cur_x > 0) {
        this->cur_x--;
      } else if (this->cur_y > 0) {
        this->cur_y--;
        this->cur_x = this->width-1;
      }

      int mem_pos = this->cur_y * this->width + this->cur_x;
      
      this->putchar(mem_pos, ' ');
      continue;
    }

    if (this->cur_x == this->width) {
      this->cur_x = 0;
      this->cur_y++;
    }

    if (this->cur_y == this->height)
      this->scroll_up();

    if (current != '\n') {
      int mem_pos = this->cur_y * this->width + this->cur_x++;
      
      this->putchar(mem_pos, current);
    }
  }

  this->set_curpos(this->cur_x, this->cur_y);
}

uint32_t Terminal::read(uint32_t count, uint8_t* buffer) {}

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