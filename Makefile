disk.img: boot.sector kernel.bin hello.bin
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.sector of=disk.img conv=notrunc
	mount disk.img img.d
	cp *.bin img.d/
	umount img.d
	chmod 777 disk.img

boot.sector: boot.asm
	nasm boot.asm -o $@

kernel.bin: kernel.asm
	nasm kernel.asm -o $@

hello.bin: hello.asm
	nasm hello.asm -o $@