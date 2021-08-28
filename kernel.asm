[BITS 16]
  mov ax, 2000h
  mov ds, ax

  push msg
  call print

cmd_loop:
  push cmd_prompt
  call print

  push user_cmd
  push 64
  call readline

  push newline
  call print

  push user_cmd
  push cmd_help_text
  push 4
  call strcmp

  cmp ax, 1
  je cmd_help

  push user_cmd
  push cmd_clear_text
  push 5
  call strcmp

  cmp ax, 1
  je cmd_clear

  push cmd_err
  call print

  jmp cmd_loop

cmd_help:
  push cmd_help_msg
  call print
  jmp cmd_loop

cmd_help_msg:
  db "XnoeOS Help Dialogue", 13, 10
  db "--------------------", 13, 10
  db "Commands: ", 13, 10
  db " - help", 13, 10
  db " : Displays this message", 13, 10
  db " - clear", 13, 10
  db " : Clears the screen", 13, 10
  db 0

cmd_clear:
  mov ah, 02h
  mov dh, 0
  mov dl, 0
  int 10h

  mov ah, 0ah
  mov cx, 2000
  mov al, 20h
  int 10h

  jmp cmd_loop

data:
  msg db "Kernel OK!", 13, 10, 0
  cmd_prompt db ">>> ", 0
  cmd_err db "Bad Command!", 13, 10, 13, 10, 0
  
  cmd_help_text db "help"
  cmd_clear_text db "clear"

  newline db 13, 10, 0



user_cmd:
  times 64 db 0 

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
  ret

; strcmp(length, str1, str2)
strcmp:
  push bp
  mov bp, sp
  mov si, [bp + 6]
  mov di, [bp + 8]
  mov word [bp - 2], 0
strcmp_loop:
  mov ax, [bp + 4]
  cmp [bp - 2], ax
  je strcmp_equal

  lodsb

  cmp [di], al
  jne strcmp_notequal

  inc di
  inc word [bp - 2]
  jmp strcmp_loop

strcmp_equal:
  mov ax, 1
  jmp strcmp_exit
strcmp_notequal:
  mov ax, 0
strcmp_exit:
  pop bp
  ret

; readline(max length, buffer)
readline:
  push bp
  mov bp, sp
  mov di, [bp + 6]
  mov bx, 0
readline_loop:
  mov ah, 0h
  int 16h

  cmp ah, 01ch
  je readline_exit

  cmp ah, 0eh
  je readline_backspace

  mov byte [di], al
  inc di
  inc bx

  mov ah, 0eh
  mov cx, 1
  int 10h

  mov ax, [bp + 4]
  cmp bx, ax
  je readline_exit

  jmp readline_loop

readline_backspace:

  cmp bx, 0
  je readline_loop

  dec di
  dec bx

  mov ah, 0eh
  mov cx, 1
  int 10h

  mov ah, 0ah
  mov cx, 1
  mov al, 20h
  int 10h

  jmp readline_loop

readline_exit:
  pop bp
  ret