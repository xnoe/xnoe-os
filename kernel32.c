void clear_screen();
void set_curpos(int x, int y);
void print(const char* string);

unsigned char* VMEM_ADDR = (unsigned char*)0xb8000;
const int TERM_WIDTH = 80;
const int TERM_HEIGHT = 25;

int cursor_x = 0;
int cursor_y = 0;

int main() {
  asm(
    "mov $0x10, %ax\n"
    "mov %ax, %ds\n"
    "mov %ax, %ss\n"
    "mov $0x90000, %esp"
  );

  clear_screen();
  set_curpos(0, 0);

  print("Hello, World!\n\nWe are running XnoeOS Code in C now, Protected Mode has been achieved and everything is working super nicely!\n\nHow wonderful!\n\nNow I just need to hope my print function works properly too~~");

  while (1) {}
}

void clear_screen()  {
  for (int i=0; i<TERM_WIDTH*TERM_HEIGHT; i++) {
    VMEM_ADDR[i*2] = 0x20;
    VMEM_ADDR[i*2 + 1] = 0x07;
  }
}

void set_curpos(int x, int y) {
  cursor_x = x;
  cursor_y = y;
}

void print(const char* string) {
  int index = 0;
  char current;
  while (current=string[index++]) {
    if (current == '\n') {
      set_curpos(0, cursor_y+1);
      continue;
    }

    int mem_pos = cursor_y * TERM_WIDTH + cursor_x++;
    VMEM_ADDR[mem_pos*2] = current;
    VMEM_ADDR[mem_pos*2 + 1] = 0x07;

    if (index == TERM_WIDTH) {
      cursor_x = 0;
      cursor_y++;
    }
  }
}