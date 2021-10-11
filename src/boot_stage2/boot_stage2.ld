OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/boot_stage2/boot.bin)

SECTIONS {
  . = 0x7E00;

  .text : {
    build/c_code_entry.o(.text)
    build/boot_stage2/stage2.o(.text)
    build/boot_stage2/*(.text)
  }
}