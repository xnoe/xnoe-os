OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/kernel/kernel.elf)

SECTIONS {
  . = 0xc0000000;

  .text : { 
    build/kernel/entry.o(.text)
    build/kernel/kmain.o(.text)
    build/kernel/*(.text)
    build/kernel/datatypes/*(.text)
  }
}