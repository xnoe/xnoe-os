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

void VGAModeTerminal::update() {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      putchar_internal(y * width + x, (uint8_t)(current_page_pointer[y * width + x]), 0);
    }
  }
}

void VGAModeTerminal::update_cur() {
  // Todo: Implement cursor for VGAModeTerminal
}

void VGAModeTerminal::putchar_internal(uint32_t ptr, uint8_t c, uint8_t edata) {
  uint32_t col = ptr % width;
  uint32_t row = ptr / width;

  uint32_t sx = col * 8;
  uint32_t sy = row * 8;

  if (c>127)
    return;
  uint8_t* char_data = font[c];

  for (int y=0; y<8; y++) {
    //for (int x=0; x<8; x++) {
      put_pixels_byte(sx, sy+y, 15, char_data[y]);
    //}
  }
}

void VGAModeTerminal::put_pixels_byte(uint32_t x, uint32_t y, uint8_t color, uint8_t pixel_byte) {
  uint32_t pixel = y * 720 + x;
  uint32_t pixelindex = pixel / 8;

  uint8_t trbyte = 0;
  for (int i=0; i<8; i++) {
    trbyte <<= 1;
    trbyte += (pixel_byte>>i)&1;
  }
  
  for (int i=0; i<4; i++) {
    if (color & (1<<i))
      this->planes[i][pixelindex] = trbyte;
    else
      this->planes[i][pixelindex] = 0;
  }
}

void VGAModeTerminal::put_pixel(uint32_t x, uint32_t y, uint8_t color) {
  // For any pixel we need to write 1 bit to planes 0, 1, 2, and 3

  uint32_t pixel = y * 720 + x;
  uint32_t pixelindex = pixel / 8;
  uint32_t pixelbitindex = pixel % 8;
  
  for (int i=0; i<4; i++) {
    if (color & (1<<i))
      this->planes[i][pixelindex] |= (1<<(7-pixelbitindex));
    else
      this->planes[i][pixelindex] &= ~(1<<(7-pixelbitindex));
  }
}

static void VGAModeTerminal::bufferToVRAM(frame_struct* frame, VGAModeTerminal* terminal) {
  uint32_t count4 = (720 * 480) / 8 / 4;
  for (int i=0; i<4; i++) {
    outb(0x3c4, 2);
    outb(0x3c5, 1<<i);

    for (int c=0; c<count4; c++) {
      ((uint32_t*)terminal->vga_pointer)[c] = ((uint32_t*)terminal->planes[i])[c];
    }
  }
}

VGAModeTerminal::VGAModeTerminal(uint8_t* vga_pointer): Terminal(90, 60, 1) {
  this->vga_pointer = vga_pointer;

  for (int i=0; i<4; i++) {
    this->planes[i] = new uint8_t[720 * 480 / 8];
  }

  unsigned char g_720x480x16[] =
  {
    /* MISC */
      0xE7,
    /* SEQ */
      0x03, 0x01, 0x08, 0x00, 0x06,
    /* CRTC */
      0x6B, 0x59, 0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E,
      0x00, 0x40, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00,
      0xEA, 0x0C, 0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xE3,
      0xFF,
    /* GC */
      0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
      0xFF,
    /* AC */
      0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
      0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
      0x01, 0x00, 0x0F, 0x00, 0x00,
  };

  uint8_t* creg = g_720x480x16;

  outb(0x3c2, *(creg++));
  for (int i=0; i<5; i++) {
    outb(0x3c4, i);
    outb(0x3c5, *(creg++));
  }
  outb(0x3d4, 0x3);
  outb(0x3d5, inb(0x3d5) | 0x80);
  outb(0x3d4, 0x11);
  outb(0x3d5, inb(0x3d5) & ~0x80);

  creg[0x03] = creg[0x03] | 0x80;
  creg[0x11] = creg[0x11] & ~0x80;

  for (int i=0; i<25; i++) {
    outb(0x3d4, i);
    outb(0x3d5, *(creg++));
  }

  for (int i=0; i<9; i++) {
    outb(0x3ce, i);
    outb(0x3cf, *(creg++));
  }

  for (int i=0; i<21; i++) {
    inb(0x3da);
    outb(0x3c0, i);
    outb(0x3c1, *(creg++));
  }

  inb(0x3da);
  outb(0x3c0, 0x20);

  uint32_t width4 = 480 / 8 / 4;
  uint32_t height4 = 720 / 8 / 4;
  uint32_t total4 = width4 * height4;

  for (int plane=0; plane<4; plane++) {
    outb(0x3c4, 2);
    outb(0x3c5, 1<<plane);
    for (int i=0; i<total4; i++) {
      ((uint32_t*)this->vga_pointer)[i] = 0;
    }
  }

  Timer::register_event(16, &VGAModeTerminal::bufferToVRAM, this);
}