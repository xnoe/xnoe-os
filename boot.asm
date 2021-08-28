[BITS 16]  ;tell the assembler that its a 16 bit code

mov ax, 7c0h
mov ds, ax

mov si, _boot_msg
call _boot_print

mov ah, 02h
mov al, 1
mov ch, 0
mov cl, 2
mov dh, 0

mov bx, 2000h
mov es, bx
xor bx, bx
int 13h

jmp 2000h:0

_boot_print:
  mov ah, 0eh
  mov cx, 1
  mov bh, 0
_boot_print_loop:
  lodsb
  cmp al, 0
  je _boot_print_exit
  int 10h
  jmp _boot_print_loop
_boot_print_exit:
  ret

_boot_msg db "Boot Sector OK!", 13, 10, 0

TIMES 510 - ($ - $$) db 0    ;fill the rest of sector with 0
DW 0xAA55          ; add boot signature at the end of bootloader
