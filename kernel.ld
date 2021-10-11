OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(kernel.bin)

SECTIONS {
  . = 0xc0000000;

  .text : { 
    c_code_entry.o(.text)
    kernel.o(.text)
    *(.text)
    heap_section = .;
  }
}