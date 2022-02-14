#include "../common/common.h"
#include <stdbool.h>

typedef struct {
  char* buffer;
  int x;
  int y;
  uint32_t process;
  uint32_t stdin;
  uint32_t stdout;
} procbuffer;

void scrollBuffer(char* buf) {
  for (int y=0; y<21; y++)
    for (int x=0; x<38; x++)
      if (y != 20)
        buf[y*38+x] = buf[(y+1)*38+x];
      else
        buf[y*38+x] = ' ';
}

void writeToBuf(char c, procbuffer* buf) {
  switch (c) {
    case '\n':
      buf->x = 0;
      buf->y++;
      break;
    
    case '\b':
      if (buf->x > 0)
        buf->x--;
      else if (buf->y > 0) {
        buf->x = 37;
        buf->y--;
      }
      buf->buffer[buf->y*38+buf->x] = ' ';
      break;

    default:
      buf->buffer[buf->y*38+buf->x++] = c;
  }
  if (buf->x == 38) {
    buf->x = 0;
    buf->y++;
  }
  if (buf->y == 21) {
    buf->y--;
    scrollBuffer(buf->buffer);
  }
}

void clearBuf(procbuffer* buf) {
  for (int i=0; i<21*38;i++) {
    buf->buffer[i] = ' ';
  }
  buf->x = 0;
  buf->y = 0;
}

void writeStrToBuf(char* c, procbuffer* b) {
  char* s = c;
  while(*s)
    writeToBuf(*(s++), b);
}

void displayBuf(procbuffer* b, int dx, int dy) {
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
    write(38, 0, b->buffer+(38*i));
    pset[3]++;
    if (pset[3] == 0x3a) {
      pset[3] = 0x30;
      pset[2]++;
    }
  }
}

void setCurPos(int x, int y) {
  char pset[9] = "\x1b[00;00H";
  for (int i=0; i<y;i++) {
    pset[3]++;
    if (pset[3] == 0x3a) {
      pset[3] = 0x30;
      pset[2]++;
    }
  }
  for (int i=0; i<x;i++) {
    pset[6]++;
    if (pset[6] == 0x3a) {
      pset[6] = 0x30;
      pset[5]++;
    }
  }
  print(pset);
}

void readline(int count, char* buffer) {
  int index = 0;
  char c;
  while (index < count) {
    if (read(1, 1, &c)) {
      if (c == '\n')
        break;
      if (c == '\b') {
        if (index == 0)
          continue;
        else {
          index--;
          buffer[index] = 0;
          write(1, 0, &c);
          continue;
        }
      }

      buffer[index++] = c;
      write(1, 0, &c);
    }
  }
}

bool strcmp(char* a, char* b) {
  int index=0;
  while (a[index])
    if (a[index] == b[index])
      index++;
    else
      return false;
  return true;
}

bool strcmpcnt(int count, char* a, char* b) {
  int index=0;
  while (index < count)
    if (a[index] == b[index])
      index++;
    else
      return false;
  return true;
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

  uint32_t program = fopen("hello.bin");
  uint32_t p1 = fork(program);
  uint32_t p1out = bindStdout(p1);
  uint32_t p1in = bindStdin(p1);
  fclose(program);
  program = fopen("hello.bin");
  uint32_t p2 = fork(program);
  uint32_t p2out = bindStdout(p2);
  uint32_t p2in = bindStdin(p2);
  fclose(program);

  procbuffer b1 = {
    .buffer = localalloc(21 * 38),
    .x = 0,
    .y = 0,
    .process = p1,
    .stdin = p1in,
    .stdout = p1out
  };

  procbuffer b2 = {
    .buffer = localalloc(21 * 38),
    .x = 0,
    .y = 0,
    .process = p2,
    .stdin = p2in,
    .stdout = p2out
  };

  procbuffer* selectedBuf = &b1;

  writeStrToBuf("XoSH (Xnoe SHell) v0.0.1\nPress : to use commands.\n :help for help.\n", &b1);

  while (1) {
    char c;
    if (b1.process)
      if (read(1, b1.stdout, &c))
        writeToBuf(c, &b1);
    if (b2.process)
      if (read(1, b2.stdout, &c))
        writeToBuf(c, &b2);
    if (read(1, 1, &c)) {
      if (c == ':') {
        char buf[32] = {0};
        print("\x1b[24;2H");
        print(":                                ");
        print("\x1b[24;3H");
        readline(32, buf);
        if (strcmpcnt(6, buf, "switch")) {
          if (selectedBuf == &b1) {
            selectedBuf = &b2;
          } else {
            selectedBuf = &b1;
          }
        } else if (strcmpcnt(4, buf, "help")) {
          writeStrToBuf("\n--------\n", selectedBuf);
          writeStrToBuf(":help\n", selectedBuf);
          writeStrToBuf("  Displays this message.\n", selectedBuf);
          writeStrToBuf(":switch\n", selectedBuf);
          writeStrToBuf("  Switches which process you're using\n", selectedBuf);
          writeStrToBuf(":kill\n", selectedBuf);
          writeStrToBuf("  Kills the current process\n", selectedBuf);
          writeStrToBuf(":load <filename>\n", selectedBuf);
          writeStrToBuf("  Loads and executes the program <filename>\n", selectedBuf);
          writeStrToBuf("--------\n", selectedBuf);
        } else if (strcmpcnt(4, buf, "kill")) {
          if (selectedBuf->process) {
            kill(selectedBuf->process);
            clearBuf(selectedBuf);
            selectedBuf->process = 0;
            selectedBuf->stdin = 0;
            selectedBuf->stdout = 0;
            if (selectedBuf == &b1) {
              selectedBuf = &b2;
            } else {
              selectedBuf = &b1;
            }
          }
        } else if (strcmpcnt(4, buf, "load")) {
          if (!selectedBuf->process) {
            char* potFn = buf+5;
            uint32_t fh = fopen(potFn);
            selectedBuf->process = fork(fh);
            selectedBuf->stdout = bindStdout(selectedBuf->process);
            selectedBuf->stdin = bindStdin(selectedBuf->process);
            fclose(fh);
          }
        }
      } else {
        if (selectedBuf->process)
          write(1, selectedBuf->stdin, &c);
      }
    }
    
    displayBuf(&b1, 2, 2);
    displayBuf(&b2, 42, 2);
  }
}