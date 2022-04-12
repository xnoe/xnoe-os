#include "terminal.h"

int strToInt(char* str, uint32_t max) {
  int r=0;
  int i=0;
  while (*str >= 0x30 && *str <= 0x39 && i < max) {
    r *= 10;
    r += *(str++) - 0x30;
    i++;
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

void Terminal::scroll_up(uint32_t count) {
  for (int i=0; i<count; i++) {
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
      last_line[x] = 0x20 | (edata << 8);
    }
    this->cur_y--;
  }

  this->update();
}


void Terminal::scroll_down(uint32_t count) {
  for (int i=0; i<count; i++) {
    // Scroll the entire buffer up.
    for (int y = (height * pages)-2; y >= 0; y--) {
      uint16_t* nline = buffer + y * width;
      uint16_t* cline = buffer + (y+1) * width;
      for (int x = 0; x < width; x++) {
        cline[x] = nline[x];
      }
    }
    // Clear the last line
    uint16_t* last_line = buffer + (height * (pages - 1)) * width;
    for (int x = 0; x < width; x++) {
      last_line[x] = 0x20 | (edata << 8);
    }
    this->cur_y--;
  }

  this->update();
}

void Terminal::putchar(uint8_t c) {
  again:
  switch (this->state) {
    case None:
      switch (c) {
        case 0x1b:
          this->state = EscapeCode;
          break;
        case '\n':
          this->cur_x = 0;
          this->cur_y++;
          break;
        case '\b':
          if (this->cur_x == 0) {
            if (this->cur_y > 0) {
              this->cur_x = this->width-1;
              this->cur_y--;
            }
          } else {
            this->cur_x--;
          }
          last_page_pointer[this->cur_y*this->width+this->cur_x] = ' ' | (edata<<8);
          if (active)
            putchar_internal(this->cur_y*this->width+this->cur_x, ' ');
          break;
        default:
          if (this->cur_x == this->width) {
            this->cur_x = 0;
            this->cur_y++;
          }
          // All modifications to the screen are done to the last page.
          last_page_pointer[this->cur_y*this->width+this->cur_x] = c | (edata<<8);

          if (active)
            putchar_internal(this->cur_y*this->width+this->cur_x, c);
          this->cur_x++;
          break;
      }
      break;
    case EscapeCode:
      switch (c) {
        case '[':
          this->state = CSI;
          break;
        default:
          break;
      }
      break;
    case CSI:
      this->state = ParameterBytes;
      this->parameterIndex = 0;
      this->intermediaryIndex = 0;
      goto again;
      break;
    case ParameterBytes:
      if (parameterIndex < 128 && c >= 0x30 && c <= 0x3F) {
        parameterBytes[parameterIndex++] = c;
      } else {
        parameterIndex;
        this->state = IntermediaryBytes;
        goto again;
      }
      break;
    case IntermediaryBytes:
      if (intermediaryIndex < 128 && c >= 0x20 && c <= 0x2F) {
        intermediaryBytes[intermediaryIndex++] = c;
      } else {
        intermediaryIndex;
        this->state = FinalByte;
        goto again;
      }
      break;
    case FinalByte:
      switch (c) {
        case 'A':
          this->cur_y -= clamp(strToInt(parameterBytes, parameterIndex), 0, this->cur_y);
          break;
        case 'B':
          this->cur_y += clamp(strToInt(parameterBytes, parameterIndex), 0, this->height - this->cur_y);
          break;
        case 'C':
          this->cur_x += clamp(strToInt(parameterBytes, parameterIndex), 0, this->width - this->cur_x);
          break;
        case 'D':
          this->cur_x -= clamp(strToInt(parameterBytes, parameterIndex), 0, this->cur_x);
          break;
        case 'E':
          this->cur_y += clamp(strToInt(parameterBytes, parameterIndex), 0, this->height - this->cur_y);
          this->cur_x = 0;
          break;
        case 'F':
          this->cur_y -= clamp(strToInt(parameterBytes, parameterIndex), 0, this->cur_y);
          this->cur_x = 0;
          break;
        case 'G':
          this->cur_x = clamp(strToInt(parameterBytes, parameterIndex), 0, this->width);
          break;
        case 'f':
        case 'H': {
          uint32_t semicolonIndex = 0;
          while (parameterBytes[semicolonIndex++] != ';' && semicolonIndex <= parameterIndex);
          this->cur_y = clamp(strToInt(parameterBytes, parameterIndex) - 1, 0, this->height) ;
          this->cur_x = clamp(strToInt(parameterBytes+semicolonIndex, parameterIndex-semicolonIndex) - 1, 0, this->width);
          break;
        }
        case 'm':
          this->state = SGR;
          goto again;
          break;
        default: 
          break;
      }
      this->state = None;
      break;
    case SGR: {
      uint32_t index = 0;
      while (index <= parameterIndex) {
        uint32_t n = strToInt(parameterBytes+index, parameterIndex-index);
        switch (n) {
          case 0:
            this->edata = 0xf;
            break;
          case 1:
            if ((this->edata&0xf) <= 0x7)
              this->edata += 8;
            break;
          case 2:
            if ((this->edata&0xf) >= 0x7)
              this->edata -= 8;
            break;
          case 30 ... 37:
            this->edata &= 0xf0;
            this->edata |= n-30;
            break;
          case 40 ... 47:
            this->edata &= 0x0f;
            this->edata |= (n-40)<<4;
            break;
          default:
            break;
        }
        while (parameterBytes[index++] != ';' && index <= parameterIndex);
      }
      this->state = None;
    }
    default:
      break;
  }
  if (this->cur_y == this->height) {
    this->cur_y--;
    scroll_up();
  }
}

void Terminal::update(){}
void Terminal::update_cur(){}
void Terminal::putchar_internal(uint32_t ptr, uint8_t c) {}

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

void Terminal::printf(const char* string, ...) {
  va_list ptr;
  va_start(ptr, string);

  int index = 0;
  char current;

  while (current=string[index++]) {
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
          int promoted = va_arg(ptr, int);
          char charred = promoted;
          
          this->putchar(charred);
          break;
        }
      }
      continue;
    }
      
    this->putchar(current);
  }

  this->set_curpos(this->cur_x, this->cur_y);

  va_end(ptr);
}

uint32_t Terminal::write(uint32_t count, uint8_t* string) {
  for (int index=0; index < count; index++)
    this->putchar(string[index]);

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

void TextModeTerminal::putchar_internal(uint32_t ptr, uint8_t c) {
  text_mode_pointer[ptr] = c | (edata << 8);
}

TextModeTerminal::TextModeTerminal(uint16_t* text_mode_pointer): Terminal(80, 25, 1) {
  this->text_mode_pointer = text_mode_pointer;
}

void VGAModeTerminal::update() {
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      putchar_internal(y * width + x, (uint8_t)(current_page_pointer[y * width + x]));
    }
  }
}

void VGAModeTerminal::update_cur() {
  // Todo: Implement cursor for VGAModeTerminal
}

void VGAModeTerminal::putchar_internal(uint32_t ptr, uint8_t c) {
  uint32_t col = ptr % width;
  uint32_t row = ptr / width;

  uint32_t sx = col * 8;
  uint32_t sy = row * 8;

  if (c>127)
    return;
  uint8_t* char_data = font[c];

  for (int y=0; y<8; y++)
    put_pixels_byte(sx, sy+y, edata, char_data[y]);
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
    this->planes[i][pixelindex] = 0;
    if (color & (1<<i))
      this->planes[i][pixelindex] |= trbyte;
    if ((color>>4) & (1<<i))
      this->planes[i][pixelindex] |= ~trbyte;
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