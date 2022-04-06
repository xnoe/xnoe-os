[BITS 32]

_start:
  call main
_loop:
  jmp _loop

extern main