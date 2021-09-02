OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

INPUT(kernel32_strap.o kernel32.o)
OUTPUT(kernel32.bin)

SECTIONS {
  . = 0x80000;

  .text : { 
    kernel32_strap.o(.text)
    kernel32.o(.text)
  }
}