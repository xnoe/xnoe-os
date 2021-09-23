[BITS 32]

_start:
  mov ax, 10h
  mov ds, ax
  mov ss, ax
  mov esp, 90000h

  call main

extern main