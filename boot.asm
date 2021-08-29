[BITS 16]  ;tell the assembler that its a 16 bit code

jmp short bootcode
nop

bpOEMid db "XNOE    "
bpBytesPerSector dw 512
bpSectorsPerCluster db 1
bpReservedSectors dw 1
bpNoFATs db 2
bpRootDirEntries dw 256
bpLVSectors dw 8419
bpMediumID db 0F0h
bpSectorsPerFat dw 9
bpSectorsPerTrack dw 18
bpSides dw 2
bpHiddenSectors dd 0
bpLargeSectors dd 0
bpDriveNo dw 0
bpSignature db 41
bpVolumeID dd 00000000h
bpVolumeLabel db "XNOE OS    "
bpFileSystem db "FAT16   "

bootcode:
  mov ax, 7c0h
  mov ds, ax

  mov byte [drive], dl

  mov si, boot_msg
  call _boot_print

  mov ax, ds
  mov es, ax
  mov bx, buffer

  mov ah, 2
  mov al, 16
  mov cl, 20
  mov ch, 0
  mov dh, 0
  mov dl, byte [drive]

  int 13h

  mov cx, [bpRootDirEntries]
  mov di, buffer

  mov ax, 0

;  jmp $

kernel_finder:
  xchg cx, dx

  mov di, buffer
  add di, ax

;  mov si, di
;  call _boot_print
;  mov si, new_line
;  call _boot_print

  mov si, kernel_file
  mov cx, 11
  rep cmpsb
  je kernel_found

  add ax, 20h

  xchg cx, dx
  loop kernel_finder

  mov si, kernel_nf
  call _boot_print

  jmp $

kernel_found:
  mov ax, [es:di+0fh]
  mov word [cluster], ax

fat_loader:
  mov ax, ds
  mov es, ax
  mov bx, buffer

  mov ah, 2
  mov al, 9
  mov cl, 2
  mov ch, 0
  mov dh, 0
  mov dl, byte [drive]

  int 13h

kernel_loader:
  mov ax, 2000h
  mov es, ax
  mov bx, word [pointer]

  mov al, byte [cluster]
  mov word [buffer+20h], ax

  add al, 34

  mov cl, al
  mov al, 1
  mov ah, 2
  mov ch, 0
  mov dh, 0
  mov dl, byte [drive]

  int 13h

  add word [pointer], 512

  mov si, word [cluster]
  add si, buffer
  add si, 1
  cmp word [si], 0ffffh
  je kernel_loaded

  add word [cluster], 1
  jmp kernel_loader

kernel_loaded:
  mov dword [buffer], 0xdeadbeef
  jmp 2000h:0h

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

boot_msg db "Boot Sector OK!", 13, 10, 0
kernel_nf db "KERNEL.BIN Missing!", 13, 10, 0
new_line db " ", 0
kernel_file db "KERNEL  BIN"

cluster dw 0
pointer dw 0
drive db 0

TIMES 510 - ($ - $$) db 0    ;fill the rest of sector with 0
DW 0xAA55          ; add boot signature at the end of bootloader

buffer: