OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

SECTIONS {
  . = 0x1020;

  .text : {
    build/programs/entry.o(.text)
    build/common/common.o(.text)
  }
}