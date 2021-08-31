[BITS 16]
  ; Update the IVT with our interrupt handler
  mov bx, 0
  mov ds, bx
  mov word [ds:136], ihdlr
  mov word [ds:138], cs

  mov ax, 2000h
  mov ds, ax

  pop ax
  mov byte [bootdisk], al

  ; Set up kernel specifics

  ; Root Directory Entries will be at 2000h:2000h
  ; FAT1 will be at 2000h:4000h

  mov ax, 2000h
  mov es, ax
  mov bx, 2000h
  mov ah, 2
  mov al, 16
  mov cl, 20
  mov ch, 0
  mov dh, 0
  mov dl, [bootdisk]

  ; Load sectors 20 through 36 to 2000h:2000h
  int 13h

  mov ax, 2000h
  mov es, ax
  mov bx, 4000h
  mov ah, 2
  mov al, 9
  mov cl, 2
  mov ch, 0
  mov dh, 0
  mov dl, [bootdisk]
  
  ; Load sectors 2 through 11 to 2000h:4000h
  int 13h

  push msg
  call print

cmd_loop:
  push cmd_prompt
  call print

  ; Clear command buffer
  mov al, 0
  mov di, user_cmd
  mov cx, 64

clear_loop:
  stosb
  loop clear_loop

  push user_cmd
  push 64
  call readline

  push newline
  call print

  mov si, user_cmd
  mov di, cmd_help_text
  mov cx, 4
  rep cmpsb
  je cmd_help

  mov si, user_cmd
  mov di, cmd_clear_text
  mov cx, 5
  rep cmpsb
  je cmd_clear

  ; If the user hasn't run a command we should determine if they're trying to run a program.

  ; Decode input to filename
  push user_cmd
  call decode_filename

  ; Search for file


  mov si, ax
  call file_exists
  ; If it doesn't exist jump to error
  cmp ax, 0
  je handle_error

  ; Define the segment where we will load our programs
  program_segment equ 4000h

  ; If the file exists load it at program_segment:0
  push ax
  push 0
  push program_segment
  call load_file

  ; Let programs know what segment they're loaded at
  push program_segment
  ; Make a call to program_segment:0
  call program_segment:0
  ; We've now returned.
  ; Clean up pushed segment
  pop ax



  ; Recover ds
  mov ax, 2000h
  mov ds, ax
  mov es, ax

;  jmp $

  ; Loop
  jmp cmd_loop

handle_error:
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
  cmd_err db "Bad Command or filename!", 13, 10, 13, 10, 0
  
  cmd_help_text db "HELP"
  cmd_clear_text db "CLEAR"

  newline db 13, 10, 0

  bootdisk db 0

  program_two db "HELLO   BIN"



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
  ; Use Unix Line endings (\n)
  cmp al, 10
  jne _print_skip
  int 10h
  mov al, 13
  int 10h
_print_skip:
  int 10h
  jmp print_loop
print_exit:
  pop bp
  ret 2

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

  ; Process all user input as upper case
  cmp al, 0x61
  jl not_lower
  cmp al, 0x79
  jg not_lower
  and al, 223
not_lower:
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
  ret 4

file_exists:
  push si

  mov bp, sp
  mov cx, 256 ; Hardcode the amount of entries for now

  mov ax, 2000h
  mov es, ax

  xor ax, ax
file_exists_loop:
  pop si ; Get value in to si
  push si ; Keep track of original value

  xchg cx, dx

  mov di, 2000h ; Root Entries is at 2000h:2000h
  add di, ax
  mov cx, 11
  rep cmpsb
  je file_exists_found

  add ax, 32

  xchg cx, dx
  loop file_exists_loop
file_exists_not_found:
  mov ax, 0
  pop si
  ret
file_exists_found:
  mov ax, [es:di+0fh]
  pop si
  ret

; loadfile(segment, offset, initsector)
load_file:
  push bp
  mov bp, sp

  push ds
  mov ax, 2000h
  mov ds, ax

  mov ax, word [bp + 4]
  mov es, ax

load_file_loop:
  mov bx, word [bp + 6]

  mov al, byte [bp + 8]
  add al, 34

  mov cl, al
  mov al, 1
  mov ah, 2
  mov ch, 0
  mov dh, 0
  mov dl, byte [bootdisk]

  int 13h

  add word [bp + 6], 512

  mov si, word [bp + 8]
  shl si, 1
  add si, 4000h
;  add si, 1
  cmp word [ds:si], 0ffffh
  je load_file_loaded

  add word [bp + 8], 1
  jmp load_file_loop

load_file_loaded:
  pop ds
  pop bp
  ret 6

decode_filename: 
  push bp
  mov bp, sp
  push word [bp + 4]
  ; First we want to clear the buffer with 0x20s
  mov al, 20h
  mov cx, 11
  mov di, _decode_buffer
decode_clear_loop:
  stosb
  loop decode_clear_loop

  pop si


  mov cx, 8
  mov bx, 0
decode_filename_loop:

  lodsb
  cmp al, "."
  je decode_filename_stage2
  mov byte [_decode_buffer+bx], al

  inc bx

  loop decode_filename_loop

decode_filename_stage2:
  mov bx, 8
  mov cx, 3
;  add si, 1

decode_filename_stage2_loop:

  lodsb
  cmp al, 0
  je decode_filename_final
  mov byte [_decode_buffer+bx], al

  inc bx

  loop decode_filename_stage2_loop

decode_filename_final:
  pop bp
  mov ax, _decode_buffer
  ret 2

_decode_buffer:
  times 11 db 0

ihdlr:
  cmp ah, 01h
  jne _ihdlr_2
  push si
  call print
  jmp _ihdlr_fin
_ihdlr_2:
  cmp ah, 02h
  jne _ihdlr_3
  push si
  push di
  call readline
  jmp _ihdlr_fin
_ihdlr_3:
  cmp ah, 03h
  jne _ihdlr_4
  push si
  push di
  push bx
  call load_file
  jmp _ihdlr_fin
_ihdlr_4:
  cmp ah, 04h
  jne _ihdlr_5
  call file_exists
  jmp _ihdlr_fin
_ihdlr_5:
_ihdlr_fin:
  iret