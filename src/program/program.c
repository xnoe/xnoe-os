#include "../kernel/types.h"
#include "../common/common.h"

int main() {
  print("Testing C code program\n");
  print("My strings are messed up for some reason...\n");

  uint32_t alpha_size = filesize("HELLO   TXT");
  char sizebuf[32];
  uint32_t index = int_to_decimal(alpha_size, sizebuf);
  print(sizebuf+index);
  print("\n");
  uint8_t* alpha_buffer = (uint8_t*)localalloc(alpha_size + 32);
  print("alpha_buffer: ");
  index = int_to_hex(alpha_buffer, sizebuf);
  print(sizebuf+index);
  print("\n");
  readfile("HELLO   TXT", alpha_buffer);
  print(alpha_buffer);
}