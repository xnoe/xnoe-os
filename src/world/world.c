#include "../common/common.h"
#include <stdbool.h>

void scrollBuffer(char* buf) {
  for (int y=0; y<21; y++)
    for (int x=0; x<38; x++)
      if (y != 20)
        buf[y*38+x] = buf[(y+1)*38+x];
      else
        buf[y*38+x] = ' ';
}

void writeToBuf(char c, char* buf, int* cx, int* cy) {
  switch (c) {
    case '\n':
      *cx = 0;
      (*cy)++;
      break;
    
    case '\b':
      if (*cx > 0)
        (*cx)--;
      else if (*cy > 0) {
        *cx = 37;
        (*cy)--;
      }
      buf[(*cy)*38+(*cx)] = ' ';
      break;

    default:
      buf[(*cy)*38+(*cx)++] = c;
  }
  if (*cx == 38) {
    *cx = 0;
    (*cy)++;
  }
  if (*cy == 21) {
    (*cy)--;
    scrollBuffer(buf);
  }
}

void writeStrToBuf(char* c, char* buf, int* cx, int* cy) {
  char* s = c;
  while(*s)
    writeToBuf(*(s++), buf, cx, cy);
}

void displayBuf(char* buf, int dx, int dy) {
  char pset[9] = "\x1b[00;00H";
  for (int i=0; i<dy;i++) {
    pset[3]++;
    if (pset[3] == 0x3a) {
      pset[3] = 0x30;
      pset[2]++;
    }
  }
  for (int i=0; i<dx;i++) {
    pset[6]++;
    if (pset[6] == 0x3a) {
      pset[6] = 0x30;
      pset[5]++;
    }
  }
  for (int i=0; i<21; i++) {
    print(pset);
    write(38, 0, buf+(38*i));
    pset[3]++;
    if (pset[3] == 0x3a) {
      pset[3] = 0x30;
      pset[2]++;
    }
  }
}

int main() {
  bindToKeyboard();

  char space = ' ';
  char plus = '+';

  print("\x1b[1;1H");
  for (int i=0; i < 1000; i++)
    write(1, 0, &space);
  print("\x1b[1;1H");

  char* mid =    "+                                      ++                                      +";
  char* bottom = "+                                                                              +";
  for (int i=0; i<80;i++)
    write(1, 0, &plus);
  for (int i=0; i<21;i++)
    write(80, 0, mid);
  for (int i=0; i<80;i++)
    write(1, 0, &plus);
  write(80, 0, bottom);
  for (int i=0; i<80;i++)
    write(1, 0, &plus);

  char* hello_bin = "HELLO   BIN";
  uint32_t p1 = fork(hello_bin);
  uint32_t p2 = fork(hello_bin);

  uint32_t p1out = bindStdout(p1);
  uint32_t p1in = bindStdin(p1);

  uint32_t p2out = bindStdout(p2);
  uint32_t p2in = bindStdin(p2);

  char* buf1 = localalloc(21 * 38);
  char* buf2 = localalloc(21 * 38);

  int b1cx = 0;
  int b1cy = 0;

  int b2cx = 0;
  int b2cy = 0;

  char* selectedBuf = buf1;
  uint32_t selectedOut = p1out;
  uint32_t selectedIn = p1in;
  

  while (1) {
    char c;
    if (read(1, selectedOut, &c))
      writeToBuf(c, selectedBuf, &b1cx, &b1cy);
    if (read(1, p2out, &c))
      writeToBuf(c, buf2, &b2cx, &b2cy);
    if (read(1, 1, &c)) {
      write(1, selectedIn, &c);
      write(1, p2in, &c);
    }
    
    displayBuf(selectedBuf, 2, 2);
    displayBuf(buf2, 42, 2);
  }
}