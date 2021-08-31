disk.img: boot.sector kernel.bin hello.bin print.bin hello.txt boot32.bin
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.sector of=disk.img conv=notrunc
	mount disk.img img.d
	cp *.bin img.d/
	cp hello.txt img.d/
	umount img.d
	chmod 777 disk.img

boot.sector: boot.asm
	nasm $< -o $@

kernel.bin: kernel.asm
	nasm $< -o $@

hello.bin: hello.asm
	nasm $< -o $@

print.bin: print.asm
	nasm $< -o $@

boot32.bin: boot32.asm
	nasm $< -o $@