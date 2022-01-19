OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

OUTPUT(build/hello/hello.bin)

SECTIONS {
  . = 0x20;

  .text : {
    build/program_code_entry.o(.text)
    build/hello/hello.o(.text)
    build/common/common.o(.text)
  }
}