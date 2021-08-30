  mov ax, 3000h
  mov ds, ax

  mov si, filename
  mov ah, 4
  int 22h

  cmp ax, 0
  je error

  mov si, ax
  mov di, 0
  mov bx, 4000h
  mov ah, 3
  int 22h

  mov ax, 4000h
  mov ds, ax
  mov si, 0
  mov ah, 1
  int 22h

  mov ax, 2000h
  mov ds, ax
  jmp exit

error:
  mov si, err_msg
  mov ah, 1
  int 22h

exit:
  retf

filename db "HELLO   TXT"
err_msg db "HELLO.TXT Missing!"