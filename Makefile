disk.img: boot kernel
	cat boot.bin kernel.bin > disk.img

boot.bin: boot.asm
	nasm boot.asm -o $@

kernel.bin: kernel.asm
	nasm kernel.asm -o $@
