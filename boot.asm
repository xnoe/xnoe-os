[BITS 16]

jmp short bootcode
nop

bpOEMid db "XNOE    "

bpBytesPerSector dw 512
bpSectorsPerCluster db 1
bpReservedSectors dw 1
bpNoFATs db 2
bpRootDirEntries dw 256
bpLVSectors dw 8586
bpMediumID db 0xF8
bpSectorsPerFat dw 34
bpSectorsPerTrack dw 18
bpHeads dw 2
bpHiddenSectors dw 0

TIMES 36 - ($ - $$) db 0

bpDriveNo db 0
db 0
bpSignature db 0x29
bpVolumeID dd 0x0
bpVolumeLabel db "XNOE OS    "
bpFileSystem db "FAT16   "

bootcode:
  mov ax, 7c0h
  mov ds, ax

  mov byte [bpDriveNo], dl

  ; Get the disk configuration from the BIOS
  mov ah, 08h
  int 13h

  add dh, 1
  movzx dx, dh
  mov word [bpHeads], dx
  and cl, 03fh
  movzx cx, cl
  mov word [bpSectorsPerTrack], cx

  mov si, boot_msg
  call _boot_print

  mov ax, ds
  mov es, ax
  mov bx, buffer

  ; Calculate position of rootdirentries
  mov ax, word [bpSectorsPerFat]
  movzx cx, byte [bpNoFATs]
  mul cx

  add ax, 2
  call prep_i13

  int 13h

  mov cx, [bpRootDirEntries]
  mov di, buffer

  mov ax, 0

;  jmp $

kernel_finder:
  xchg cx, dx

  mov di, buffer
  add di, ax

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
;  jmp $
  mov ax, [es:di+0fh]
  mov word [cluster], ax

fat_loader:
  mov ax, ds
  mov es, ax
  mov bx, buffer

  mov ax, word [bpSectorsPerFat]
  mov ah, 2
  mov cl, 2
  mov ch, 0
  mov dh, 0
  mov dl, byte [bpDriveNo]

  int 13h

kernel_loader:

  ; Calculate file offset
  mov ax, word [bpSectorsPerFat]
  movzx cx, byte [bpNoFATs]
  mul cx

  push ax
  mov ax, word [bpRootDirEntries]
  mov cx, 16
  div cx

  mov bx, ax
  pop ax
  add ax, bx

  mov word [fileoffset], ax

  mov ax, 2000h
  mov es, ax
  mov bx, word [pointer]

  movzx ax, byte [cluster]
  add ax, word [fileoffset]

  call prep_i13

  int 13h

  add word [pointer], 512

  mov si, word [cluster]
  shl si, 1
  add si, buffer
  cmp word [si], 0ffffh
  je kernel_loaded

  add word [cluster], 1
  jmp kernel_loader

kernel_loaded:
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

fileoffset dw 0

cluster dw 0
pointer dw 0

; AX set to the logical sector number.
prep_i13:
  xor dx, dx
  div word [bpSectorsPerTrack]

  push dx

  xor dx, dx
  div word [bpHeads]

  mov ch, al
  mov dh, dl
  mov dl, byte [bpDriveNo]

  pop ax
  mov cl, al

  mov al, 1
  mov ah, 2

  ret

TIMES 510 - ($ - $$) db 0
DW 0xAA55

buffer: