[BITS 32]

_start:
  call main
  call die

extern die
extern main