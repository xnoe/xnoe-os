  push bp
  mov bp, sp

  mov ax, [bp+6]
  mov ds, ax
  mov word [program_segment], ax

  mov si, filename
  mov ah, 4
  int 22h

  cmp ax, 0
  je error

  mov si, ax
  mov di, 0
  mov bx, word [program_segment]
  add bx, 1000h
  mov ah, 3
  int 22h

  push word [program_segment]
  mov ax, word [program_segment]
  add ax, 1000h
  mov ds, ax
  mov si, 0
  mov ah, 1
  int 22h

  pop ax
  mov ds, ax

  jmp exit

error:
  mov si, err_msg
  mov ah, 1
  int 22h

exit:
  pop bp
  retf 

filename db "HELLO   TXT"
err_msg db "HELLO.TXT Missing!"
program_segment dw 0