OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/bootstage2/boot.bin)

SECTIONS {
  . = 0x7E00;

  .text : {
    build/c_code_entry.o(.text)
    build/bootstage2/main.o(.text)
    build/bootstage2/*(.text)
  }
}