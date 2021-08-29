disk.img: boot.bin kernel.bin
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.bin of=disk.img conv=notrunc
	mount disk.img img.d
	cp kernel.bin img.d
	umount img.d
	chmod 777 disk.img

boot.bin: boot.asm
	nasm boot.asm -o $@

kernel.bin: kernel.asm
	nasm kernel.asm -o $@
