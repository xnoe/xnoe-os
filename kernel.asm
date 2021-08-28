  mov ax, 2000h
  mov ds, ax

  push msg
  call print

  push msg2
  call print

  push 13
  push msg
  push msg
  call strcmp

  cmp ax, 1
  jne _ne
  push msg_same
  call print
_ne:

  jmp $

data:
  msg db "Kernel OK!", 13, 10, 0
  msg2 db "Hello, World!", 13, 10, 0
  msg_same db "They are the same", 13, 10, 0

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

strcmp:
  push bp
  mov bp, sp
  mov ax, [bp + 4]
  lea si, [bp+6]
  lea di, [bp+8]
  dec di
  mov word [bp - 2], 0
strcmp_loop:
  inc di
  inc word [bp - 2]
  lodsb
  cmp [di], al
  jne strcmp_notequal
  cmp [bp - 2], ax
  jne strcmp_equal


strcmp_equal:
  mov ax, 1
  pop bp
  ret
strcmp_notequal:
  mov ax, 0
  pop bp
  ret