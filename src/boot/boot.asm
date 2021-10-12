[ORG 0x7C00]
[BITS 16]

jmp short bootcode
nop

bpOEMid db "XNOE    "

bpBytesPerSector dw 512
bpSectorsPerCluster db 1
bpReservedSectors dw 32 ; We should reserve some sectors. Boot sector + stage2
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

  ; Load the stage2 (32-bit) from the next sectors
  ; Load it at 0x7e00
  mov ax, 7e0h
  mov es, ax
  xor bx, bx

  mov ax, 2 ; Begin with the 2nd sector
  call prep_i13
  mov al, 31 ; Load the next 9 sectors (4.5k)
  int 13h

  ; We need to get the memory configuration from the BIOS now
  ; To do this we can use int 15h eax=e820h
  ; We will load the e820 data to 0x20000

  mov ax, 2000h
  mov es, ax
  xor di, di
  xor ebx, ebx

get_memory_map_loop:
  mov edx, 0x534D4150
  mov ecx, 24

  mov eax, 0xe820
  int 15h
  jc mmap_finish

  cmp ebx, 0
  je mmap_finish

  add di, 24
  jmp get_memory_map_loop

mmap_finish:
  ; Now we should prepare to enter in to protected mode for the sole purpose of running the stage2 bootloader

  ; Enable the A20 line
  in al, 0x92
  or al, 2
  out 0x92, al

  ; Load the temporary GDT
  cli
  lgdt [gdt_desc]
  mov eax, cr0
  or eax, 1
  mov cr0, eax

  jmp 08h:7e00h ; Far jump to where we loaded the stage 2

; In ax with sector addr, out correct values for int 13h
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

gdt:
null:
  dq 0
code:
  dw 0xffff
  dw 0
  db 0
  db 10011010b
  db 11001111b
  db 0
data:
  dw 0xffff
  dw 0
  db 0
  db 10010010b
  db 11001111b
  db 0
gdt_end:
gdt_desc:
  dw gdt_end - gdt - 1
  dd gdt

TIMES 510 - ($ - $$) db 0
DW 0xAA55