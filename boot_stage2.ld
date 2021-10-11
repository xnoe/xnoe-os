OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386:i386)

INPUT(io.o atapio.o strings.o c_code_entry.o screenstuff.o paging.o)
OUTPUT(boot.stage2)

SECTIONS {
  . = 0x7E00;

  .text : {
    c_code_entry.o(.text)
    boot.stage2.o(.text)
    *(.text)
  }
}