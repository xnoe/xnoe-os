OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/program/hello.bin)

SECTIONS {
  . = 0x8020;

  .text : {
    build/program_code_entry.o(.text)
    build/program/hello.o(.text)
    build/common/common.o(.text)
  }
}