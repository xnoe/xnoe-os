mov ax, 3000h
mov ds, ax

push hello_world_str
call print

;jmp $

retf

; print(str)
print:
  push bp
  mov bp, sp
  mov si, [bp + 4]
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
  pop bp
  ret 2

hello_world_str db "Hello, World!", 13, 10, 13, 10, 0