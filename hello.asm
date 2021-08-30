mov ax, 3000h
mov ds, ax

mov si, hello_world_str
mov ah, 01h
int 22h

retf

hello_world_str db "Hello, World!", 13, 10, 13, 10, 0
new_line db 13, 10, 0

buffer:
  times 32 db 0