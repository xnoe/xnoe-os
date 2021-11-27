[BITS 32]

_start:
  call main
  int 0x80

extern main