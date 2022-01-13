[BITS 32]

_start:
  mov esp, 0xc100a000
  jmp main

extern main