  [ORG 0x40000]

  push bp
  mov bp, sp
  mov ax, [bp + 6]
;  xor ax, ax
  mov ds, ax
  mov word [program_segment], ax

  mov si, kernel32_name
  mov ah, 4
  int 22h

  cmp ax, 0
  je load_err

  push ax

  mov si, kernel32_found_success_msg
  mov ah, 1
  int 22h

  ; Enable the A20 line
  ; We don't really care about supporting super old machines
  in al, 0x92
  or al, 2
  out 0x92, al

  pop si
  mov di, 0 ; Load kernel32.bin at 0x40200
  mov bx, 8000h
  mov ah, 3
  int 22h

  mov si, kernel32_exec_msg
  mov ah, 1
  int 22h

  ; Prepare for for kernel32.bin execution

  cli
  lgdt [gdt_desc]
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  ; Execute kernel32.bin

  jmp 08h:0

  jmp exit
  

load_err:
  mov si, kernel32_load_err_msg
  mov ah, 1
  int 22h

exit:
  pop bp
  retf

kernel32_name db "KERNEL32BIN"
kernel32_load_err_msg db "FAILED TO LOAD KERNEL32.BIN", 10, 0
kernel32_found_success_msg db "FOUND KERNEL32.BIN", 10, 0
kernel32_exec_msg db "EXECUTING KERNEL32.BIN...", 10, 0

program_segment dw 0

gdt:
gdt_null:
  dq 0
gdt_code:
  dw 0xffff
  dw 0x0
  db 0x8
  db 10011010b
  db 01001111b
  db 0
gdt_data:
  dw 0xffff
  dw 0x0
  db 0x8
  db 10010010b
  db 01001111b
  db 0
gdt_end:

gdt_desc:
  dw gdt_end - gdt - 1
  dd gdt