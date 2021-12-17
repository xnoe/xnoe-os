OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/world/world.bin)

SECTIONS {
  . = 0x8020;

  .text : {
    build/program_code_entry.o(.text)
    build/world/world.o(.text)
    build/common/common.o(.text)
  }
}