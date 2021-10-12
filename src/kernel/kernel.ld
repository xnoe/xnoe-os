OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/kernel/kernel.bin)

SECTIONS {
  . = 0xc0000000;

  .text : { 
    build/kernel/entry.o(.text)
    build/kernel/kernel.o(.text)
    build/kernel/*(.text)
  }
}