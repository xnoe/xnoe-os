OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(kernel32.bin)

SECTIONS {
  . = 0x80000;

  .text : { 
    kernel32_strap.o(.text)
    kernel32.o(.text)
    *(.text)
    heap_section = .;
  }
}