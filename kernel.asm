sect2:
  mov ax, 2000h
  mov ds, ax

  mov si, msg
  call print

  mov si, msg2
  call print

  jmp $

  msg db "Kernel OK!", 13, 10, 0
  msg2 db "Hello, World!", 13, 10, 0

print:
  mov ah, 0eh
  mov cx, 1
  mov bh, 0
print_loop:
  lodsb
  cmp al, 0
  je print_exit
  int 10h
  jmp print_loop
print_exit:
  ret

